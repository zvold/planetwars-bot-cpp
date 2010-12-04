#ifndef TESTUTILS_H
#define TESTUTILS_H

#include "../utils.h"
#include "../fleet.h"
#include "../race.h"
#include "../game.h"
#include "../planetmap.h"
#include "../basebot.h"
#include "../simulator.h"
#include "../move.h"
#include "../futureorder.h"

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#define LENGTH(x) (sizeof(x)/sizeof(x[0]))

using std::map;
using std::vector;
using std::string;
using std::ifstream;

unsigned int count_fleets(map<turn_t, vector<Fleet> > const * fleets, Race owner);
unsigned int count_fleets(map<turn_t, vector<Fleet> > const * fleets, plid_t id, Race owner);

ships_t sum_growth(Race owner, Game const *game, PlanetMap const *map);

void dump(vector<FutureOrder> const * orders);
void dump(vector<plstate_t> const * states);
void dump(vector<target_t> const * targets);
void dump(vector<Move*> const &moves);

void parse_state(BaseBot &bot, string state[], uint8_t size);
void parse_state(BaseBot &bot, ifstream &state);

string name(TargetKind kind);

#endif // TESTUTILS_H
