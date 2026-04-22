/*#ifndef ENERGY_H
#define ENERGY_H

struct Metrics{
    int loc=0;
    int compCount=0;
    int arithCount=0;
    int logicCount=0;
    int ioCount=0;
    int bitCount=0;
    int memCount=0;
    int allocCount=0;
    int loopCount=0;
    int recursion=0;
    int callCount=0;
    long long iterCount=0;
    double energy=0.0;
};

struct EnergyModel{
    double compCost=0.5;
    double arithCost=0.3;
    double logicCost=0.2;
    double ioCost=1.0;
    double bitCost=0.4;
    double memCost=0.6;
    double allocCost=2.0;
    double callCost=0.8;
    double locCost=0.05;

    double compute(const Metrics& m){
        if(m.loc == 0) return 0.0;
        
        if(m.iterCount>0){
            double loopEnergy = (m.arithCount + m.logicCount + m.bitCount + m.memCount) * arithCost * m.iterCount;
            return loopEnergy + (m.compCount * compCost) + (m.ioCount * ioCost) + 
                   (m.allocCount * allocCost) + (m.loc * locCost) + (m.callCount * callCost);
        }
        
        if (m.recursion){
            double recEnergy = (m.arithCount + m.logicCount + m.bitCount + m.memCount) * arithCost * 10;
            return recEnergy + (m.compCount * compCost) + (m.ioCount * ioCost) + 
                   (m.allocCount * allocCost) + (m.loc * locCost) + (m.callCount * callCost);
        }
        
        return (m.compCount * compCost) + (m.arithCount * arithCost) + (m.logicCount * logicCost) +
               (m.ioCount * ioCost) + (m.bitCount * bitCost) + (m.memCount * memCost) +
               (m.allocCount * allocCost) + (m.loc * locCost) + (m.callCount * callCost);
    }
};

#endif*/

/*#ifndef ENERGY_H
#define ENERGY_H

#include <vector>
#include <string>

struct LoopInfo {
    std::string loopType;        // "for" or "while"
    int         line         = 0;
    long long   iterations   = 0;
    double      perIterEnergy= 0.0;
    double      totalLoopEnergy = 0.0;
};

struct RecInfo {
    bool   active         = false;
    int    estimatedDepth = 10;
    double perCallEnergy  = 0.0;
    double totalRecEnergy = 0.0;
};

struct Metrics{
    int loc=0;
    int compCount=0;
    int arithCount=0;
    int logicCount=0;
    int ioCount=0;
    int bitCount=0;
    int memCount=0;
    int allocCount=0;
    int loopCount=0;
    int recursion=0;
    int callCount=0;
    long long iterCount=0;
    double energy=0.0;
    std::vector<LoopInfo> loops;
    RecInfo recInfo;
};

struct EnergyModel{
    double compCost=0.5;
    double arithCost=0.3;
    double logicCost=0.2;
    double ioCost=1.0;
    double bitCost=0.4;
    double memCost=0.6;
    double allocCost=2.0;
    double callCost=0.8;
    double locCost=0.05;

    double compute(const Metrics& m){
        if(m.loc == 0) return 0.0;
        
        if(m.iterCount>0){
            double loopEnergy = (m.arithCount + m.logicCount + m.bitCount + m.memCount) * arithCost * m.iterCount;
            return loopEnergy + (m.compCount * compCost) + (m.ioCount * ioCost) + 
                   (m.allocCount * allocCost) + (m.loc * locCost) + (m.callCount * callCost);
        }
        
        if (m.recursion){
            double recEnergy = (m.arithCount + m.logicCount + m.bitCount + m.memCount) * arithCost * 10;
            return recEnergy + (m.compCount * compCost) + (m.ioCount * ioCost) + 
                   (m.allocCount * allocCost) + (m.loc * locCost) + (m.callCount * callCost);
        }
        
        return (m.compCount * compCost) + (m.arithCount * arithCost) + (m.logicCount * logicCost) +
               (m.ioCount * ioCost) + (m.bitCount * bitCost) + (m.memCount * memCost) +
               (m.allocCount * allocCost) + (m.loc * locCost) + (m.callCount * callCost);
    }
};

#endif*/

#ifndef ENERGY_H
#define ENERGY_H

#include <bits/stdc++.h>
using namespace std;

struct LoopInfo {
    string loopType;
    int depth;
    int line;
    long long ownIters;
    int arith=0, comp=0, logic=0, io=0, bit=0, mem=0, alloc=0, calls=0;
    double perIterCost = 0.0;
    double loopCost    = 0.0;
};

struct Metrics {
    int loc=0, compCount=0, arithCount=0, logicCount=0;
    int ioCount=0, bitCount=0, memCount=0, allocCount=0;
    int loopCount=0, recursion=0, callCount=0;
    long long iterCount=0;
    double energy=0.0;
    vector<LoopInfo> loopDetails;
};

struct EnergyModel {
    double compCost=0.5, arithCost=0.3, logicCost=0.2, ioCost=1.0;
    double bitCost=0.4,  memCost=0.6,   allocCost=2.0, callCost=0.8, locCost=0.05;

    double compute(const Metrics& m) {
        if (m.loc == 0) return 0.0;
        if (m.iterCount > 0) {
            double le = (m.arithCount+m.logicCount+m.bitCount+m.memCount)*arithCost*m.iterCount;
            return le+(m.compCount*compCost)+(m.ioCount*ioCost)
                     +(m.allocCount*allocCost)+(m.loc*locCost)+(m.callCount*callCost);
        }
        if (m.recursion) {
            double re = (m.arithCount+m.logicCount+m.bitCount+m.memCount)*arithCost*10;
            return re+(m.compCount*compCost)+(m.ioCount*ioCost)
                     +(m.allocCount*allocCost)+(m.loc*locCost)+(m.callCount*callCost);
        }
        return (m.compCount*compCost)+(m.arithCount*arithCost)+(m.logicCount*logicCost)
              +(m.ioCount*ioCost)+(m.bitCount*bitCost)+(m.memCount*memCost)
              +(m.allocCount*allocCost)+(m.loc*locCost)+(m.callCount*callCost);
    }
};

#endif