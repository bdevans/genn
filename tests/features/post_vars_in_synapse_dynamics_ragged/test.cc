//--------------------------------------------------------------------------
/*! \file post_vars_in_synapse_dynamics_ragged/test.cc

\brief Main test code that is part of the feature testing
suite of minimal models with known analytic outcomes that are used for continuous integration testing.
*/
//--------------------------------------------------------------------------
// Google test includes
#include "gtest/gtest.h"

// Autogenerated simulation code includess
#include DEFINITIONS_HEADER

// **NOTE** base-class for simulation tests must be
// included after auto-generated globals are includes
#include "../../utils/simulation_test_vars.h"
#include "../../utils/simulation_neuron_policy_pre_post_var.h"
#include "../../utils/simulation_synapse_policy_ragged.h"

// Combine neuron and synapse policies together to build variable-testing fixture
typedef SimulationTestVars<SimulationNeuronPolicyPrePostVar, SimulationSynapsePolicyRagged> SimTest;

TEST_F(SimTest, PostVarsInSynapseDynamicsRagged)
{
    float err = Simulate(
        [](unsigned int, unsigned int, unsigned int j, float t, float &newX)
        {
            if (t > 0.0001+DT) {
                newX = t-2*DT+10*((j+1)%10);
                return true;
            }
            else {
                return false;
            }
        });

    // Check total error is less than some tolerance
    EXPECT_LT(err, 5e-3);
}
