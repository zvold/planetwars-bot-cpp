#ifndef FLEET_H
#define FLEET_H

#include "utils.h"

#include "race.h"

class Fleet {
private:
    Race        _owner;
    ships_t     _ships;
    plid_t      _to;
    turn_t      _eta;

public:
    Fleet(Race owner, ships_t ships, plid_t to, turn_t eta);

    turn_t  eta()   const {return _eta;}
    Race    owner() const {return _owner;}
    ships_t ships() const {return _ships;}
    plid_t  to()    const {return _to;}

};

#endif // FLEET_H
