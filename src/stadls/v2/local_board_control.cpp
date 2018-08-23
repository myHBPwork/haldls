#include "stadls/v2/local_board_control.h"

#include <chrono>
#include <sstream>
#include <thread>

#include "flyspi-rw_api/flyspi_com.h"
#include "halco/common/iter_all.h"
#include "log4cxx/logger.h"
#include "uni/bytewise_output_iterator.h"
#include "uni/decoder.h"
#include "uni/program_builder.h"

#include "haldls/exception/exceptions.h"
#include "haldls/v2/board.h"
#include "haldls/v2/chip.h"
#include "haldls/v2/common.h"
#include "haldls/v2/playback.h"
#include "haldls/v2/fpga.h"
#include "haldls/v2/spike.h"
#include "stadls/v2/ocp.h"
#include "stadls/visitors.h"

namespace {

// vvv ------8<----------- (legacy code copied from frickel-dls)

struct Sdram_block_write_allocator
{
	static size_t const block_size = 4096;

	typedef rw_api::flyspi::SdramBlockWriteQuery Container;
	typedef uni::Bytewise_output_iterator<rw_api::FlyspiCom::Data, rw_api::FlyspiCom::BufferPtr>
		Iterator;

	rw_api::FlyspiCom& com;
	rw_api::FlyspiCom::Locator loc;
	uint32_t address;

	Sdram_block_write_allocator(rw_api::FlyspiCom& com, uint32_t address)
		: com(com), address(address)
	{
		loc = com.locate().chip(0);
	}

	Iterator begin(Container& c) { return uni::bytewise(std::begin(c)); }
	Iterator end(Container& c) { return uni::bytewise(std::end(c)); }
	Container allocate(size_t capacity)
	{
		rw_api::flyspi::SdramBlockWriteQuery rv(com, loc, capacity);
		rv.resize(capacity);
		rv.addr(address);
		address += capacity;
		return rv;
	}
};

// ^^^ ------8<-----------

struct UniDecoder
{
	std::vector<haldls::v2::hardware_word_type> words;
	haldls::v2::hardware_time_type current_time = 0;
	std::vector<haldls::v2::RecordedSpike> spikes;

	template <typename T>
	void operator()(T const& /*inst*/)
	{}

	void operator()(uni::Write_inst const& inst) { words.push_back(inst.data); }

	void operator()(uni::Set_time_inst const& inst) { current_time = inst.t; }

	void operator()(uni::Wait_until_inst const& inst) { current_time = inst.t; }

	void operator()(uni::Wait_for_7_inst const& inst) { current_time += inst.t; }

	void operator()(uni::Wait_for_16_inst const& inst) { current_time += inst.t; }

	void operator()(uni::Wait_for_32_inst const& inst) { current_time += inst.t; }

	void operator()(uni::Fire_inst const& inst)
	{
		using namespace haldls::v2;
		using namespace halco::hicann_dls::v2;
		using namespace halco::common;
		assert(inst.fire.size() == SynapseBlock::Synapse::Address::size);
		for (auto const address : iter_all<NeuronOnDLS>()) {
			if (!inst.fire.test(NeuronOnDLS::max - address))
				continue;
			spikes.emplace_back(current_time, address);
		}
	}

	void operator()(uni::Fire_one_inst const& inst)
	{
		using namespace haldls::v2;
		using namespace halco::hicann_dls::v2;
		assert(inst.index < NeuronOnDLS::size);
		spikes.emplace_back(current_time, NeuronOnDLS(NeuronOnDLS::max - inst.index));
	}
};

} // namespace

namespace stadls {
namespace v2 {

class LocalBoardControl::Impl
{
public:
	typedef haldls::v2::hardware_word_type hardware_word_type;
	typedef haldls::v2::hardware_address_type hardware_address_type;

	Impl(std::string const& usb_serial_number) : com(usb_serial_number) {}

	rw_api::FlyspiCom com;

	haldls::v2::PlaybackProgram::serial_number_type program_serial_number =
		haldls::v2::PlaybackProgram::invalid_serial_number;
	static constexpr hardware_address_type program_address = 0;
	hardware_address_type program_size = 0;
	static constexpr hardware_address_type result_address = 0;
};

LocalBoardControl::LocalBoardControl(std::string const& usb_serial_number)
	: m_impl(new Impl(usb_serial_number))
{
	soft_reset();
}

LocalBoardControl::LocalBoardControl(LocalBoardControl&&) noexcept = default;

LocalBoardControl& LocalBoardControl::operator=(LocalBoardControl&&) noexcept = default;

LocalBoardControl::~LocalBoardControl() = default;

std::string LocalBoardControl::usb_serial() const
{
	return m_impl->com.usb_serial;
}

void LocalBoardControl::soft_reset()
{
	if (!m_impl)
		throw std::logic_error("unexpected access to moved-from object");

	halco::common::Unique unique;

	// Set dls and soft reset
	haldls::v2::FlyspiConfig reset_config;
	reset_config.set_dls_reset(true);
	reset_config.set_soft_reset(true);
	ocp_write_container(m_impl->com, unique, reset_config);

	// Set default config
	ocp_write_container(m_impl->com, unique, haldls::v2::FlyspiConfig());
}

void LocalBoardControl::configure_static(
	std::vector<haldls::v2::ocp_address_type> const& board_addresses,
	std::vector<haldls::v2::ocp_word_type> const& board_words,
	std::vector<std::vector<haldls::v2::instruction_word_type> > const& chip_program_bytes)
{
	if (!m_impl)
		throw std::logic_error("unexpected access to moved-from object");

	// Write the board config
	ocp_write(m_impl->com, board_words, board_addresses);

	transfer(chip_program_bytes);
	execute();
}

void LocalBoardControl::configure_static(
	haldls::v2::Board const& board, haldls::v2::Chip const& chip)
{
	if (!m_impl)
		throw std::logic_error("unexpected access to moved-from object");

	// An experiment always happens as follows:
	// * Set the board config, including DACs, spike router and FPGA config
	// * Set the chip config and wait for the cap-mem to settle

	halco::common::Unique const coord;

	// Set the board
	ocp_write_container(m_impl->com, coord, board);

	// If the dls is in reset during playback of a playback program, the FPGA
	// will never stop execution for v2 and freeze the FPGA. Therefore, the
	// playback of programs is prohibited if the DLS is in reset.
	// TODO: Move this check to LocalBoardControl::execute()?? This adds
	// additional overhead in the execution of experiments
	if (board.get_flyspi_config().get_dls_reset()) {
		auto log = log4cxx::Logger::getLogger(__func__);
		LOG4CXX_WARN(log, "DLS in reset during configuration");
		LOG4CXX_WARN(log, "The chip configuration cannot be written");
	} else {
		auto setup = get_configure_program(chip);

		run(setup);
	}
}

haldls::v2::PlaybackProgram get_configure_program(haldls::v2::Chip chip)
{
	// Chip configuration program
	haldls::v2::PlaybackProgramBuilder setup_builder;
	setup_builder.set_time(0);
	setup_builder.write(halco::common::Unique(), chip);
	// Wait for the cap-mem to settle (based on empirical measurement by DS)
	// clang-format off
	setup_builder.wait_for(2'000'000); // ~ 20.8 ms for 96 MHz
	// clang-format on
	setup_builder.halt();
	return setup_builder.done();
}


void LocalBoardControl::transfer(
	std::vector<std::vector<haldls::v2::instruction_word_type> > const& program_bytes)
{
	if (!m_impl)
		throw std::logic_error("unexpected access to moved-from object");

	// vvv ------8<----------- (legacy code copied from frickel-dls)

	using namespace rw_api::flyspi;

	std::vector<SdramBlockWriteQuery> queries;
	std::vector<SdramRequest> reqs;
	Sdram_block_write_allocator alloc(m_impl->com, m_impl->program_address);

	// copy to USB buffer memory and transfer
	for (auto const& container : program_bytes) {
		queries.push_back(alloc.allocate(container.size() / 4));

		auto it_in = std::begin(container);
		auto it_out = uni::bytewise(std::begin(queries.back()));
		while (it_in != std::end(container)) {
			*it_out = *it_in;
			++it_in;
			++it_out;
		}

		reqs.push_back(queries.back().commit());
	}

	// wait for completion of transfer
	for (auto& req : reqs) {
		req.wait();
	}

	// ^^^ ------8<-----------

	m_impl->program_size = alloc.address - m_impl->program_address;

	// write program address, size and result pointer
	haldls::v2::FlyspiProgramAddress program_address(m_impl->program_address);
	haldls::v2::FlyspiProgramSize program_size(m_impl->program_size);
	haldls::v2::FlyspiResultAddress result_address(m_impl->result_address);

	halco::common::Unique unique;

	ocp_write_container(m_impl->com, unique, program_address);
	ocp_write_container(m_impl->com, unique, program_size);
	ocp_write_container(m_impl->com, unique, result_address);
}

void LocalBoardControl::transfer(haldls::v2::PlaybackProgram const& playback_program)
{
	if (!m_impl) {
		throw std::logic_error("unexpected access to moved-from object");
	}

	if (playback_program.serial_number() == haldls::v2::PlaybackProgram::invalid_serial_number) {
		throw std::logic_error("trying to transfer program with invalid state");
	}

	m_impl->program_serial_number = playback_program.serial_number();
	transfer(playback_program.instruction_byte_blocks());
}

void LocalBoardControl::execute()
{
	auto log = log4cxx::Logger::getLogger(__func__);

	if (!m_impl)
		throw std::logic_error("unexpected access to moved-from object");

	if (m_impl->program_size == 0)
		throw std::runtime_error("execute: no valid playback program has been transferred yet");

	halco::common::Unique unique;

	// check that the DLS is not in reset
	auto config = ocp_read_container<haldls::v2::FlyspiConfig>(m_impl->com, unique);
	if (config.get_dls_reset()) {
		LOG4CXX_ERROR(log, "Asking to execute a program although the DLS is in reset.");
		LOG4CXX_ERROR(log, "This is prohibited for v2 as it will freeze the system.");
		throw haldls::exception::InvalidConfiguration(
			"Refuse to execute playback program with DLSv2 in reset");
	}

	// start execution by setting the execute bit
	haldls::v2::FlyspiControl control;
	control.set_execute(true);
	LOG4CXX_DEBUG(log, "start execution");
	ocp_write_container(m_impl->com, unique, control);

	// wait until execute bit is cleared again
	{
		std::chrono::microseconds sleep_till_poll(50);
		while (control.get_execute()) {
			LOG4CXX_DEBUG(
				log, "execute flag not yet cleared, sleep for " << sleep_till_poll.count() << "us");
			std::this_thread::sleep_for(sleep_till_poll);
			control = ocp_read_container<haldls::v2::FlyspiControl>(m_impl->com, unique);

			// wait up to one minute for the execute flag to clear (should this
			// be an adjustable constant? one minute seems far far more than
			// any realistic experiment execution time
			// --obreitwi, 23-05-18 15:26:47)
			if (sleep_till_poll.count() > 60 /* s */ * 1000 /* ms */ * 1000 /* us */) {
				LOG4CXX_ERROR(log, "execute flag not cleared for 1 minute, aborting!");
				auto exception =
					ocp_read_container<haldls::v2::FlyspiException>(m_impl->com, unique);
				LOG4CXX_ERROR(log, exception)
				break;
			}
			// Increase exponential sleep time
			sleep_till_poll *= 2;
		}
	}
	LOG4CXX_DEBUG(log, "execution finished");
}

std::vector<haldls::v2::instruction_word_type> LocalBoardControl::fetch()
{
	auto log = log4cxx::Logger::getLogger(__func__);
	if (!m_impl)
		throw std::logic_error("unexpected access to moved-from object");

	if (m_impl->program_size == 0)
		throw std::runtime_error("fetch: no valid playback program has been transferred yet");

	// get result size
	halco::common::Unique unique;
	auto result_size = ocp_read_container<haldls::v2::FlyspiResultSize>(m_impl->com, unique);
	if (!result_size.get_value()) {
		throw std::logic_error("no result size read from board");
	}
	if (result_size.get_value().value() > rw_api::FlyspiCom::SdramChannel::max_size) {
		throw std::logic_error(
			"to be read back data(" + std::to_string(result_size.get_value().value()) +
			") exceeds FPGA memory(" + std::to_string(rw_api::FlyspiCom::SdramChannel::max_size) + ")");
	}
	auto exception = ocp_read_container<haldls::v2::FlyspiException>(m_impl->com, unique);
	if (!exception.check().value()) {
		LOG4CXX_ERROR(log, "FPGA exception raised: " << exception);
		throw std::logic_error("FPGA exception raised, aborting fetching");
	}

	// vvv ------8<----------- (legacy code copied from frickel-dls)

	using namespace rw_api::flyspi;

	// transfer data back
	auto loc = (m_impl->com).locate().chip(0);
	SdramBlockReadQuery q_read((m_impl->com), loc, result_size.get_value().value());
	q_read.addr(0x08000000 + m_impl->result_address);

	auto r_read = q_read.commit();
	r_read.wait();

	// ^^^ ------8<-----------

	// extract read/write results from data

	std::vector<haldls::v2::instruction_word_type> bytes;
	std::copy(
		uni::raw_byte_iterator<rw_api::FlyspiCom::BufferType>(std::begin(r_read)),
		uni::raw_byte_iterator<rw_api::FlyspiCom::BufferType>(std::end(r_read)),
		std::back_inserter(bytes));
	return bytes;
}

void LocalBoardControl::fetch(haldls::v2::PlaybackProgram& playback_program)
{
	if (!m_impl)
		throw std::logic_error("unexpected access to moved-from object");

	if (m_impl->program_serial_number != playback_program.serial_number())
		throw std::runtime_error("Different playback program as transferred to chip");
	decode_result_bytes(fetch(), playback_program);
}

void LocalBoardControl::decode_result_bytes(
	std::vector<haldls::v2::instruction_word_type> const& result_bytes,
	haldls::v2::PlaybackProgram& playback_program)
{
	UniDecoder decoder;
	uni::decode(result_bytes.begin(), result_bytes.end(), decoder);
	playback_program.set_results(std::move(decoder.words));
	playback_program.set_spikes(std::move(decoder.spikes));
}

std::vector<haldls::v2::instruction_word_type> LocalBoardControl::run(
	std::vector<std::vector<haldls::v2::instruction_word_type> > const& program_bytes)
{
	transfer(program_bytes);
	execute();
	return fetch();
}

void LocalBoardControl::run(haldls::v2::PlaybackProgram& playback_program)
{
	transfer(playback_program);
	execute();
	fetch(playback_program);
}

void LocalBoardControl::run_experiment(
	haldls::v2::Board const& board,
	haldls::v2::Chip const& chip,
	haldls::v2::PlaybackProgram& playback_program)
{
	configure_static(board, chip);
	run(playback_program);
}

std::vector<std::string> available_board_usb_serial_numbers()
{
	std::vector<std::string> result;
	char const* const ptr = std::getenv("SLURM_FLYSPI_ID");
	if (ptr == nullptr)
		return result;
	std::istringstream board_ids(ptr);
	std::string token;
	while (std::getline(board_ids, token, ','))
		result.push_back(token);
	return result;
}

} // namespace v2
} // namespace stadls
