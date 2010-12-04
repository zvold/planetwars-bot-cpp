#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "utils.h"

#include <map>
#include <vector>

#include "game.h"
#include "race.h"
#include "planetmap.h"
#include "move.h"

typedef struct _plstate {
    Race    _owner;
    ships_t _ships;

    _plstate(Race owner, ships_t ships) :
        _owner(owner), _ships(ships) {}
    _plstate() {assert(false);}
} plstate_t;

bool operator==(const plstate_t &p1, const plstate_t &p2);

enum TargetKind {
    attack,
    defence,
    expand,
    target_kind_last
};

typedef struct _target {
    plid_t      _id;
    Race        _owner;
    ships_t     _ships;
    turn_t      _turn;
    TargetKind  _kind;

    _target(plid_t id, Race owner, ships_t ships, turn_t turn, TargetKind kind) :
        _id(id), _owner(owner), _ships(ships), _turn(turn), _kind(kind) {}
} target_t;

bool operator==(const target_t &t1, const target_t &t2);

std::ostream & operator<<(std::ostream &out, const target_t &t);
std::ostream & operator<<(std::ostream &out, const vector<target_t> &targets);

struct profile {
    int32_t     _ships_diff;
    int32_t     _growth_diff;

    profile() : _ships_diff(0), _growth_diff(0) {};

    void add_ships(int32_t ships, Race owner) {
        if (owner == ally) _ships_diff += ships;
        else if (owner == enemy) _ships_diff -= ships;
    }

    void add_growth(int32_t ships, Race owner) {
        if (owner == ally) _growth_diff += ships;
        else if (owner == enemy) _growth_diff -= ships;
    }
};

std::ostream & operator<<(std::ostream &out, const profile &p);

class Simulator {
private:
    vector<pair<Race, ships_t> >    _arriving;

    const Game                     *_game;
    const PlanetMap                *_map;
    turn_t                          _turns;
    map<plid_t, vector<plstate_t> > _states;
    map<plid_t, vector<ships_t> >   _avail[race_last]; // map for neutral is unused
    map<plid_t, vector<ships_t> >   _avail_safe[race_last];
    profile                         _end_profile;
    int32_t                         _score;
    pmask_t                         _evacuation[race_last];

    void    init_available_ships(Race owner);

public:
    Simulator(Game const *game, PlanetMap const *map, turn_t turns);

    void simulate(turn_t turns, bool force_turns = false);
    void simulate_safe(Move &move, turn_t turns);
    bool simulate(Move &move, turn_t turns, bool force_turns = false, bool sanitize = false);

    ships_t ships_avail(plid_t id, turn_t turn, Race owner) const;
    ships_t ships_avail_safe(plid_t id, turn_t turn, Race owner) const;

    vector<plstate_t> const * planet_states(plid_t id) const;
    vector<target_t> select_targets(Race from) const;
    int32_t score(turn_t turn);
    void    set_score(int32_t score) {_score = score;}

    const Game      * game() const {return _game;}
    const PlanetMap * pmap()  const {return _map;}
          turn_t      turns() const {return _turns;}
    const profile   & end_profile() const {return _end_profile;}
          bool        winning(Race race) const;

    void clear_evacuation_marks();
    void mark_for_evacuation(plid_t id, Race owner);
    bool marked_for_evacuation(plid_t id, Race owner) const;

private:
    ships_t calc_safe_ships(plid_t id, Race owner, turn_t turn);
    void init_safe_available_ships(Race owner);
    turn_t  latest_arrival(const Move &move) const;
    ships_t end_in_flight(const Move &move, Race owner) const;
    void resize_data(turn_t turns);

    plstate_t & orders_depart(plid_t id, Move &move, turn_t turn, plstate_t &pstate,
                              bool sanitize = false);
    ships_t orders_arrive(plid_t id, const Move &move, turn_t turn, Race owner);

    // debugging
public:
    void dump() const;
private:
    void dump_race(plid_t id, Race owner) const;
};

#endif // SIMULATOR_H
