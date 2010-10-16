#ifndef FASTEXPAND_H
#define FASTEXPAND_H

#include "utils.h"

#include "basebot.h"
#include "simulator.h"

class FastExpand {
private:
    const BaseBot     *_bot;
          Simulator   *_sim;

public:
    FastExpand(const BaseBot &bot, Simulator &sim);

    void execute();

private:

    vector<plid_t> knapsack(plid_t home, ships_t ships, const vector<plid_t> &neutrals);

};

class KSData {
public:
    KSData      *_back_edge;
    uint16_t     _value;
    plid_t       _item;
    KSData();

    void setFrom(KSData &from);
};

#endif // FASTEXPAND_H
