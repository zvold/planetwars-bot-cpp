#ifndef MINIBOT_H
#define MINIBOT_H

#include "utils.h"

#include "basebot.h"
#include "move.h"
#include "simulator.h"

#include <vector>

using std::vector;

class MyBot : public BaseBot
{
private:
    Move            _prev_move;
    Simulator       sim;

    // issues orders for a given move, updates _prev_move with
    // a sanitized version of remaining orders
    void issue_orders(const Move &move, Move &prev_move) const;
    bool timeout();
    void fill_targets(Move &move, vector<pair<Move, vector<target_t> > > &shuf_targets, Race attacker);
    void force_defence(Move &prev_move, vector<Move*> &pre_moves, Race attacker);

public:
    MyBot();
    virtual ~MyBot();

    virtual void do_turn();
};

#endif // MINIBOT_H
