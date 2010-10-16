#include "testutils.h"

#include <iostream>

unsigned int count_fleets(map<turn_t, vector<Fleet> > const * fleets, plid_t id, Race owner) {
    unsigned int ret = 0;
    map<turn_t, vector<Fleet> >::const_iterator j;
    vector<Fleet>::const_iterator i;
    for (j=fleets->begin(); j!=fleets->end(); j++)
        for (i=j->second.begin(); i<j->second.end(); i++)
            if (i->owner() == owner &&
                (i->to() == id || id == (plid_t)-1))
                ret++;
    return ret;
}

unsigned int count_fleets(map<turn_t, vector<Fleet> > const * fleets, Race owner) {
    return count_fleets(fleets, (plid_t)-1, owner);
}

ships_t sum_growth(Race owner, Game const *game, PlanetMap const *map) {
    ships_t ret = 0;
    for (plid_t id=0; id<game->num_planets(); id++)
        if (game->owner(id) == owner)
            ret += map->growth(id);
    return ret;
}

void parse_state(BaseBot &bot, string state[], uint8_t size) {
    bot.reset_parser();
    for (unsigned int i=0; i<size; i++)
        bot.parse_line(state[i]);
    bot.finish_parsing();
    bot.inc_turn();
}

void parse_state(BaseBot &bot, ifstream &state) {
    bot.reset_parser();
    string line;
    while (getline(state, line))
        bot.parse_line(line);
    bot.finish_parsing();
    bot.inc_turn();
}

void dump(vector<plstate_t> const * states) {
    vector<plstate_t>::const_iterator i;
    for (i=states->begin(); i<states->end(); i++)
        std::cout << name(i->_owner) << ":" << i->_ships << " ";
    std::cout << std::endl;
}

void dump(vector<target_t> const * targets) {
    vector<target_t>::const_iterator i;
    for (i=targets->begin(); i<targets->end(); i++)
        std::cout << "Planet " << i->_id << " (" << name(i->_owner)
                  << "), " + name(i->_kind) + " with " << i->_ships << " at " << i->_turn
                  << std::endl;
}

void dump(vector<Move*> const &moves) {
    for (vector<Move*>::const_iterator i=moves.begin(); i<moves.end(); i++)
        cout << *i;
}

string name(TargetKind kind) {
    switch (kind) {
        case attack:  return "attack";
        case defence: return "defence";
        case expand:  return "expand";
        default:
            assert(false);
            return "none";
    }
}
