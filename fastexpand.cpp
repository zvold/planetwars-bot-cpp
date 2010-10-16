#include "fastexpand.h"

#include "config.h"
#include "sortfuncs.h"

#include <math.h>

FastExpand::FastExpand(const BaseBot &bot, Simulator &sim) : _bot(&bot), _sim(&sim) {
}

void FastExpand::execute() {
    plid_t enemy_home, ally_home;
    plid_t num_planets = _bot->game()->num_planets();
    for (enemy_home=0; enemy_home<num_planets; enemy_home++)
        if (_bot->game()->owner(enemy_home) == enemy)
            break;
    for (ally_home=0; num_planets; ally_home++)
        if (_bot->game()->owner(ally_home) == ally)
            break;
    assert(ally_home < num_planets && enemy_home < num_planets);

    ships_t enemy_ships, ally_ships;

    turn_t eta = _bot->map()->distance(enemy_home, ally_home);
    _sim->simulate(eta + 2, true); // to ensure planet states are simulated at turn == eta

    enemy_ships = (*_sim->planet_states(enemy_home))[1]._ships;
    ally_ships = (*_sim->planet_states(ally_home))[eta]._ships;
    assert(enemy_ships == 105);
    assert(ally_ships >= enemy_ships);

    ally_ships = min((int)(*_sim->planet_states(ally_home))[0]._ships,
                     ally_ships - enemy_ships);
    _bot->log("ships available for fast expand: " + str(ally_ships));

    vector<plid_t> neutrals;
    for (plid_t id=0; id<num_planets; id++)
        if (_bot->game()->owner(id) == neutral &&
            FEXPAND_FACTOR * _bot->map()->distance(id, ally_home) <
                             _bot->map()->distance(id, enemy_home))
            neutrals.push_back(id);

    vector<plid_t> attack = knapsack(ally_home, ally_ships, neutrals);
    if (attack.empty()) return;

    _bot->log("expanding to " + str(attack.size()) + " planets");
    ships_t ships_sent = 0;
    for (vector<plid_t>::iterator i=attack.begin(); i<attack.end(); i++) {
        assert(_bot->game()->owner(*i) == neutral);
        ships_t num_ships = _bot->game()->ships(*i) + 1;
        _bot->issue_order(ally_home, *i, num_ships);
        ships_sent += num_ships;
    }
    assert(ships_sent <= ally_ships);
}

KSData::KSData() : _back_edge(NULL), _value(0), _item(-1) {}

void KSData::setFrom(KSData &from) {
    _back_edge = &from;
    _value = from._value;
    _item = -1;
}

vector<plid_t> FastExpand::knapsack(plid_t home, ships_t W, const vector<plid_t> &neutrals) {
    const uint16_t num = neutrals.size();
    KSData data[W+1][num+1];

    growth_closeness sorter(_bot->game(), _bot->map(), neutral);
    KSData *maxData = &data[0][0];
    for (uint16_t w=1; w<W + 1; w++) {
        for (uint16_t i=1; i<num + 1; i++) {
            plid_t planet = neutrals.at(i - 1);
            assert(_bot->game()->owner(planet) == neutral);
            int pw = _bot->game()->ships(planet) + 1;
            if (pw > w) {
                // planet weight is more than the current weight limit
                data[w][i].setFrom(data[w][i-1]);
            } else {
                uint16_t value = -(int)(1000.0 * sorter.score(planet));
                // planet weight is less or equal to the current weight limit
                if (data[w][i - 1]._value >= data[w - pw][i - 1]._value + value) {
                    data[w][i].setFrom(data[w][i - 1]);
                } else {
                    data[w][i].setFrom(data[w - pw][i - 1]);
                    data[w][i]._value += value;
                    data[w][i]._item = planet;
                    if (data[w][i]._value > maxData->_value)
                        maxData = &data[w][i];
                }
            }
        }
    }

    vector<plid_t> ret;
    // backtrack from maxData
    do {
        if (maxData->_item != (plid_t)-1) {
            ret.push_back(maxData->_item);
        }
        maxData = maxData->_back_edge;
    } while (maxData != NULL);

    return ret;
}
