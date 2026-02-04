#ifndef ENERGY_MODEL_H
#define ENERGY_MODEL_H

struct FunctionMetrics{
    int loc=0;
    int comparisonCount=0;
    int arithmeticCount=0;
    int logicalCount=0;
    int inputOutputCount=0;
    int bitwiseCount=0;
    int memoryAccessCount=0;
    int allocationCount=0;
    int loopCount=0;
    int recursionFlag=0;
    int functionCallCount=0;
    long long estimatediterationCount=0;
    double energyConsumption=0.0;
};
struct EnergyModel{
    double energyPerComparison=0.5;
    double energyPerArithmetic=0.3;
    double energyPerLogical=0.2;
    double energyPerInputOutput=1.0;
    double energyPerBitwise=0.4;
    double energyPerMemoryAccess=0.6;
    double energyPerAllocation=2.0;
    double energyPerFunctionCall=0.8;
    double energyPerlineOfCode=0.05;

double computeEnergy(const FunctionMetrics& metrics){
    if(metrics.loc == 0) return 0.0;
    if(metrics.estimatediterationCount>0){
        double LoopEnergy = (metrics.arithmeticCount + metrics.logicalCount +  metrics.bitwiseCount + metrics.memoryAccessCount) *
                                 energyPerArithmetic * metrics.estimatediterationCount;
                                
            return LoopEnergy +
                   (metrics.comparisonCount * energyPerComparison) +
                   (metrics.inputOutputCount * energyPerInputOutput) +
                   (metrics.allocationCount * energyPerAllocation) +
                   (metrics.loc * energyPerlineOfCode) +
                   (metrics.functionCallCount * energyPerFunctionCall);

        }
        if (metrics.recursionFlag){
            double RecursionEnergy = (metrics.arithmeticCount + metrics.logicalCount +   metrics.bitwiseCount + metrics.memoryAccessCount) *
                                      energyPerArithmetic * 10;//recursion depth= 10
            return RecursionEnergy +
                   (metrics.comparisonCount * energyPerComparison) +
                   (metrics.inputOutputCount * energyPerInputOutput) +
                   (metrics.allocationCount * energyPerAllocation) +
                   (metrics.loc * energyPerlineOfCode) +
                   (metrics.functionCallCount * energyPerFunctionCall);
        }
        return (metrics.comparisonCount * energyPerComparison) +
               (metrics.arithmeticCount * energyPerArithmetic) +
               (metrics.logicalCount * energyPerLogical) +
               (metrics.inputOutputCount * energyPerInputOutput) +
               (metrics.bitwiseCount * energyPerBitwise) +
               (metrics.memoryAccessCount * energyPerMemoryAccess) +
               (metrics.allocationCount * energyPerAllocation) +
               (metrics.loc * energyPerlineOfCode) +
               (metrics.functionCallCount * energyPerFunctionCall);

    };
};
#endif