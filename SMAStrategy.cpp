#ifdef _WIN32
#include "stdafx.h"
#endif

#include "sma_crossover_strategy.h"
#include <numeric>

SMACrossoverStrategy::SMACrossoverStrategy(StrategyID strategyID,
                                           const std::string& strategyName,
                                           const std::string& groupName)
    : Strategy(strategyID, strategyName, groupName)
    , shortPeriod(20)
    , longPeriod(50)
    , position(0.0)
{
}

SMACrossoverStrategy::~SMACrossoverStrategy() {}

void SMACrossoverStrategy::DefineStrategyParams() {}

void SMACrossoverStrategy::RegisterForStrategyEvents(StrategyEventRegister* eventRegister, DateType currDate) {
    eventRegister->RegisterForBarEvent(this);
    eventRegister->RegisterForDepthEvent(this);
}

void SMACrossoverStrategy::OnDepth(const MarketDepthEventMsg& msg) {
    const Instrument& instrument = msg.instrument();
    prices.push_back(instrument.top_quote().midpoint());

    if (prices.size() >= std::max(shortPeriod, longPeriod)) {
        shortSMA.clear();
        longSMA.clear();

        for (int i = static_cast<int>(prices.size()) - 1; i >= 0; --i) {
            shortSMA.push_back(std::accumulate(prices.begin() + std::max(0, i - shortPeriod + 1), prices.begin() + i + 1, 0.0) / shortPeriod);
            longSMA.push_back(std::accumulate(prices.begin() + std::max(0, i - longPeriod + 1), prices.begin() + i + 1, 0.0) / longPeriod);
        }

        std::reverse(shortSMA.begin(), shortSMA.end());
        std::reverse(longSMA.begin(), longSMA.end());

        AdjustPortfolio();
    }
}

void SMACrossoverStrategy::OnBar(const BarEventMsg& msg) {}

void SMACrossoverStrategy::AdjustPortfolio() {
    int currentIndex = static_cast<int>(prices.size()) - 1;

    if (currentIndex >= static_cast<int>(std::max(shortPeriod, longPeriod)) - 1) {
        double currentShortSMA = shortSMA[currentIndex];
        double currentLongSMA = longSMA[currentIndex];
        double prevShortSMA = shortSMA[currentIndex - 1];
        double prevLongSMA = longSMA[currentIndex - 1];

        if (currentShortSMA > currentLongSMA && prevShortSMA <= prevLongSMA) {
            SendOrder(&msg.instrument(), 1000); // Buy signal
        } else if (currentShortSMA < currentLongSMA && prevShortSMA >= prevLongSMA) {
            SendOrder(&msg.instrument(), -1000); // Sell signal
        }
    }
}

void SMACrossoverStrategy::SendOrder(const Instrument* instrument, int trade_size) {
    double price = instrument->top_quote().midpoint();
    OrderSide side = (trade_size > 0) ? ORDER_SIDE_BUY : ORDER_SIDE_SELL;

    OrderParams params(
        *instrument,
        abs(trade_size),
        price,
        MARKET_CENTER_ID_IEX,
        side,
        ORDER_TIF_DAY,
        ORDER_TYPE_LIMIT);

    TradeActionResult tra = trade_actions()->SendNewOrder(params);
    if (tra == TRADE_ACTION_RESULT_SUCCESSFUL) {
        std::cout << "Sending new trade order successful!" << std::endl;
        position += trade_size;
    } else {
        std::cout << "Error sending new trade order..." << tra << std::endl;
    }
}

extern "C" {
    _STRATEGY_EXPORTS const char* GetType() {
        return "SMACrossoverStrategy";
    }

    _STRATEGY_EXPORTS IStrategy* CreateStrategy(const char* strategyType,
                                                 unsigned strategyID,
                                                 const char* strategyName,
                                                 const char* groupName) {
        if (strcmp(strategyType, GetType()) == 0) {
            return *(new SMACrossoverStrategy(strategyID, strategyName, groupName));
        } else {
            return NULL;
        }
    }

    _STRATEGY_EXPORTS const char* GetAuthor() {
        return "Your Name";
    }

    _STRATEGY_EXPORTS const char* GetAuthorGroup() {
        return "Your Group";
    }

    _STRATEGY_EXPORTS const char* GetReleaseVersion() {
        return Strategy::release_version();
    }
}
