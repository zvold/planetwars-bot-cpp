#include "simulator.h"

#include <algorithm>
#include <iostream>

#include "config.h"

using namespace std;

bool operator==(const plstate_t &p1, const plstate_t &p2) {
    return p1._owner == p2._owner &&
           p1._ships == p2._ships;
}

bool operator==(const target_t &t1, const target_t &t2) {
    return t1._id == t2._id &&
           t1._owner == t2._owner &&
           t1._ships == t2._ships &&
           t1._turn == t2._turn &&
           t1._kind == t2._kind;
}

Simulator::Simulator(Game const *game, PlanetMap const *map, turn_t turns) :
    _game(game), _map(map), _turns(turns) {
    _arriving.reserve(3);
    simulate(_turns, true);
}

bool cmp_fn(pair<Race, ships_t> p1, pair<Race, ships_t> p2) {
    return p1.second > p2.second;
}

void Simulator::resize_data(turn_t turns) {
    _turns = turns;
    assert(ally == 1 && enemy == 2);
    for (plid_t id=0; id<_game->num_planets(); id++) {
        _states[id].reserve(_turns);
        _states[id].clear();

        _avail [ally] [id].reserve(_turns);
        _avail [ally] [id].clear();

        _avail [enemy][id].reserve(_turns);
        _avail [enemy][id].clear();

        _avail_safe [ally] [id].reserve(_turns);
        _avail_safe [ally] [id].clear();

        _avail_safe [enemy][id].reserve(_turns);
        _avail_safe [enemy][id].clear();
    }
    _end_profile._ships_diff = _end_profile._growth_diff = 0;
}

void Simulator::simulate(turn_t turns, bool force_turns) {
    Move m;
    simulate(m, turns, force_turns);
}

void Simulator::simulate_safe(Move &move, turn_t turns) {
    uint16_t num = 1;
    while (simulate(move, turns, false, true) && num < 50) {num++;}
    assert(num != 50);
    if (num > 2 && VERBOSE)
        cerr << "#\t simulated in " << num << " loops" << endl;
}

bool Simulator::simulate(Move &move, turn_t turns, bool force_turns, bool sanitize) {
    if (!force_turns) {
        turns = min((int)turns, latest_arrival(move)+2);
        turns = max(MAX_FUTURE, turns);
    }
    if (turns == 0) return false;
    resize_data(turns);

    // store planet's state at zero turn
    for (plid_t id=0; id<_game->num_planets(); id++) {
        plstate_t pstate(_game->owner(id), _game->ships(id));
        pstate = orders_depart(id, move, 0, pstate, sanitize);
        _states[id].push_back(pstate);
    }

    for (turn_t turn=1; turn<_turns; turn++) {
        for (plid_t id=0; id<_game->num_planets(); id++) {
            assert(_states[id].size() == turn);
            plstate_t pstate = _states[id][turn - 1];

            // growth planet's population
            if (pstate._owner != neutral)
                pstate._ships += _map->growth(id);

            // calculate number of ships arriving this turn
            ships_t ships;
            _arriving.clear();
            ships = pstate._owner == ally ? pstate._ships : 0;
            ships += _game->ships_arriving(id, turn, ally);
            ships += orders_arrive(id, move, turn, ally);
            if (ships != 0)
                _arriving.push_back(make_pair(ally,  ships));

            ships = pstate._owner == enemy ? pstate._ships : 0;
            ships += _game->ships_arriving(id, turn, enemy);
            ships += orders_arrive(id, move, turn, enemy);
            if (ships != 0)
                _arriving.push_back(make_pair(enemy, ships));

            if (pstate._owner == neutral)
                _arriving.push_back(make_pair(neutral, pstate._ships));

            if (_arriving.size() > 1) {
                sort(_arriving.begin(), _arriving.end(), cmp_fn);
                pair<Race, ships_t> max  = _arriving[0];
                pair<Race, ships_t> next = _arriving[1];
                assert(max.second >= next.second);
                if (max.second == next.second) {
                    // owner isn't changed as there are no survivors
                    pstate._ships = 0;
                } else {
                    pstate._ships = max.second - next.second;
                    pstate._owner = max.first;
                }
            } else if (!_arriving.empty()) {
                // single fleet - planet must be the same owner
                // removed: not true for growth 0 planets
                // assert(_arriving[0].first == pstate._owner);
                if (pstate._owner != neutral) {
                    pstate._ships = _arriving[0].second;
                    pstate._owner = _arriving[0].first;
                }
            }

            pstate = orders_depart(id, move, turn, pstate, sanitize);
            // store simulated planet's state for the turn
            _states[id].push_back(pstate);
        }
    }

    for (plid_t id=0; id<_game->num_planets(); id++) {
        assert(_states[id].size() == _turns);
        plstate_t &pstate = _states[id][_turns - 1];
        _end_profile.add_ships(pstate._ships, pstate._owner);
        _end_profile.add_growth(_map->growth(id), pstate._owner);
    }
    _end_profile._ships_diff += (int32_t)end_in_flight(move, ally) - (int32_t)end_in_flight(move, enemy);

    init_available_ships(ally);
    init_available_ships(enemy);
    assert(_avail[neutral].empty());

    init_safe_available_ships(ally);
    init_safe_available_ships(enemy);
    assert(_avail[neutral].empty());

    if (sanitize)
        return move.sanitize();
    return false;
}

turn_t Simulator::latest_arrival(const Move &move) const {
    turn_t ret = 0;
    for (map<turn_t, vector<Fleet> >::const_iterator f=_game->fleets()->begin();
                                                     f!=_game->fleets()->end(); f++)
        if (f->first > ret) ret = f->first;
    for (vector<FutureOrder>::const_iterator o=move.orders().begin();
                                             o<move.orders().end(); o++) {
        assert(o->turn() != (turn_t)-1);
        turn_t arrival = o->turn() + _map->distance(o->from(), o->to());
        if (arrival > ret) ret = arrival;
    }
    return ret;
}

// should go through all FutureOrders in all Moves, and subtract departing ships from planet's state
plstate_t & Simulator::orders_depart(plid_t id, Move &move, turn_t turn, plstate_t &pstate,
                                     bool sanitize) {
    for (vector<FutureOrder>::iterator order=move.mod_orders().begin();
                                       order<move.mod_orders().end();
                                       order++)
        if (order->from() == id && order->turn() == turn) {
            if (order->ships() <= pstate._ships &&
                order->owner() == pstate._owner) {
                    pstate._ships -= order->ships();  // valid order
            } else {
                if (sanitize)
                    order->_turn = (turn_t)-1;        // mark for removal
                else {
                    assert(order->ships() <= pstate._ships);
                    assert(order->owner() == pstate._owner);
                }
            }
        }
    return pstate;
}

// return all ships for a given race, which are in flight at turn (_turns-1)
ships_t Simulator::end_in_flight(const Move &move, Race owner) const {
    ships_t ret = 0;
    for (vector<FutureOrder>::const_iterator order=move.orders().begin();
                                             order<move.orders().end();
                                             order++)
        if (order->owner() == owner        // consider flights for a given owner
            && (order->turn() <= _turns-1) // if flight has departed
            && (order->turn() + _map->distance(order->from(), order->to()) > _turns-1      // and not arrived
            && _states.find(order->from())->second[order->turn()]._owner == order->owner())
           )
            ret += order->ships();
    return ret;
}

ships_t Simulator::orders_arrive(plid_t id, const Move &move, turn_t turn, Race owner) {
    ships_t ret = 0;
    for (vector<FutureOrder>::const_iterator order=move.orders().begin();
                                             order<move.orders().end();
                                             order++)
        if (order->turn() < turn
            && order->to() == id
            && order->owner() == owner
            && (turn == order->turn() + _map->distance(order->from(), id))
            && _states.find(order->from())->second[order->turn()]._owner == order->owner()
           )
            ret += order->ships();
    return ret;
}

bool Simulator::winning(Race race) const {
    return race == ally ? (_score > 0) : (_score <= 0);
}

ships_t Simulator::calc_safe_ships(plid_t id, Race owner, turn_t turn) {
    ships_t ret = _avail[owner][id][turn];
    vector<pair<plid_t, turn_t> > neighbours = _map->neighbours(id);
    turn_t min_eta = -1;
    Race enemy_race = opposite(owner);
    for (vector<pair<plid_t, turn_t> >::const_iterator i=neighbours.begin(); i<neighbours.end(); i++) {
        if (_game->owner(i->first) != enemy_race) continue;
        turn_t eta = i->second;
        if (min_eta == (turn_t)-1)
            min_eta = eta;
        else
            assert(eta >= min_eta);
        if (eta == min_eta) {
            ships_t ships = ships_avail(i->first, turn + 1, enemy_race);
            if (ships >= ret) return 0;
            ret -= ships;
        }
    }
    return ret;
}

void Simulator::init_available_ships(Race owner) {
    for (plid_t id=0; id<_states.size(); id++) {
        assert(_states.find(id) != _states.end());
        vector<plstate_t>::const_iterator i;
        for (i=_states[id].begin(); i<_states[id].end(); i++)
            _avail[owner][id].push_back(i->_owner == owner ? i->_ships : 0);

        for (turn_t t=_avail[owner][id].size() - 1; t>0; t--)
            _avail[owner][id][t-1] = min(_avail[owner][id][t], _avail[owner][id][t-1]);
    }
}

void Simulator::init_safe_available_ships(Race owner) {
    for (plid_t id=0; id<_states.size(); id++)
        for (uint16_t turn=0; turn<(turn_t)_avail[owner][id].size() - 1; turn++)
            _avail_safe[owner] [id].push_back(calc_safe_ships(id, owner, turn));
}

ships_t Simulator::ships_avail_safe(plid_t id, turn_t turn, Race owner) const {
    assert(owner != neutral);
    assert(_avail_safe[owner].find(id) != _avail_safe[owner].end());
    assert(true == (turn < _avail_safe[owner].find(id)->second.size()));
    return _avail_safe[owner].find(id)->second[turn];
}

ships_t Simulator::ships_avail(plid_t id, turn_t turn, Race owner) const {
    assert(owner != neutral);
    assert(_avail[owner].find(id) != _avail[owner].end());
    assert(true == (turn < _avail[owner].find(id)->second.size()));
    return _avail[owner].find(id)->second[turn];
}

vector<plstate_t> const * Simulator::planet_states(plid_t id) const {
    map<plid_t, vector<plstate_t> >::const_iterator i;
    i = _states.find(id);
    assert(i != _states.end());
    return &i->second;
}

vector<target_t> Simulator::select_targets(Race from) const {
    vector<target_t> ret;
    ret.reserve(_states.size());
    for (plid_t id=0; id<_states.size(); id++) {
        assert(_states.find(id) != _states.end());
        vector<plstate_t> pl_states = _states.find(id)->second;
        assert(pl_states.size() == _turns);
        turn_t turn;
        for (turn=0; turn<_turns-2; turn++) {
            if (pl_states[_turns-1]._owner == neutral) {
                // neutral expand, at turn 1 or later
                ret.push_back(target_t(id, neutral, pl_states[1]._ships + 1, 1, expand));
                break;
            } else if (pl_states[turn]._owner == from && pl_states[turn+1]._owner == opposite(from)) {
                // sneak defence
                ret.push_back(target_t(id, from, pl_states[turn+1]._ships, turn+1, defence));
                break;
            } else if (pl_states[turn]._owner == neutral &&
                       pl_states[turn+1]._owner == opposite(from) &&
                       pl_states[turn+2]._owner == opposite(from)) {
                // sneak attack
                turn_t attack_turn = turn+2;
                ret.push_back(target_t(id, pl_states[attack_turn]._owner,
                                       pl_states[attack_turn]._ships + 1, attack_turn, attack));
                break;
            }
        }
        if (turn == _turns-2 && pl_states[_turns-1]._owner == opposite(from)) {
            // earliest possible attack on opposite planet
            //assert(pl_states[0]._owner == opposite(from));
            ret.push_back(target_t(id, opposite(from), pl_states[1]._ships + 1, 1, expand));
        }
    }
    return ret;
}

int32_t Simulator::score(turn_t turn) {
    int32_t turns_rem = TURN_LIMIT - (int32_t)(turn + _turns);
    return _end_profile._ships_diff +
           _end_profile._growth_diff * (turns_rem>0 ? turns_rem : 0);
}

void Simulator::dump_race(plid_t id, Race owner) const {
    map<plid_t, vector<ships_t> >::const_iterator j = _avail[owner].find(id);
    vector<ships_t>::const_iterator i;
    assert(j != _avail[owner].end());
    cerr << "# " << name(owner) << " " << id << ":\t";
    for (i=j->second.begin(); i<j->second.end(); i++)
        cerr << *i << "\t";
    cerr << endl;

    j = _avail_safe[owner].find(id);
    cerr << "#s" << name(owner) << " " << id << ":\t";
    for (i=j->second.begin(); i<j->second.end(); i++)
        cerr << *i << "\t";
    cerr << endl;

}

void Simulator::dump() const {
    for (plid_t id=0; id<_avail[ally].size(); id++) {
        dump_race(id, ally);
        dump_race(id, enemy);
    }
}

std::ostream & operator<<(std::ostream &out, const target_t &t) {
    out << "id:" << t._id
        << ", owner:" << name(t._owner)
        << ", ships:" << t._ships
        << ", turn:" << t._turn
        << ", kind:" << t._kind << endl;
    return out;
}

std::ostream & operator<<(std::ostream &out, const vector<target_t> &targets) {
    for (vector<target_t>::const_iterator t=targets.begin(); t<targets.end(); t++)
        out << *t;
    return out;
}

std::ostream & operator<<(std::ostream &out, const profile &p) {
    out << (p._ships_diff>0 ? "+" : "") << p._ships_diff << "/"
        << (p._growth_diff>0 ? "+" : "") << p._growth_diff;
    return out;
}
