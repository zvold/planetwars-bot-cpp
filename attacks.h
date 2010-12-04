#ifndef ATTACKS_H
#define ATTACKS_H

#include "utils.h"

#include "move.h"
#include "game.h"
#include "planetmap.h"
#include "simulator.h"
#include "race.h"

#include <vector>
#include <iostream>

using std::vector;
using std::cerr;

// takes current game state, and generates the move to attack planet exactly as specified in target
// the caller is responsible for Move* cleanup
Move * exact_attack(const Simulator &sim,
                    const target_t &target,
                    Race attacker,
                    bool safe = true);

// returns a move attacking target at target._turn or later, but as early as possible, or NULL
Move * waiting_attack(const Simulator &sim,
                      const target_t &target,
                      Race attacker,
                      bool safe = true);

Move * create_move(Simulator &sim,
                   const Move &existing,
                   const vector<target_t>::const_iterator &begin,
                   const vector<target_t>::const_iterator &end,
                   Race attacker,
                   bool safe = true);

void shuffle_targets(const Simulator &sim,
                     const Move &existing,
                     vector<target_t> &targets,
                     vector<pair<Move, vector<target_t> > > &shuf_targets,
                     Race attacker);

Move * sneak_defence(Simulator &sim,
                     const Move &existing,
                     vector<target_t> &targets,
                     Race attacker);

void reinforce(Simulator &sim,
               vector<Move *> &moves,
               Race attacker);

unsigned int total_orders(const vector<Move*> &moves);
unsigned int unique_orders(const vector<Move*> &moves);


#endif // ATTACKS_H
