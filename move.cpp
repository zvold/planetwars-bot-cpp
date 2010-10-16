#include "move.h"

#include <algorithm>
#include <iostream>

#include "utils.h"
#include "basebot.h"

using std::cout;
using std::cerr;
using std::endl;

Move::Move() {}
Move::Move(const Move &move) {add(move);}
Move::~Move() {}

Move::Move(Move *m1) {add(m1);}
Move::Move(Move *m1, Move *m2) {add(m1); add(m2);}
Move::Move(Move *m1, Move *m2, Move *m3){add(m1); add(m2); add(m3);}

void Move::add(const FutureOrder &order) {
    _orders.push_back(order);
}

void Move::add(const Move *move) {
    assert(move != NULL);
    _orders.insert(_orders.end(), move->orders().begin(), move->orders().end());
}

void Move::add(const Move &move) {add(&move);}

const vector<FutureOrder> & Move::orders() const {
    return _orders;
}

std::ostream & operator<<(std::ostream &out, const Move *move) {
    if (move == NULL) {
        out << "Move: (null)" << std::endl;
        return out;
    }
    out << "Move of size " << move->size() << ":" << std::endl;
    vector<FutureOrder>::const_iterator i;
    const vector<FutureOrder> orders = move->orders();
    for (i=orders.begin(); i<orders.end(); i++)
        out << i->to_string() << std::endl;
    return out;
}

void Move::advance() {
    for (vector<FutureOrder>::iterator i=_orders.begin(); i<_orders.end(); i++) {
        i->_turn--; // turn 0 marked for removal
    }
}

struct invalid_order {
    bool operator()(const FutureOrder &order) {
        return order.turn() == (turn_t)-1;
    }
};

bool Move::sanitize() {
    vector<FutureOrder>::iterator start = std::remove_if(_orders.begin(), _orders.end(), invalid_order());
    if (start != _orders.end()) {
        _orders.erase(start, _orders.end());
        return true;
    }
    return false;
}

struct target_order {
    plid_t _id;
    Race _owner;
    target_order(plid_t id, Race owner) : _id(id), _owner(owner) {}
    bool operator()(const FutureOrder &order) {
        return (_owner != race_last) ?
               (order.to() == _id && order.owner() == _owner) :
               (order.to() == _id);
    }
};

void Move::remove_targeted(plid_t id, Race attacker) {
    _orders.erase(std::remove_if(_orders.begin(), _orders.end(), target_order(id, attacker)),
                  _orders.end());
}

void Move::issue_orders(const BaseBot &bot) const {
    for (vector<FutureOrder>::const_iterator j=_orders.begin(); j<_orders.end(); j++) {
        assert(j->owner() == ally);
        if (j->turn() == 0)
            bot.issue_order(j->from(), j->to(), j->ships());
    }
}

