#pragma once

#include <cstdint>

#include "halco/common/genpybind.h"
#include "halco/common/typed_array.h"
#include "halco/hicann-dls/v2/coordinates.h"

#include "hate/visibility.h"
#include "haldls/v2/common.h"

namespace haldls {
namespace v2 GENPYBIND(tag(haldls_v2)) {

class GENPYBIND(visible) CapMemCell
{
public:
	typedef halco::hicann_dls::v2::CapMemCellOnDLS coordinate_type;
	typedef std::true_type is_leaf_node;

	struct GENPYBIND(inline_base("*")) Value
		: public halco::common::detail::RantWrapper<Value, uint_fast16_t, 1023, 0>
	{
		constexpr explicit Value(uintmax_t const val = 0) SYMBOL_VISIBLE : rant_t(val) {}
	};

	CapMemCell() SYMBOL_VISIBLE;
	explicit CapMemCell(Value const& value) SYMBOL_VISIBLE;

	Value get_value() const SYMBOL_VISIBLE;
	void set_value(Value const& value) SYMBOL_VISIBLE;

	static size_t constexpr config_size_in_words GENPYBIND(hidden) = 1;
	std::array<hardware_address_type, config_size_in_words> addresses(
		coordinate_type const& cell) const SYMBOL_VISIBLE GENPYBIND(hidden);
	std::array<hardware_word_type, config_size_in_words> encode() const SYMBOL_VISIBLE
		GENPYBIND(hidden);
	void decode(std::array<hardware_word_type, config_size_in_words> const& data) SYMBOL_VISIBLE
		GENPYBIND(hidden);

	bool operator==(CapMemCell const& other) const SYMBOL_VISIBLE;
	bool operator!=(CapMemCell const& other) const SYMBOL_VISIBLE;

private:
	Value m_value;
};

class GENPYBIND(visible) CapMem
{
public:
	typedef halco::hicann_dls::v2::CapMemOnDLS coordinate_type;
	typedef std::false_type has_local_data;

	/// \brief Default constructor, yielding safe default values.
	CapMem() SYMBOL_VISIBLE;

	CapMemCell::Value get(halco::hicann_dls::v2::CapMemCellOnDLS const& cell) const SYMBOL_VISIBLE;
	void set(halco::hicann_dls::v2::CapMemCellOnDLS const& cell, CapMemCell::Value const& value) SYMBOL_VISIBLE;

	CapMemCell::Value get(
		halco::hicann_dls::v2::NeuronOnDLS const& neuron,
		halco::hicann_dls::v2::NeuronParameter const& neuron_parameter) const SYMBOL_VISIBLE;
	void set(
		halco::hicann_dls::v2::NeuronOnDLS const& neuron,
		halco::hicann_dls::v2::NeuronParameter const& neuron_parameter,
		CapMemCell::Value const& value) SYMBOL_VISIBLE;
	CapMemCell::Value get(
		halco::hicann_dls::v2::CommonNeuronParameter const& common_parameter) const SYMBOL_VISIBLE;
	void set(
		halco::hicann_dls::v2::CommonNeuronParameter const& common_parameter,
		CapMemCell::Value const& value) SYMBOL_VISIBLE;

	bool operator==(CapMem const& other) const SYMBOL_VISIBLE;
	bool operator!=(CapMem const& other) const SYMBOL_VISIBLE;

	friend detail::VisitPreorderImpl<CapMem>;

private:
	halco::common::typed_array<CapMemCell, halco::hicann_dls::v2::CapMemCellOnDLS> m_capmem_cells;
};


class GENPYBIND(visible) CapMemConfig
{
public:
	typedef halco::hicann_dls::v2::CapMemConfigOnDLS coordinate_type;
	typedef std::true_type is_leaf_node;

	struct GENPYBIND(inline_base("*")) OutAmpBias
		: public halco::common::detail::RantWrapper<OutAmpBias, uint_fast16_t, 15, 0>
	{
		constexpr explicit OutAmpBias(uintmax_t const val = 0) SYMBOL_VISIBLE : rant_t(val) {}
	};
	struct GENPYBIND(inline_base("*")) SourceFollowerBias
		: public halco::common::detail::RantWrapper<SourceFollowerBias, uint_fast16_t, 15, 0>
	{
		constexpr explicit SourceFollowerBias(uintmax_t const val = 0) SYMBOL_VISIBLE : rant_t(val) {}
	};
	struct GENPYBIND(inline_base("*")) LevelShifterBias
		: public halco::common::detail::RantWrapper<LevelShifterBias, uint_fast16_t, 15, 0>
	{
		constexpr explicit LevelShifterBias(uintmax_t const val = 0) SYMBOL_VISIBLE : rant_t(val) {}
	};

	struct GENPYBIND(inline_base("*")) VGlobalBias
		: public halco::common::detail::RantWrapper<VGlobalBias, uint_fast16_t, 15, 0>
	{
		constexpr explicit VGlobalBias(uintmax_t const val = 0) SYMBOL_VISIBLE : rant_t(val) {}
	};
	struct GENPYBIND(inline_base("*")) CurrentCellRes
		: public halco::common::detail::RantWrapper<CurrentCellRes, uint_fast16_t, 63, 0>
	{
		constexpr explicit CurrentCellRes(uintmax_t const val = 0) SYMBOL_VISIBLE : rant_t(val) {}
	};
	struct GENPYBIND(inline_base("*")) BoostFactor
		: public halco::common::detail::RantWrapper<BoostFactor, uint_fast16_t, 15, 0>
	{
		constexpr explicit BoostFactor(uintmax_t const val = 0) SYMBOL_VISIBLE : rant_t(val) {}
	};
	struct GENPYBIND(inline_base("*")) PrescalePause
		: public halco::common::detail::RantWrapper<PrescalePause, uint_fast16_t, 6, 0>
	{
		constexpr explicit PrescalePause(uintmax_t const val = 0) SYMBOL_VISIBLE : rant_t(val) {}
	};
	struct GENPYBIND(inline_base("*")) PrescaleRamp
		: public halco::common::detail::RantWrapper<PrescaleRamp, uint_fast16_t, 6, 0>
	{
		constexpr explicit PrescaleRamp(uintmax_t const val = 0) SYMBOL_VISIBLE : rant_t(val) {}
	};
	struct GENPYBIND(inline_base("*")) SubCounter
		: public halco::common::detail::
			  RantWrapper<SubCounter, uint_fast16_t, 65535 /* 2^16*-1 */, 0>
	{
		constexpr explicit SubCounter(uintmax_t const val = 0) SYMBOL_VISIBLE : rant_t(val) {}
	};
	struct GENPYBIND(inline_base("*")) PauseCounter
		: public halco::common::detail::
			  RantWrapper<PauseCounter, uint_fast32_t, 4294967295 /* 2^32-1 */, 0>
	{
		constexpr explicit PauseCounter(uintmax_t const val = 0) SYMBOL_VISIBLE : rant_t(val) {}
	};
	struct GENPYBIND(inline_base("*")) PulseA
		: public halco::common::detail::RantWrapper<PulseA, uint_fast16_t, 65535, 0>
	{
		constexpr explicit PulseA(uintmax_t const val = 0) SYMBOL_VISIBLE : rant_t(val) {}
	};
	struct GENPYBIND(inline_base("*")) PulseB
		: public halco::common::detail::RantWrapper<PulseB, uint_fast16_t, 65535, 0>
	{
		constexpr explicit PulseB(uintmax_t const val = 0) SYMBOL_VISIBLE : rant_t(val) {}
	};
	struct GENPYBIND(inline_base("*")) BoostA
		: public halco::common::detail::RantWrapper<BoostA, uint_fast16_t, 65535, 0>
	{
		constexpr explicit BoostA(uintmax_t const val = 0) SYMBOL_VISIBLE : rant_t(val) {}
	};
	struct GENPYBIND(inline_base("*")) BoostB
		: public halco::common::detail::RantWrapper<BoostB, uint_fast16_t, 65535, 0>
	{
		constexpr explicit BoostB(uintmax_t const val = 0) SYMBOL_VISIBLE : rant_t(val) {}
	};

	enum class VRefSelect : uint_fast8_t
	{
		disabled = 0,
		v_ref_v = 1,
		v_ref_i = 2
	};

	enum class IOutSelect : uint_fast8_t
	{
		disabled = 0,
		i_out_mux = 1,
		i_out_ramp = 2
	};

	CapMemConfig() SYMBOL_VISIBLE;

	bool get_enable_capmem() const SYMBOL_VISIBLE;
	void set_enable_capmem(bool const value) SYMBOL_VISIBLE;

	bool get_debug_readout_enable() const SYMBOL_VISIBLE;
	void set_debug_readout_enable(bool const value) SYMBOL_VISIBLE;

	halco::hicann_dls::v2::CapMemCellOnDLS get_debug_capmem_coord() const SYMBOL_VISIBLE;
	void set_debug_capmem_coord(halco::hicann_dls::v2::CapMemCellOnDLS const& value) SYMBOL_VISIBLE;

	VRefSelect get_debug_v_ref_select() const SYMBOL_VISIBLE;
	void set_debug_v_ref_select(VRefSelect const& value) SYMBOL_VISIBLE;

	IOutSelect get_debug_i_out_select() const SYMBOL_VISIBLE;
	void set_debug_i_out_select(IOutSelect const& value) SYMBOL_VISIBLE;

	OutAmpBias get_debug_out_amp_bias() const SYMBOL_VISIBLE;
	void set_debug_out_amp_bias(OutAmpBias const& value) SYMBOL_VISIBLE;

	SourceFollowerBias get_debug_source_follower_bias() const SYMBOL_VISIBLE;
	void set_debug_source_follower_bias(SourceFollowerBias const& value) SYMBOL_VISIBLE;

	LevelShifterBias get_debug_level_shifter_bias() const SYMBOL_VISIBLE;
	void set_debug_level_shifter_bias(LevelShifterBias const& value) SYMBOL_VISIBLE;

	VGlobalBias get_v_global_bias() const SYMBOL_VISIBLE;
	void set_v_global_bias(VGlobalBias const& value) SYMBOL_VISIBLE;

	CurrentCellRes get_current_cell_res() const SYMBOL_VISIBLE;
	void set_current_cell_res(CurrentCellRes const& value) SYMBOL_VISIBLE;

	BoostFactor get_boost_factor() const SYMBOL_VISIBLE;
	void set_boost_factor(BoostFactor const& value) SYMBOL_VISIBLE;

	bool get_enable_boost() const SYMBOL_VISIBLE;
	void set_enable_boost(bool const value) SYMBOL_VISIBLE;

	bool get_enable_autoboost() const SYMBOL_VISIBLE;
	void set_enable_autoboost(bool const value) SYMBOL_VISIBLE;

	PrescalePause get_prescale_pause() const SYMBOL_VISIBLE;
	void set_prescale_pause(PrescalePause const& value) SYMBOL_VISIBLE;

	PrescaleRamp get_prescale_ramp() const SYMBOL_VISIBLE;
	void set_prescale_ramp(PrescaleRamp const& value) SYMBOL_VISIBLE;

	SubCounter get_sub_counter() const SYMBOL_VISIBLE;
	void set_sub_counter(SubCounter const& value) SYMBOL_VISIBLE;

	PauseCounter get_pause_counter() const SYMBOL_VISIBLE;
	void set_pause_counter(PauseCounter const& value) SYMBOL_VISIBLE;

	PulseA get_pulse_a() const SYMBOL_VISIBLE;
	void set_pulse_a(PulseA const& value) SYMBOL_VISIBLE;

	PulseB get_pulse_b() const SYMBOL_VISIBLE;
	void set_pulse_b(PulseB const& value) SYMBOL_VISIBLE;

	BoostA get_boost_a() const SYMBOL_VISIBLE;
	void set_boost_a(BoostA const& value) SYMBOL_VISIBLE;

	BoostB get_boost_b() const SYMBOL_VISIBLE;
	void set_boost_b(BoostB const& value) SYMBOL_VISIBLE;

	static size_t constexpr config_size_in_words GENPYBIND(hidden) = 10;
	std::array<hardware_address_type, config_size_in_words> addresses(
		coordinate_type const& unique) const SYMBOL_VISIBLE GENPYBIND(hidden);
	std::array<hardware_word_type, config_size_in_words> encode() const SYMBOL_VISIBLE GENPYBIND(hidden);
	void decode(std::array<hardware_word_type, config_size_in_words> const& data) SYMBOL_VISIBLE GENPYBIND(hidden);

	bool operator==(CapMemConfig const& other) const SYMBOL_VISIBLE;
	bool operator!=(CapMemConfig const& other) const SYMBOL_VISIBLE;

private:
	bool m_enable_capmem;
	bool m_debug_readout_enable;
	halco::hicann_dls::v2::CapMemCellOnDLS m_debug_capmem_coord;
	VRefSelect m_debug_v_ref_select;
	IOutSelect m_debug_i_out_select;
	OutAmpBias m_debug_out_amp_bias;
	SourceFollowerBias m_debug_source_follower_bias;
	LevelShifterBias m_debug_level_shifter_bias;
	VGlobalBias m_v_global_bias;
	CurrentCellRes m_current_cell_res;
	bool m_enable_boost;
	BoostFactor m_boost_factor;
	bool m_enable_autoboost;
	PrescalePause m_prescale_pause;
	PrescaleRamp m_prescale_ramp;
	SubCounter m_sub_counter;
	PauseCounter m_pause_counter;
	PulseA m_pulse_a;
	PulseB m_pulse_b;
	BoostA m_boost_a;
	BoostB m_boost_b;
};

namespace detail {

template <>
struct VisitPreorderImpl<CapMem> {
	template <typename ContainerT, typename VisitorT>
	static void call(ContainerT& config, halco::hicann_dls::v2::CapMemOnDLS const& coord, VisitorT&& visitor)
	{
		using halco::common::iter_all;
		using namespace halco::hicann_dls::v2;

		visitor(coord, config);

		for (auto const cell : iter_all<CapMemCellOnDLS>()) {
			// No std::forward for visitor argument, as we want to pass a reference to the
			// nested visitor in any case, even if it was passed as an rvalue to this function.
			visit_preorder(config.m_capmem_cells[cell], cell, visitor);
		}
	}
};

} // namespace detail

} // namespace v2
} // namespace haldls

namespace std {

HALCO_GEOMETRY_HASH_CLASS(haldls::v2::CapMemCell::Value)
HALCO_GEOMETRY_HASH_CLASS(haldls::v2::CapMemConfig::OutAmpBias)
HALCO_GEOMETRY_HASH_CLASS(haldls::v2::CapMemConfig::SourceFollowerBias)
HALCO_GEOMETRY_HASH_CLASS(haldls::v2::CapMemConfig::LevelShifterBias)
HALCO_GEOMETRY_HASH_CLASS(haldls::v2::CapMemConfig::VGlobalBias)
HALCO_GEOMETRY_HASH_CLASS(haldls::v2::CapMemConfig::CurrentCellRes)
HALCO_GEOMETRY_HASH_CLASS(haldls::v2::CapMemConfig::BoostFactor)
HALCO_GEOMETRY_HASH_CLASS(haldls::v2::CapMemConfig::PrescalePause)
HALCO_GEOMETRY_HASH_CLASS(haldls::v2::CapMemConfig::PrescaleRamp)
HALCO_GEOMETRY_HASH_CLASS(haldls::v2::CapMemConfig::SubCounter)
HALCO_GEOMETRY_HASH_CLASS(haldls::v2::CapMemConfig::PauseCounter)
HALCO_GEOMETRY_HASH_CLASS(haldls::v2::CapMemConfig::PulseA)
HALCO_GEOMETRY_HASH_CLASS(haldls::v2::CapMemConfig::PulseB)
HALCO_GEOMETRY_HASH_CLASS(haldls::v2::CapMemConfig::BoostA)
HALCO_GEOMETRY_HASH_CLASS(haldls::v2::CapMemConfig::BoostB)

} // namespace std
