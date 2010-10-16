#ifndef FUTUREORDER_H
#define FUTUREORDER_H

#include "utils.h"

#include "race.h"

class FutureOrder {
    friend class Move;
    friend class Simulator;

protected:
    turn_t      _turn;
    plid_t      _from;
    plid_t      _to;
    ships_t     _ships;
    Race        _owner;

public:
    FutureOrder(Race owner, ships_t ships, plid_t from, plid_t to, turn_t turn);

    // debugging
    std::string to_string() const;

    turn_t  turn()  const {return _turn;}
    plid_t  from()  const {return _from;}
    plid_t  to()    const {return _to;}
    ships_t ships() const {return _ships;}
    Race    owner() const {return _owner;}

    bool operator==(const FutureOrder &o) const {
        return _turn == o._turn &&
               _from == o._from &&
               _to == o._to &&
               _ships == o._ships &&
               _owner == o._owner;
    }

};

struct order_less {
    bool operator()(const FutureOrder &o1, const FutureOrder &o2) {
        return o1.turn() < o2.turn() ||
               o1.from() < o2.from() ||
               o1.to() < o2.to() ||
               o1.ships() < o2.ships() ||
               o1.owner() < o2.owner();
    }
};

#endif // FUTUREORDER_H
