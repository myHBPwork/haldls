#include "haldls/v2/capmem.h"

#include <numeric>
#include <utility>

#include "halco/common/iter_all.h"
#include "halco/common/typed_array.h"

namespace haldls {
namespace v2 {

using halco::common::typed_array;
using namespace halco::hicann_dls::v2;

static const typed_array<uint_fast16_t, CapMemRowOnDLS> default_neuron_parameters{{
	300,  //  0: v_leak
	600,  //  1: v_treshold
	690,  //  2: v_exc_syn_input_reference
	690,  //  3: v_inh_syn_input_reference
	0,    //  4: unused
	0,    //  5: unused
	0,    //  6: unused
	0,    //  7: unused
	0,    //  8: unused
	650,  //  9: i_bias_spike_comparator
	130,  // 10: i_spike_comparator_delay
	1022, // 11: i_bias_leak_main
	1022, // 12: i_bias_leak_sd
	0,    // 13: i_bias_readout_buffer
	300,  // 14: i_refractory_time
	0,    // 15: i_bias_exc_syn_input_main
	0,    // 16: i_bias_exc_syn_input_sd
	500,  // 17: i_bias_exc_syn_input_resistor
	0,    // 18: i_bias_exc_syn_input_offset
	500,  // 19: i_bias_inh_syn_input_resistor
	0,    // 20: i_bias_inh_syn_input_main
	0,    // 21: unused
	0,    // 22: i_bias_inh_syn_input_sd
	0     // 23: i_bias_inh_syn_input_offset
}};

static const typed_array<uint_fast16_t, CapMemRowOnDLS> default_shared_neuron_parameters{{
    100, // e_reset
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0,   // unused
    0    // unused
}};

CapMemCell::CapMemCell()
	: CapMemCell(CapMemCell::Value(0))
{
}

CapMemCell::CapMemCell(CapMemCell::Value const& value) : m_value(value) {}

CapMemCell::Value CapMemCell::get_value() const
{
	return m_value;
}

void CapMemCell::set_value(CapMemCell::Value const& value)
{
	m_value = value;
}

std::array<hardware_address_type, CapMemCell::config_size_in_words> CapMemCell::addresses(coordinate_type const& cell) const
{
	hardware_address_type constexpr base_address = 0x18000000;
	hardware_address_type constexpr row_stride = 32;
	return {{static_cast<hardware_address_type>(
		base_address + row_stride * cell.toCapMemColumnOnDLS() + cell.toCapMemRowOnDLS())}};
}

std::array<hardware_word_type, CapMemCell::config_size_in_words> CapMemCell::encode() const
{
	return {{static_cast<hardware_word_type>(get_value())}};
}

void CapMemCell::decode(std::array<hardware_word_type, CapMemCell::config_size_in_words> const& data)
{
	set_value(Value(data[0]));
}

bool CapMemCell::operator==(CapMemCell const& other) const
{
	return m_value == other.get_value();
}

bool CapMemCell::operator!=(CapMemCell const& other) const
{
	return !(*this == other);
}

CapMem::CapMem()
{
	for (auto column : halco::common::iter_all<CapMemColumnOnDLS>()) {
		for (auto row : halco::common::iter_all<CapMemRowOnDLS>()) {
			if (column.isShared()) {
				m_capmem_cells.at(CapMemCellOnDLS(column, row)) =
				    CapMemCell(CapMemCell::Value(default_shared_neuron_parameters.at(row)));
			} else {
				m_capmem_cells.at(CapMemCellOnDLS(column, row)) =
				    CapMemCell(CapMemCell::Value(default_neuron_parameters.at(row)));
			}
		}
	}
}

CapMemCell::Value CapMem::get(CapMemCellOnDLS const& coord) const
{
	return m_capmem_cells.at(coord).get_value();
}

void CapMem::set(CapMemCellOnDLS const& coord, CapMemCell::Value const& value)
{
	m_capmem_cells.at(coord).set_value(value);
}

CapMemCell::Value CapMem::get(
	NeuronOnDLS const& neuron, NeuronParameter const& neuron_parameter) const
{
	return m_capmem_cells.at(CapMemCellOnDLS(neuron, neuron_parameter)).get_value();
}

void CapMem::set(
	NeuronOnDLS const& neuron,
	NeuronParameter const& neuron_parameter,
	CapMemCell::Value const& value)
{
	m_capmem_cells.at(CapMemCellOnDLS(neuron, neuron_parameter)).set_value(value);
}

CapMemCell::Value CapMem::get(CommonNeuronParameter const& common_parameter) const
{
	return m_capmem_cells.at(CapMemCellOnDLS(common_parameter)).get_value();
}

void CapMem::set(CommonNeuronParameter const& common_parameter, CapMemCell::Value const& value)
{
	m_capmem_cells.at(CapMemCellOnDLS(common_parameter)).set_value(value);
}

bool CapMem::operator==(CapMem const& other) const
{
	return m_capmem_cells == other.m_capmem_cells;
}

bool CapMem::operator!=(CapMem const& other) const
{
	return !(*this == other);
}

CapMemConfig::CapMemConfig()
	: m_enable_capmem(true),
	  m_debug_readout_enable(false),
	  m_debug_capmem_coord(CapMemCellOnDLS()),
	  m_debug_v_ref_select(CapMemConfig::VRefSelect::disabled),
	  m_debug_i_out_select(CapMemConfig::IOutSelect::disabled),
	  m_debug_out_amp_bias(7),
	  m_debug_source_follower_bias(3),
	  m_debug_level_shifter_bias(3),
	  m_v_global_bias(7),
	  m_current_cell_res(8),
	  m_enable_boost(false),
	  m_boost_factor(0),
	  m_enable_autoboost(false),
	  m_prescale_pause(3),
	  m_prescale_ramp(3),
	  m_sub_counter(8),
	  m_pause_counter(8096),
	  m_pulse_a(4),
	  m_pulse_b(7),
	  m_boost_a(0),
	  m_boost_b(0)
{
}

bool CapMemConfig::get_enable_capmem() const
{
	return m_enable_capmem;
}

void CapMemConfig::set_enable_capmem(bool const value)
{
	m_enable_capmem = value;
}

bool CapMemConfig::get_debug_readout_enable() const
{
	return m_debug_readout_enable;
}

void CapMemConfig::set_debug_readout_enable(bool const value)
{
	m_debug_readout_enable = value;
}

CapMemCellOnDLS CapMemConfig::get_debug_capmem_coord() const
{
	return m_debug_capmem_coord;
}

void CapMemConfig::set_debug_capmem_coord(CapMemCellOnDLS const& value)
{
	m_debug_capmem_coord = value;
}

CapMemConfig::VRefSelect CapMemConfig::get_debug_v_ref_select() const
{
	return m_debug_v_ref_select;
}

void CapMemConfig::set_debug_v_ref_select(CapMemConfig::VRefSelect const& value)
{
	m_debug_v_ref_select = value;
}

CapMemConfig::IOutSelect CapMemConfig::get_debug_i_out_select() const
{
	return m_debug_i_out_select;
}

void CapMemConfig::set_debug_i_out_select(CapMemConfig::IOutSelect const& value)
{
	m_debug_i_out_select = value;
}

CapMemConfig::OutAmpBias CapMemConfig::get_debug_out_amp_bias() const
{
	return m_debug_out_amp_bias;
}

void CapMemConfig::set_debug_out_amp_bias(CapMemConfig::OutAmpBias const& value)
{
	m_debug_out_amp_bias = value;
}

CapMemConfig::SourceFollowerBias CapMemConfig::get_debug_source_follower_bias() const
{
	return m_debug_source_follower_bias;
}

void CapMemConfig::set_debug_source_follower_bias(CapMemConfig::SourceFollowerBias const& value)
{
	m_debug_source_follower_bias = value;
}

CapMemConfig::LevelShifterBias CapMemConfig::get_debug_level_shifter_bias() const
{
	return m_debug_level_shifter_bias;
}

void CapMemConfig::set_debug_level_shifter_bias(CapMemConfig::LevelShifterBias const& value)
{
	m_debug_level_shifter_bias = value;
}

CapMemConfig::VGlobalBias CapMemConfig::get_v_global_bias() const
{
	return m_v_global_bias;
}

void CapMemConfig::set_v_global_bias(CapMemConfig::VGlobalBias const& value)
{
	m_v_global_bias = value;
}

CapMemConfig::CurrentCellRes CapMemConfig::get_current_cell_res() const
{
	return m_current_cell_res;
}

void CapMemConfig::set_current_cell_res(CapMemConfig::CurrentCellRes const& value)
{
	m_current_cell_res = value;
}

CapMemConfig::BoostFactor CapMemConfig::get_boost_factor() const
{
	return m_boost_factor;
}

void CapMemConfig::set_boost_factor(CapMemConfig::BoostFactor const& value)
{
	m_boost_factor = value;
}

bool CapMemConfig::get_enable_boost() const
{
	return m_enable_boost;
}

void CapMemConfig::set_enable_boost(bool const value)
{
	m_enable_boost = value;
}

bool CapMemConfig::get_enable_autoboost() const
{
	return m_enable_autoboost;
}

void CapMemConfig::set_enable_autoboost(bool const value)
{
	m_enable_autoboost = value;
}

CapMemConfig::PrescalePause CapMemConfig::get_prescale_pause() const
{
	return m_prescale_pause;
}

void CapMemConfig::set_prescale_pause(CapMemConfig::PrescalePause const& value)
{
	m_prescale_pause = value;
}

CapMemConfig::PrescaleRamp CapMemConfig::get_prescale_ramp() const
{
	return m_prescale_ramp;
}

void CapMemConfig::set_prescale_ramp(CapMemConfig::PrescaleRamp const& value)
{
	m_prescale_ramp = value;
}

CapMemConfig::SubCounter CapMemConfig::get_sub_counter() const
{
	return m_sub_counter;
}

void CapMemConfig::set_sub_counter(CapMemConfig::SubCounter const& value)
{
	m_sub_counter = value;
}

CapMemConfig::PauseCounter CapMemConfig::get_pause_counter() const
{
	return m_pause_counter;
}

void CapMemConfig::set_pause_counter(CapMemConfig::PauseCounter const& value)
{
	m_pause_counter = value;
}

CapMemConfig::PulseA CapMemConfig::get_pulse_a() const
{
	return m_pulse_a;
}

void CapMemConfig::set_pulse_a(CapMemConfig::PulseA const& value)
{
	m_pulse_a = value;
}

CapMemConfig::PulseB CapMemConfig::get_pulse_b() const
{
	return m_pulse_b;
}

void CapMemConfig::set_pulse_b(CapMemConfig::PulseB const& value)
{
	m_pulse_b = value;
}

CapMemConfig::BoostA CapMemConfig::get_boost_a() const
{
	return m_boost_a;
}

void CapMemConfig::set_boost_a(CapMemConfig::BoostA const& value)
{
	m_boost_a = value;
}

CapMemConfig::BoostB CapMemConfig::get_boost_b() const
{
	return m_boost_b;
}

void CapMemConfig::set_boost_b(CapMemConfig::BoostB const& value)
{
	m_boost_b = value;
}

bool CapMemConfig::operator==(CapMemConfig const& other) const
{
	// clang-format off
	return (
		m_enable_capmem == other.get_enable_capmem() &&
		m_debug_readout_enable == other.get_debug_readout_enable() &&
		m_debug_capmem_coord == other.get_debug_capmem_coord() &&
		m_debug_v_ref_select == other.get_debug_v_ref_select() &&
		m_debug_i_out_select == other.get_debug_i_out_select() &&
		m_debug_out_amp_bias == other.get_debug_out_amp_bias() &&
		m_debug_source_follower_bias == other.get_debug_source_follower_bias() &&
		m_debug_level_shifter_bias == other.get_debug_level_shifter_bias() &&
		m_v_global_bias == other.get_v_global_bias() &&
		m_current_cell_res == other.get_current_cell_res() &&
		m_boost_factor == other.get_boost_factor() &&
		m_enable_boost == other.get_enable_boost() &&
		m_enable_autoboost == other.get_enable_autoboost() &&
		m_prescale_pause == other.get_prescale_pause() &&
		m_prescale_ramp == other.get_prescale_ramp() &&
		m_sub_counter == other.get_sub_counter() &&
		m_pause_counter == other.get_pause_counter() &&
		m_pulse_a == other.get_pulse_a() &&
		m_pulse_b == other.get_pulse_b() &&
		m_boost_a == other.get_boost_a() &&
		m_boost_b == other.get_boost_b());
	// clang-format on
}

bool CapMemConfig::operator!=(CapMemConfig const& other) const
{
	return !(*this == other);
}

namespace {

struct CapMemConfigBitfield
{
	typedef std::array<hardware_word_type, CapMemConfig::config_size_in_words> array_type;

	union
	{
		array_type raw;
		// clang-format off
		struct __attribute__((packed)) {
			hardware_word_type hotbit_capmem_row          : 32;

			hardware_word_type capmem_column              : 32;

			hardware_word_type debug_v_ref_select         : 32;

			hardware_word_type v_global_bias              :  4;
			hardware_word_type debug_level_shifter_bias   :  4;
			hardware_word_type debug_source_follower_bias :  4;
			hardware_word_type debug_out_amp_bias         :  4;
			hardware_word_type                            : 16;

			hardware_word_type current_cell_res           : 32;

			hardware_word_type debug_i_out_select         : 32;

			hardware_word_type sub_counter                : 16;
			hardware_word_type enable_capmem              :  1;
			hardware_word_type enable_boost               :  1;
			hardware_word_type enable_autoboost           :  1;
			hardware_word_type                            :  1;
			hardware_word_type prescale_ramp              :  4;
			hardware_word_type prescale_pause             :  4;
			hardware_word_type boost_factor               :  4;

			hardware_word_type pulse_b                    : 16;
			hardware_word_type pulse_a                    : 16;

			hardware_word_type boost_b                    : 16;
			hardware_word_type boost_a                    : 16;

			hardware_word_type pause_counter              : 32;
		} m;
		// clang-format on
		static_assert(sizeof(raw) == sizeof(m), "sizes of union types should match");
	} u;

	CapMemConfigBitfield() { u.raw = {{0}}; }
	CapMemConfigBitfield(array_type const& data) { u.raw = data; }
};

} // namespace

std::array<hardware_address_type, CapMemConfig::config_size_in_words>
CapMemConfig::addresses(coordinate_type const& /*unique*/) const
{
	hardware_address_type const base_address = 0x18010000;
	std::array<hardware_address_type, CapMemConfig::config_size_in_words> result;
	std::iota(result.begin(), result.end(), base_address);
	return result;
}

std::array<hardware_word_type, CapMemConfig::config_size_in_words>
CapMemConfig::encode() const
{
	CapMemConfigBitfield bitfield;
	if (m_debug_readout_enable) {
		auto capmem_coord = m_debug_capmem_coord;
		bitfield.u.m.hotbit_capmem_row = 1u << capmem_coord.toCapMemRowOnDLS();
		bitfield.u.m.capmem_column = capmem_coord.toCapMemColumnOnDLS();
	} else {
		bitfield.u.m.hotbit_capmem_row = 0;
		bitfield.u.m.capmem_column = 0;
	}
	bitfield.u.m.debug_v_ref_select = static_cast<hardware_word_type>(m_debug_v_ref_select);
	bitfield.u.m.debug_out_amp_bias = m_debug_out_amp_bias.value();
	bitfield.u.m.debug_source_follower_bias = m_debug_source_follower_bias.value();
	bitfield.u.m.debug_level_shifter_bias = m_debug_level_shifter_bias.value();
	bitfield.u.m.v_global_bias = m_v_global_bias.value();
	bitfield.u.m.current_cell_res = m_current_cell_res.value();
	bitfield.u.m.debug_i_out_select = static_cast<hardware_word_type>(m_debug_i_out_select);
	bitfield.u.m.boost_factor = m_boost_factor.value();
	bitfield.u.m.prescale_pause = m_prescale_pause.value();
	bitfield.u.m.prescale_ramp = m_prescale_ramp.value();
	bitfield.u.m.enable_autoboost = m_enable_autoboost;
	bitfield.u.m.enable_boost = m_enable_boost;
	bitfield.u.m.enable_capmem = m_enable_capmem;
	bitfield.u.m.sub_counter = m_sub_counter.value();
	bitfield.u.m.pulse_a = m_pulse_a.value();
	bitfield.u.m.pulse_b = m_pulse_b.value();
	bitfield.u.m.boost_a = m_boost_a.value();
	bitfield.u.m.boost_b = m_boost_b.value();
	bitfield.u.m.pause_counter = m_pause_counter.value();

	return bitfield.u.raw;
}

void CapMemConfig::decode(
	std::array<hardware_word_type, CapMemConfig::config_size_in_words> const& data)
{
	CapMemConfigBitfield bitfield(data);
	if (bitfield.u.m.hotbit_capmem_row) {
		m_debug_readout_enable = true;
		hardware_word_type hotbit = bitfield.u.m.hotbit_capmem_row;
		size_t position = 0;
		assert(hotbit != 0);
		while (hotbit >>= 1)
			++position;
		m_debug_capmem_coord = CapMemCellOnDLS(
			CapMemColumnOnDLS(bitfield.u.m.capmem_column), CapMemRowOnDLS(position));
	} else {
		m_debug_readout_enable = false;
		m_debug_capmem_coord = CapMemCellOnDLS();
	}
	m_debug_v_ref_select = VRefSelect(bitfield.u.m.debug_v_ref_select);
	m_debug_out_amp_bias = OutAmpBias(bitfield.u.m.debug_out_amp_bias);
	m_debug_source_follower_bias = SourceFollowerBias(bitfield.u.m.debug_source_follower_bias);
	m_debug_level_shifter_bias = LevelShifterBias(bitfield.u.m.debug_level_shifter_bias);
	m_v_global_bias = VGlobalBias(bitfield.u.m.v_global_bias);
	m_current_cell_res = CurrentCellRes(bitfield.u.m.current_cell_res);
	m_debug_i_out_select = IOutSelect(bitfield.u.m.debug_i_out_select);
	m_boost_factor = BoostFactor(bitfield.u.m.boost_factor);
	m_prescale_pause = PrescalePause(bitfield.u.m.prescale_pause);
	m_prescale_ramp = PrescaleRamp(bitfield.u.m.prescale_ramp);
	m_enable_autoboost = bitfield.u.m.enable_autoboost;
	m_enable_boost = bitfield.u.m.enable_boost;
	m_enable_capmem = bitfield.u.m.enable_capmem;
	m_sub_counter = SubCounter(bitfield.u.m.sub_counter);
	m_pulse_a = PulseA(bitfield.u.m.pulse_a);
	m_pulse_b = PulseB(bitfield.u.m.pulse_b);
	m_boost_a = BoostA(bitfield.u.m.boost_a);
	m_boost_b = BoostB(bitfield.u.m.boost_b);
	m_pause_counter = PauseCounter(bitfield.u.m.pause_counter);
}

} // namespace v2
} // namespace haldls
