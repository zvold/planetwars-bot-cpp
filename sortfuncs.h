#ifndef SORTFUNCS_H
#define SORTFUNCS_H

#include "utils.h"

#include "game.h"
#include "planetmap.h"
#include "simulator.h"
#include "race.h"

struct target_kind_comp {
    const Game      *_game;
    const PlanetMap *_map;
    TargetKind       _kind;
    Race             _owner;
    target_kind_comp(const Game *game, const PlanetMap *map) :
        _game(game), _map(map), _kind(target_kind_last), _owner(race_last) {}
    target_kind_comp(TargetKind kind) :
        _game(NULL), _map(NULL), _kind(kind), _owner(race_last) {}
    target_kind_comp(Race owner) :
        _game(NULL), _map(NULL), _kind(target_kind_last), _owner(owner) {}

    // give sneak attack/defence targets bigger priority than expanding ones
    bool operator()(const target_t &t1, const target_t &t2) const {
        if ((t1._kind == attack || t1._kind == defence) &&
             t2._kind == expand)
             return true;
        return false;
    }

    bool operator()(target_t t) {
        if (_kind != target_kind_last)
            return t._kind == _kind;
        else if (_owner != race_last)
            return t._owner == _owner;
        assert(false);
        return false;
    }
};


struct simple_closeness {
    const Game      *_game;
    const PlanetMap *_map;
    Race            _owner;
    simple_closeness(const Game *game, const PlanetMap *map, Race owner) :
        _game(game), _map(map), _owner(owner) {}

    bool operator()(const target_t &t1, const target_t &t2) const {
        return score(t1._id) > score(t2._id);
    }

    bool operator()(plid_t p1, plid_t p2) const {
        return score(p1) > score(p2);
    }

    double score(plid_t id) const {
        double ret = 0.0;
        uint16_t num = 0;
        for (plid_t id2=0; id2<_game->num_planets(); id2++) {
            double distance = (id==id2 ? 1.0 : (double)_map->distance(id, id2));
            if (_game->owner(id2) != _owner) continue;
            ret += distance;
            num++;
        }
        if (num != 0) ret /= (double)num;
        return ret;
    }
};

struct field_closeness {
    const Game      *_game;
    const PlanetMap *_map;
    Race            _owner;
    field_closeness(const Game *game, const PlanetMap *map, Race owner) :
        _game(game), _map(map), _owner(owner) {}

    bool operator()(const target_t &t1, const target_t &t2) const {
        return score(t1._id) < score(t2._id);
    }

    bool operator()(plid_t p1, plid_t p2) const {
        return score(p1) < score(p2);
    }

    double score(plid_t id) const {
        double ret = 0.0;
        for (plid_t id2=0; id2<_game->num_planets(); id2++) {
            // skipping the planet screws up scoring, instead
            // emulate current planet as it's in 1 turn flight
            double distance = (id==id2 ? 1.0 : (double)_map->distance(id, id2));
            if (_game->owner(id2) == neutral) continue;
            double sign = (_game->owner(id2) == _owner) ? 1.0 : -1.0;
            ret -= sign * (double)(_game->ships(id2)+1) / distance;
        }
        return ret;
    }
};

struct growth_closeness {
    const Game      *_game;
    const PlanetMap *_map;
    Race            _owner;
    growth_closeness(const Game *game, const PlanetMap *map, Race owner) :
        _game(game), _map(map), _owner(owner) {}

    bool operator()(const target_t &t1, const target_t &t2) const {
        return score(t1._id) < score(t2._id);
    }

    bool operator()(plid_t p1, plid_t p2) const {
        return score(p1) < score(p2);
    }

    double score(plid_t id) const {
        double ret = 0.0;
        for (plid_t id2=0; id2<_game->num_planets(); id2++) {
            // skipping the planet screws up scoring, instead
            // emulate current planet as it's in 1 turn flight
            double distance = (id==id2 ? 1.0 : (double)_map->distance(id, id2));
            if (_game->owner(id2) != _owner) continue;
            ret -= (double)(_map->growth(id2)) / distance;
        }
        return ret;
    }
};

#endif // SORTFUNCS_H
