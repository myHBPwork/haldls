#include <gtest/gtest.h>

#include "haldls/v2/spike.h"

using namespace haldls::v2;
using namespace halco::hicann_dls::v2;
using namespace halco::common;

TEST(PlaybackSpike, General)
{
	hardware_time_type time_0 = 20;
	PlaybackSpike spike{time_0, SynapseBlock::Synapse::Address(10), SynapseDriverOnDLS(15)};

	hardware_time_type time_1 = 40;

	// test getter/setter
	ASSERT_EQ(spike.get_time(), time_0);
	ASSERT_EQ(spike.get_source_address(), SynapseBlock::Synapse::Address(10));
	ASSERT_EQ(spike.get_synapse_driver(), SynapseDriverOnDLS(15));

	PlaybackSpike spike2 = spike;
	PlaybackSpike spike3(time_1, SynapseBlock::Synapse::Address(5), SynapseDriverOnDLS(5));
	ASSERT_EQ(spike3.get_time(), time_1);
	ASSERT_EQ(spike3.get_source_address(), SynapseBlock::Synapse::Address(5));
	ASSERT_EQ(spike3.get_synapse_driver(), SynapseDriverOnDLS(5));

	PlaybackSpike spike4(time_1, SynapseBlock::Synapse::Address(2), SynapseDriverOnDLS(5));

	PlaybackSpike spike5(time_1, SynapseBlock::Synapse::Address(2), SynapseDriverOnDLS(2));

	ASSERT_EQ(spike, spike2);
	ASSERT_EQ(spike == spike3, false);

	ASSERT_NE(spike, spike3);
	ASSERT_EQ(spike != spike2, false);

	ASSERT_LT(spike, spike3);
	ASSERT_EQ(spike3 < spike, false);

	ASSERT_GT(spike3, spike);
	ASSERT_EQ(spike > spike3, false);

	ASSERT_GE(spike3, spike);
	ASSERT_GE(spike, spike2);
	ASSERT_EQ(spike >= spike3, false);

	ASSERT_LE(spike, spike3);
	ASSERT_LE(spike, spike2);
	ASSERT_EQ(spike3 <= spike, false);

	ASSERT_GT(spike3, spike4);
	ASSERT_GT(spike3, spike5);

	ASSERT_LT(spike4, spike3);
	ASSERT_LT(spike5, spike3);
}

TEST(RecordedSpike, General)
{
	hardware_time_type time_0 = 20;
	RecordedSpike spike{time_0, NeuronOnDLS(10)};

	hardware_time_type time_1 = 40;

	// test getter/setter
	ASSERT_EQ(spike.get_time(), time_0);
	ASSERT_EQ(spike.get_neuron(), NeuronOnDLS(10));

	RecordedSpike spike2 = spike;
	RecordedSpike spike3(time_1, NeuronOnDLS(5));
	ASSERT_EQ(spike3.get_time(), time_1);
	ASSERT_EQ(spike3.get_neuron(), NeuronOnDLS(5));

	RecordedSpike spike4(time_1, NeuronOnDLS(2));

	ASSERT_EQ(spike, spike2);
	ASSERT_EQ(spike == spike3, false);

	ASSERT_NE(spike, spike3);
	ASSERT_EQ(spike != spike2, false);

	ASSERT_LT(spike, spike3);
	ASSERT_EQ(spike3 < spike, false);

	ASSERT_GT(spike3, spike);
	ASSERT_EQ(spike > spike3, false);

	ASSERT_GE(spike3, spike);
	ASSERT_GE(spike, spike2);
	ASSERT_EQ(spike >= spike3, false);

	ASSERT_LE(spike, spike3);
	ASSERT_LE(spike, spike2);
	ASSERT_EQ(spike3 <= spike, false);

	ASSERT_GT(spike3, spike4);

	ASSERT_LT(spike4, spike3);
}
