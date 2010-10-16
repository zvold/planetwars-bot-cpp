#ifndef MOVE_H
#define MOVE_H

#include "utils.h"

#include "futureorder.h"

#include <vector>
#include <iostream>

using std::vector;

class BaseBot;

class Move {
private:
    vector<FutureOrder>     _orders;

public:
    Move();
    Move(const Move &move);
    Move(Move *m1);
    Move(Move *m1, Move *m2);
    Move(Move *m1, Move *m2, Move *m3);

    ~Move();

    void add(const FutureOrder &order);
    void add(const Move &move);
    void add(const Move *move);
    const vector<FutureOrder> & orders() const;

    uint16_t size() const {return _orders.size();}
    bool empty() const {return _orders.empty();}

    vector<FutureOrder> & mod_orders() {return _orders;}
    void clear() {_orders.clear();}
    void advance();
    bool sanitize();
    void issue_orders(const BaseBot &bot) const;
    void remove_targeted(plid_t id);
    void remove_targeted(plid_t id, Race attacker = race_last);
};

std::ostream & operator<<(std::ostream &out, const Move *move);

#endif // MOVE_H
