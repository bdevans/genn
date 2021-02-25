// Google test includes
#include "gtest/gtest.h"

// GeNN includes
#include "modelSpecInternal.h"

//--------------------------------------------------------------------------
// Anonymous namespace
//--------------------------------------------------------------------------
namespace
{
class Sum : public CustomUpdateModels::Base
{
    DECLARE_CUSTOM_UPDATE_MODEL(Sum, 0, 1, 2);

    SET_UPDATE_CODE("$(sum) = $(a) + $(b);\n");

    SET_VARS({{"sum", "scalar"}});
    SET_VAR_REFS({{"a", "scalar", VarAccessMode::READ_ONLY}, 
                  {"b", "scalar", VarAccessMode::READ_ONLY}});
};
IMPLEMENT_MODEL(Sum);

class Sum2 : public CustomUpdateModels::Base
{
    DECLARE_CUSTOM_UPDATE_MODEL(Sum2, 0, 1, 2);

    SET_UPDATE_CODE("$(a) = $(mult) * ($(a) + $(b));\n");

    SET_VARS({{"mult", "scalar", VarAccess::READ_ONLY}});
    SET_VAR_REFS({{"a", "scalar", VarAccessMode::READ_WRITE}, 
                  {"b", "scalar", VarAccessMode::READ_WRITE}});
};
IMPLEMENT_MODEL(Sum2);


class Cont : public WeightUpdateModels::Base
{
public:
    DECLARE_WEIGHT_UPDATE_MODEL(Cont, 0, 1, 0, 0);

    SET_VARS({{"g", "scalar"}});

    SET_SYNAPSE_DYNAMICS_CODE(
        "$(addToInSyn, $(g) * $(V_pre));\n");
};
IMPLEMENT_MODEL(Cont);
}
//--------------------------------------------------------------------------
// Tests
//--------------------------------------------------------------------------
TEST(CustomUpdates, VarReferenceTypeChecks)
{
    ModelSpecInternal model;

    // Add two neuron group to model
    NeuronModels::Izhikevich::ParamValues paramVals(0.02, 0.2, -65.0, 8.0);
    NeuronModels::Izhikevich::VarValues varVals(0.0, 0.0);
    model.addNeuronPopulation<NeuronModels::Izhikevich>("Pre", 10, paramVals, varVals);
    model.addNeuronPopulation<NeuronModels::Izhikevich>("Post", 25, paramVals, varVals);

    auto *sg1 = model.addSynapsePopulation<WeightUpdateModels::StaticPulseDendriticDelay, PostsynapticModels::DeltaCurr>(
        "Synapses", SynapseMatrixType::DENSE_INDIVIDUALG, NO_DELAY,
        "Pre", "Post",
        {}, {1.0, 4},
        {}, {});

    Sum::VarValues sumVarValues(0.0);
    Sum::WUVarReferences sumVarReferences1(createWUVarRef(sg1, "g"), createWUVarRef(sg1, "g"));
    Sum::WUVarReferences sumVarReferences2(createWUVarRef(sg1, "g"), createWUVarRef(sg1, "d"));

    model.addCustomUpdate<Sum>("SumWeight1", "CustomUpdate",
                               {}, sumVarValues, sumVarReferences1);

    try {
        model.addCustomUpdate<Sum>("SumWeight2", "CustomUpdate",
                                   {}, sumVarValues, sumVarReferences2);
        FAIL();
    }
    catch(const std::runtime_error &) {
    }

    model.finalize();
}
//--------------------------------------------------------------------------
TEST(CustomUpdates, VarSizeChecks)
{
    ModelSpecInternal model;

    // Add two neuron group to model
    NeuronModels::Izhikevich::ParamValues paramVals(0.02, 0.2, -65.0, 8.0);
    NeuronModels::Izhikevich::VarValues varVals(0.0, 0.0);
    auto *ng1 = model.addNeuronPopulation<NeuronModels::Izhikevich>("Neuron1", 10, paramVals, varVals);
    auto *ng2 = model.addNeuronPopulation<NeuronModels::Izhikevich>("Neuron2", 10, paramVals, varVals);
    auto *ng3 = model.addNeuronPopulation<NeuronModels::Izhikevich>("Neuron3", 25, paramVals, varVals);

    Sum::VarValues sumVarValues(0.0);
    Sum::VarReferences sumVarReferences1(createVarRef(ng1, "V"), createVarRef(ng1, "U"));
    Sum::VarReferences sumVarReferences2(createVarRef(ng1, "V"), createVarRef(ng2, "V"));
    Sum::VarReferences sumVarReferences3(createVarRef(ng1, "V"), createVarRef(ng3, "V"));

    model.addCustomUpdate<Sum>("Sum1", "CustomUpdate",
                               {}, sumVarValues, sumVarReferences1);
    model.addCustomUpdate<Sum>("Sum2", "CustomUpdate",
                               {}, sumVarValues, sumVarReferences2);

    try {
        model.addCustomUpdate<Sum>("Sum3", "CustomUpdate",
                                   {}, sumVarValues, sumVarReferences3);
        FAIL();
    }
    catch(const std::runtime_error &) {
    }

    model.finalize();
}
//--------------------------------------------------------------------------
TEST(CustomUpdates, VarDelayChecks)
{
    ModelSpecInternal model;

    // Add two neuron group to model
    NeuronModels::Izhikevich::ParamValues paramVals(0.02, 0.2, -65.0, 8.0);
    NeuronModels::Izhikevich::VarValues varVals(0.0, 0.0);
    auto *pre1 = model.addNeuronPopulation<NeuronModels::Izhikevich>("Pre1", 10, paramVals, varVals);
    auto *post = model.addNeuronPopulation<NeuronModels::Izhikevich>("Post", 10, paramVals, varVals);

    // Add synapse groups to force pre1's v to be delayed by 10 timesteps and pre2's v to be delayed by 5 timesteps
    model.addSynapsePopulation<Cont, PostsynapticModels::DeltaCurr>("Syn1", SynapseMatrixType::DENSE_INDIVIDUALG,
                                                                    10, "Pre1", "Post",
                                                                    {}, {0.1}, {}, {});

    Sum::VarValues sumVarValues(0.0);
    Sum::VarReferences sumVarReferences1(createVarRef(pre1, "V"), createVarRef(post, "V"));

    model.addCustomUpdate<Sum>("Sum1", "CustomUpdate",
                               {}, sumVarValues, sumVarReferences1);

    model.finalize();
}
//--------------------------------------------------------------------------
TEST(CustomUpdates, VarMixedDelayChecks)
{
    ModelSpecInternal model;

    // Add two neuron group to model
    NeuronModels::Izhikevich::ParamValues paramVals(0.02, 0.2, -65.0, 8.0);
    NeuronModels::Izhikevich::VarValues varVals(0.0, 0.0);
    auto *pre1 = model.addNeuronPopulation<NeuronModels::Izhikevich>("Pre1", 10, paramVals, varVals);
    auto *pre2 = model.addNeuronPopulation<NeuronModels::Izhikevich>("Pre2", 10, paramVals, varVals);
    model.addNeuronPopulation<NeuronModels::Izhikevich>("Post", 10, paramVals, varVals);

    // Add synapse groups to force pre1's v to be delayed by 10 timesteps and pre2's v to be delayed by 5 timesteps
    model.addSynapsePopulation<Cont, PostsynapticModels::DeltaCurr>("Syn1", SynapseMatrixType::DENSE_INDIVIDUALG,
                                                                    10, "Pre1", "Post",
                                                                    {}, {0.1}, {}, {});
    model.addSynapsePopulation<Cont, PostsynapticModels::DeltaCurr>("Syn2", SynapseMatrixType::DENSE_INDIVIDUALG,
                                                                    5, "Pre2", "Post",
                                                                    {}, {0.1}, {}, {});

    Sum::VarValues sumVarValues(0.0);
    Sum::VarReferences sumVarReferences2(createVarRef(pre1, "V"), createVarRef(pre2, "V"));

    model.addCustomUpdate<Sum>("Sum1", "CustomUpdate",
                               {}, sumVarValues, sumVarReferences2);

    try {
        model.finalize();
        FAIL();
    }
    catch(const std::runtime_error &) {
    }
}
//--------------------------------------------------------------------------
TEST(CustomUpdates, WUVarSynapseGroupChecks)
{
    ModelSpecInternal model;

    // Add two neuron group to model
    NeuronModels::Izhikevich::ParamValues paramVals(0.02, 0.2, -65.0, 8.0);
    NeuronModels::Izhikevich::VarValues varVals(0.0, 0.0);
    model.addNeuronPopulation<NeuronModels::Izhikevich>("Pre", 10, paramVals, varVals);
    model.addNeuronPopulation<NeuronModels::Izhikevich>("Post", 25, paramVals, varVals);

    auto *sg1 = model.addSynapsePopulation<WeightUpdateModels::StaticPulseDendriticDelay, PostsynapticModels::DeltaCurr>(
        "Synapses1", SynapseMatrixType::DENSE_INDIVIDUALG, NO_DELAY,
        "Pre", "Post",
        {}, {1.0, 4},
        {}, {});
    auto *sg2 = model.addSynapsePopulation<WeightUpdateModels::StaticPulseDendriticDelay, PostsynapticModels::DeltaCurr>(
        "Synapses2", SynapseMatrixType::DENSE_INDIVIDUALG, NO_DELAY,
        "Pre", "Post",
        {}, {1.0, 4},
        {}, {});


    Sum::VarValues sumVarValues(0.0);
    Sum::WUVarReferences sumVarReferences1(createWUVarRef(sg1, "g"), createWUVarRef(sg1, "g"));
    Sum::WUVarReferences sumVarReferences2(createWUVarRef(sg1, "g"), createWUVarRef(sg2, "d"));

    model.addCustomUpdate<Sum>("SumWeight1", "CustomUpdate",
                               {}, sumVarValues, sumVarReferences1);

    try {
        model.addCustomUpdate<Sum>("SumWeight2", "CustomUpdate",
                                   {}, sumVarValues, sumVarReferences2);
        FAIL();
    }
    catch(const std::runtime_error &) {
    }

    model.finalize();
}
//--------------------------------------------------------------------------
TEST(CustomUpdates, BatchingVars)
{
    ModelSpecInternal model;
    model.setBatchSize(5);

    // Add neuron and spike source (arbitrary choice of model with read_only variables) to model
    NeuronModels::IzhikevichVariable::VarValues izkVarVals(0.0, 0.0, 0.02, 0.2, -65.0, 8.);
    auto *pop = model.addNeuronPopulation<NeuronModels::IzhikevichVariable>("Pop", 10, {}, izkVarVals);
    

    // Create update where variable is duplicated but references aren't
    Sum::VarValues sumVarValues(0.0);
    Sum::VarReferences sumVarReferences(createVarRef(pop, "a"), createVarRef(pop, "b"));

    // Create updates where variable is shared and references vary
    Sum2::VarValues sum2VarValues(1.0);
    Sum2::VarReferences sum2VarReferences1(createVarRef(pop, "V"), createVarRef(pop, "U"));
    Sum2::VarReferences sum2VarReferences2(createVarRef(pop, "a"), createVarRef(pop, "b"));
    Sum2::VarReferences sum2VarReferences3(createVarRef(pop, "V"), createVarRef(pop, "a"));

    auto *sum0 = model.addCustomUpdate<Sum>("Sum0", "CustomUpdate",
                                            {}, sumVarValues, sumVarReferences);

    auto *sum1 = model.addCustomUpdate<Sum2>("Sum1", "CustomUpdate",
                                             {}, sum2VarValues, sum2VarReferences1);
    auto *sum2 = model.addCustomUpdate<Sum2>("Sum2", "CustomUpdate",
                                             {}, sum2VarValues, sum2VarReferences2);
    auto *sum3 = model.addCustomUpdate<Sum2>("Sum3", "CustomUpdate",
                                             {}, sum2VarValues, sum2VarReferences3);
    model.finalize();

    EXPECT_TRUE(static_cast<CustomUpdateInternal*>(sum0)->isBatched());
    EXPECT_TRUE(static_cast<CustomUpdateInternal*>(sum1)->isBatched());
    EXPECT_FALSE(static_cast<CustomUpdateInternal*>(sum2)->isBatched());
    EXPECT_TRUE(static_cast<CustomUpdateInternal*>(sum3)->isBatched());
}