#ifndef PLANETMAP_H
#define PLANETMAP_H

#include "utils.h"

#include <vector>
#include <iostream>

using namespace std;

typedef struct _pinfo {
    double      _x;
    double      _y;
    ships_t     _growth;

    _pinfo(double x, double y, ships_t growth) :
        _x(x), _y(y), _growth(growth) {}
} pinfo_t;

class PlanetMap
{
friend class BaseBot;

private:
    vector<pinfo_t>                          _planets;
    vector<vector<pair<plid_t, turn_t> > >   _adjacency;
    vector<vector<turn_t> >                  _distances;

    void update_distances(plid_t id);
    void add_planet(plid_t id, double x, double y, ships_t growth);
    void init_neighbours();

public:
    PlanetMap();

    ships_t  growth(plid_t id) const;
    double   x(plid_t id) const;
    double   y(plid_t id) const;

    plid_t num_planets() const {return _planets.size();}

    turn_t distance(plid_t p1, plid_t p2) const;

    // debugging
    vector<pair<plid_t, turn_t> > const & neighbours(plid_t id) const {
        return _adjacency[id];
    }

};

std::ostream & operator<<(std::ostream &out, const PlanetMap &map);

#endif // PLANETMAP_H
