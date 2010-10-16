#include "basebot.h"

#include "config.h"
#include "fleet.h"

#include <iostream>
#include <map>
#include <vector>

using std::map;
using std::vector;
using std::cout;
using std::cerr;

BaseBot::BaseBot() : _turn(0) {
    reset_parser();
}

void BaseBot::run() {
    string line;
    while (getline(cin, line)) {
        if (!_parsing) {
            _timer.start();
            reset_parser();
        }
        if (line.length() >= 2 && line.at(0) == 'g' && line.at(1) == 'o') {
            finish_parsing();
            if (_game.num_planets() != 0)
              do_turn();
            end_turn();
        } else
            parse_line(line);
    }
}

void BaseBot::parse_line(string &line) {
    if (line.empty())
        return;
    else if (line.at(0) == 'P')
        parse_planet(line);
    else if (line.at(0) == 'F')
        parse_fleet(line);
}

void BaseBot::parse_planet(string &line) {
    char    tag;
    double  x, y;
    int     owner_id;
    ships_t ships, growth;

    istringstream stream(line);
    stream >> tag;
    assert(tag == 'P');

    stream >> x >> y >> owner_id >> ships >> growth;
    Race owner = static_cast<Race>(owner_id);

    if (_turn != 0) {
        _game.set_owner(_planet_id, owner);
        _game.set_ships(_planet_id, ships);
        assert(_map.x(_planet_id) == x);
        assert(_map.y(_planet_id) == y);
        assert(_map.growth(_planet_id) == growth);
    } else {
        _game.add_planet(_planet_id, owner, ships);
        _map.add_planet(_planet_id, x, y, growth);
    }

    _planet_id++;
}

void BaseBot::parse_fleet(string &line) {
    char    tag;
    int     owner_id;
    ships_t ships;
    plid_t  from, to;
    turn_t  eta, distance;

    istringstream stream(line);
    stream >> tag;
    assert(tag == 'F');

    stream >> owner_id >> ships >> from >> to >> distance >> eta;
    Race owner = static_cast<Race>(owner_id);

    _game.add_fleet(owner, ships, to, eta);
}

void BaseBot::reset_parser() {
    _parsing = true;
    _planet_id = 0;
    _game.clear_fleets();
}

void BaseBot::finish_parsing() {
    _parsing = false;
    // commented as it's always 0 ms
    //log("parsing took " + str(_timer.check()) + " ms");

    // zero turn is special - sort planets by neighborhood
    if (_turn == 0)
        _map.init_neighbours();

    init_profile();
}

void BaseBot::init_profile() {
    _profile._ships_diff = _profile._growth_diff = 0;
    for (plid_t id=0; id<_game.num_planets(); id++) {
        _profile.add_ships(_game.ships(id), _game.owner(id));
        _profile.add_growth(_map.growth(id), _game.owner(id));
    }
    std::map<turn_t, vector<Fleet> >::const_iterator j;
    vector<Fleet>::const_iterator i;
    for (j=_game.fleets()->begin(); j!=_game.fleets()->end(); j++)
        for (i=j->second.begin(); i<j->second.end(); i++)
            _profile.add_ships(i->ships(), i->owner());
}

void BaseBot::log(string const &msg) const {
    if (!VERBOSE)
        return;
    cerr << "# " << msg << endl;
    cerr.flush();
}

void BaseBot::issue_order(plid_t from, plid_t to, ships_t ships) const {
    assert(ships > 0);
    assert(_game.ships(from) >= ships);
    assert(_game.owner(from) == ally);

    cout << from << " " << to << " " << ships << endl;
    cout.flush();
}

void BaseBot::end_turn() {
    cout << "go" << endl;
    cout.flush();
    log("turn " + str(_turn) + ": \t" + str(_timer.total()) + " ms");
    inc_turn();
}

void BaseBot::dump_planet(plid_t id) const {
    log("Planet " + str(id)
        + " " + str(_game.ships(id)) + ":" + name(_game.owner(id))
        + " (+" + str(_map.growth(id)) + ")");
}

Game const * BaseBot::game() const {
    return &_game;
}

PlanetMap const * BaseBot::map() const {
    return &_map;
}
