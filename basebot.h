#ifndef BASEBOT_H
#define BASEBOT_H

#include "utils.h"

#include "game.h"
#include "timer.h"
#include "planetmap.h"
#include "simulator.h"

class BaseBot {

protected:
    bool        _parsing;
    plid_t      _planet_id;

    turn_t      _turn;
    Timer       _timer;
    Game        _game;
    PlanetMap   _map;
    profile     _profile;

public:
    BaseBot();

    void run();

protected:
    virtual void do_turn() = 0;
    // game state parsing helpers
    void parse_planet(string &line);
    void parse_fleet(string &line);
    void init_profile();

public:
    const profile & get_profile() const {return _profile;}
    void log(string const &msg) const;
        void issue_order(plid_t from, plid_t to, ships_t ships) const;
    // these helpers made public to aid testing
    void end_turn();
    void inc_turn() {_turn++;}
    void reset_parser();
    void finish_parsing();

public:
    void parse_line(string &line);

    // debugging
    void dump_planet(plid_t id) const;
    Game const * game() const;
    PlanetMap const * map() const;

};

#endif // BASEBOT_H
