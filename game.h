#ifndef GAME_H
#define GAME_H

#include "utils.h"

#include <vector>
#include <map>

#include "race.h"
#include "futureorder.h"
#include "fleet.h"

using std::string;
using std::vector;
using std::map;

class Game {
friend class BaseBot;

private:
    pmask_t                     _planets[race_last];
    vector<ships_t>             _ships;
    map<turn_t, vector<Fleet> > _fleets;

protected:
    void        add_planet(plid_t id, Race owner, ships_t ships);
    void        add_fleet(Race owner, ships_t ships, plid_t to, turn_t eta);

    void        set_owner(plid_t id, Race owner);
    void        set_ships(plid_t id, ships_t ships);

    void        clear_fleets();

public:
    Game();

    Race        owner(plid_t id) const;
    ships_t     ships(plid_t id) const;

    void        add_ships(plid_t id, ships_t ships);

    plid_t      num_planets() const;
    ships_t     ships_arriving(plid_t id, turn_t turn, Race owner) const;

    // debugging
    map<turn_t, vector<Fleet> > const * fleets() const;

private:
    bool        verify_planets() const;
};

#endif // GAME_H
