#include "fleet.h"

Fleet::Fleet(Race owner, ships_t ships, plid_t to, turn_t eta) :
    _owner(owner), _ships(ships), _to(to), _eta(eta) {
}
