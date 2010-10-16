#include "planetmap.h"

#include <math.h>
#include <algorithm>

using namespace std;

PlanetMap::PlanetMap() {
    _planets.reserve (TYPICAL_MAPSIZE);
    _adjacency.reserve(TYPICAL_MAPSIZE);
    _distances.reserve(TYPICAL_MAPSIZE);
}

void PlanetMap::add_planet(plid_t id, double x, double y, ships_t growth) {
    assert(_planets.size() == id);
    _planets.push_back(pinfo_t(x, y, growth));

    update_distances(id);
}

void PlanetMap::update_distances(plid_t id) {
    for (plid_t i=0; i<_distances.size(); i++)
        _distances[i].push_back(distance(i, id));

    assert(id == _distances.size());
    _distances.push_back(vector<turn_t>());
    _distances[id].reserve(TYPICAL_MAPSIZE);

    for (plid_t i=0; i<_distances.size(); i++)
        _distances[id].push_back(distance(i, id));
}

turn_t PlanetMap::distance(plid_t p1, plid_t p2) const {
    turn_t ret;
    if (p1 < _distances.size() && p2 < _distances[p1].size())
        ret = _distances[p1][p2];
    else {
        double dx = x(p1) - x(p2);
        double dy = y(p1) - y(p2);
        ret = (turn_t)ceil(sqrt(dx * dx + dy * dy));
    }
    return ret;
}

bool cmp_fn(pair<plid_t, turn_t> p1, pair<plid_t, turn_t> p2) {
    return p1.second < p2.second;
}

void PlanetMap::init_neighbours() {
    for (plid_t i=0; i<_planets.size(); i++) {
        _adjacency.push_back(vector<pair<plid_t, turn_t> >());
        _adjacency[i].reserve(TYPICAL_MAPSIZE);

        for (plid_t j=0; j<_planets.size(); j++) {
            if (i == j)
                continue;
            pair<plid_t, turn_t> val(j, distance(i, j));
            _adjacency[i].push_back(val);
        }
        sort(_adjacency[i].begin(), _adjacency[i].end(), cmp_fn);
    }
}

ships_t PlanetMap::growth(plid_t id) const {
    assert(id < _planets.size());
    return _planets[id]._growth;
}

double PlanetMap::x(plid_t id) const {
    assert(id < _planets.size());
    return _planets[id]._x;
}

double PlanetMap::y(plid_t id) const {
    assert(id < _planets.size());
    return _planets[id]._y;
}

std::ostream & operator<<(std::ostream &out, const PlanetMap &map) {
    out << "Distance matrix:" << std::endl;
    for (plid_t id=0; id<map.num_planets(); id++) {
        cout << "# ";
        for (plid_t id2=0; id2<map.num_planets(); id2++)
            cout << map.distance(id, id2) << "\t";
        cout << std::endl;
    }
    return out;
}
