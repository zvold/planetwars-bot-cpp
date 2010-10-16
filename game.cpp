#include "game.h"

Game::Game() {
    for (int i=0; i<race_last; i++)
        _planets[i] = 0;
    _ships.reserve(TYPICAL_MAPSIZE);
}

Race Game::owner(plid_t id) const {
    assert(verify_planets());
    pmask_t mask = 1 << id;
    for (int i=0; i<race_last; i++)
        if (_planets[i] & mask)
            return static_cast<Race>(i);
    assert(false);
    return race_last;
}

ships_t Game::ships(plid_t id) const {
    assert(id < _ships.size());
    return _ships[id];
}

void Game::set_owner(plid_t id, Race owner) {
    assert(id < sizeof(pmask_t) * 8);
    pmask_t mask = 1 << id;
    for (int i=0; i<race_last; i++)
        if (i == owner)
            _planets[i] |= mask;
        else
            _planets[i] &= ~mask;
}

void Game::set_ships(plid_t id, ships_t ships) {
    assert(id < _ships.size());
    _ships[id] = ships;
}

void Game::add_ships(plid_t id, ships_t ships) {
    assert(id < _ships.size());
    assert(_ships[id] >= ships);
    set_ships(id, _ships[id] - ships);
}

void Game::add_planet(plid_t id, Race owner, ships_t ships) {
    assert(_ships.size() == id);
    _ships.push_back(ships);
    set_owner(id, owner);
}

void Game::add_fleet(Race owner, ships_t ships, plid_t to, turn_t eta) {
    Fleet fleet(owner, ships, to, eta);
    _fleets[eta].push_back(fleet);
}

ships_t Game::ships_arriving(plid_t id, turn_t turn, Race owner) const {
    ships_t ret = 0;
    map<turn_t, vector<Fleet> >::const_iterator fleet_iter = _fleets.find(turn);
    if (fleet_iter == _fleets.end())
        return ret;
    vector<Fleet>::const_iterator i;
    for (i=fleet_iter->second.begin(); i<fleet_iter->second.end(); i++) {
        assert(turn == i->eta());
        if (i->owner() == owner && i->to() == id)
            ret += i->ships();
    }
    return ret;
}

void Game::clear_fleets() {
    _fleets.clear();
}

bool Game::verify_planets() const {
    pmask_t mask = 0;
    for (int i=0; i<race_last; i++)
        mask ^= _planets[i];
    return ((mask == (pmask_t)-1) ||
            ((pmask_t)1 << (sizeof(pmask_t) * 8 - nlz(mask)) == mask + 1));
}

plid_t Game::num_planets() const {
    return _ships.size();
}

map<turn_t, vector<Fleet> > const * Game::fleets() const {
    return &_fleets;
}
