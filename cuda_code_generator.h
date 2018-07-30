#pragma once

// Standard C++ includes
#include <functional>
#include <string>

// NuGeNN includes
#include "code_generator.h"

//--------------------------------------------------------------------------
// CUDA::CodeGenerator
//--------------------------------------------------------------------------
namespace CUDA
{
class CodeGenerator : public ::CodeGenerator::Base
{
public:
    CodeGenerator(size_t neuronUpdateBlockSize, size_t presynapticUpdateBlockSize) 
    :   m_NeuronUpdateBlockSize(neuronUpdateBlockSize), m_PresynapticUpdateBlockSize(presynapticUpdateBlockSize)
    {
    }

    //--------------------------------------------------------------------------
    // CodeGenerator::Base virtuals
    //--------------------------------------------------------------------------
    virtual void genNeuronUpdateKernel(CodeStream &os, const NNmodel &model,
                                       std::function<void(CodeStream &, const ::CodeGenerator::Base &, const NNmodel&, const NeuronGroup &ng, const std::string &, const std::string &)> handler) const override;

    virtual void genPresynapticUpdateKernel(CodeStream &os, const NNmodel &model) const override;

    virtual void genEmitTrueSpike(CodeStream &os, const NNmodel &, const NeuronGroup &, const std::string &neuronID) const override
    {
        genEmitSpike(os, neuronID, "");
    }
    
    virtual void genEmitSpikeLikeEvent(CodeStream &os, const NNmodel &model, const NeuronGroup &ng, const std::string &neuronID) const override
    {
        genEmitSpike(os, neuronID, "Evnt");
    }

    virtual std::string getVarPrefix() const override{ return "dd_"; }

    virtual const std::vector<FunctionTemplate> &getFunctions() const override{ return cudaFunctions; }

private:
    //--------------------------------------------------------------------------
    // Private methods
    //--------------------------------------------------------------------------
    void genParallelNeuronGroup(CodeStream &os, const NNmodel &model,
                                std::function<void(CodeStream &, const ::CodeGenerator::Base &, const NNmodel &, const NeuronGroup&)> handler) const;

    void genParallelSynapseGroup(CodeStream &os, const NNmodel &model,
                                 std::function<size_t(const SynapseGroup&)> getPaddedSizeFunc,
                                 std::function<void(CodeStream &, const ::CodeGenerator::Base &, const NNmodel &, const SynapseGroup&)> handler) const;
                                 
    void genEmitSpike(CodeStream &os, const std::string &neuronID, const std::string &suffix) const;

    size_t getPresynapticUpdateKernelSize(const SynapseGroup &sg) const;
    
    bool shouldAccumulateInLinSyn(const SynapseGroup &sg) const;
    
    bool shouldAccumulateInSharedMemory(const SynapseGroup &sg) const;
    
    //--------------------------------------------------------------------------
    // Members
    //--------------------------------------------------------------------------
    const size_t m_NeuronUpdateBlockSize;
    const size_t m_PresynapticUpdateBlockSize;
};
}   // CodeGenerator
