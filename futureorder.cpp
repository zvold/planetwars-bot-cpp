#include "futureorder.h"

FutureOrder::FutureOrder(Race owner, ships_t ships, plid_t from, plid_t to, turn_t turn) :
    _turn(turn), _from(from), _to(to), _ships(ships), _owner(owner) {
}

std::string FutureOrder::to_string() const {
    return str(_ships) + ":" + name(_owner) + " " +
           str(_from) + "->" + str(_to) +
           ", at " + str(_turn);
}
