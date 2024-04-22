#pragma once

#ifndef _STRATEGY_STUDIO_LIB_EXAMPLES_SMA_CROSSOVER_STRATEGY_H_
#define _STRATEGY_STUDIO_LIB_EXAMPLES_SMA_CROSSOVER_STRATEGY_H_

#ifdef _WIN32
#define _STRATEGY_EXPORTS __declspec(dllexport)
#else
#ifndef _STRATEGY_EXPORTS
#define _STRATEGY_EXPORTS
#endif
#endif

#include <Strategy.h>
#include <MarketModels/Instrument.h>
#include <string>
#include <vector>

using namespace RCM::StrategyStudio;

class SMACrossoverStrategy : public Strategy {
public:
    SMACrossoverStrategy(StrategyID strategyID,
                         const std::string& strategyName,
                         const std::string& groupName);
    ~SMACrossoverStrategy();

    virtual void OnBar(const BarEventMsg& msg);
    virtual void OnDepth(const MarketDepthEventMsg& msg);

private:
    void RegisterForStrategyEvents(StrategyEventRegister* eventRegister, DateType currDate);
    void DefineStrategyParams();
    void AdjustPortfolio();
    void SendOrder(const Instrument* instrument, int trade_size);

private:
    int shortPeriod;
    int longPeriod;
    std::vector<double> prices;
    std::vector<double> shortSMA;
    std::vector<double> longSMA;
    double position;
};

extern "C" {
    _STRATEGY_EXPORTS const char* GetType();
    _STRATEGY_EXPORTS IStrategy* CreateStrategy(const char* strategyType,
                                                 unsigned strategyID,
                                                 const char* strategyName,
                                                 const char* groupName);
    _STRATEGY_EXPORTS const char* GetAuthor();
    _STRATEGY_EXPORTS const char* GetAuthorGroup();
    _STRATEGY_EXPORTS const char* GetReleaseVersion();
}

#endif
