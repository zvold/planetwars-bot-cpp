#include "../game.h"
#include <gtest/gtest.h>

#include <stdint.h>
#include <string>
#include <algorithm>
#include <fstream>

#include "../MyBot.h"
#include "../simulator.h"
#include "../race.h"
#include "../sortfuncs.h"
#include "../attacks.h"
#include "../futureorder.h"
#include "../config.h"
#include "testutils.h"

using std::ifstream;

TEST(AttackTest, SimpleAttack) {
    MyBot bot;
    ifstream state("test/test2.log");
    parse_state(bot, state);

    Simulator sim(bot.game(), bot.map(), 15);
    vector<target_t> targets = sim.select_targets(ally);

    Move *move = exact_attack(sim, targets[21], ally);
    EXPECT_TRUE(move != NULL) << "sneak attack move created";

    vector<FutureOrder> orders = move->orders();
    EXPECT_TRUE(std::find(orders.begin(), orders.end(), FutureOrder(ally, 91, 2, 22, 4)) != orders.end())
        << "91:ally 2->22, at 4";

    if (move != NULL)
        delete move;
}

TEST(AttackTest, SimpleAttack2) {
    MyBot bot;

    string state[] = {"P  3.0  3.0 1  5 3",
                      "P  5.0 10.0 1  5 1",
                      "P 11.0  6.0 0 10 2",
                      "P 15.0  6.0 0 10 1",
                      "F 2 21 10 2 50 11",
                      "F 2 15 10 3 50 12"};
    parse_state(bot, state, LENGTH(state));

    Simulator sim(bot.game(), bot.map(), 15);
    vector<target_t> targets = sim.select_targets(ally);

    Move *move1 = exact_attack(sim, targets[0], ally);
    EXPECT_TRUE(move1 != NULL) << "exact_attack() succeeds";

    sim.simulate(*move1, 15, true);
    Move *move2 = exact_attack(sim, targets[1], ally);
    EXPECT_TRUE(move2 != NULL) << "exact_attack() succeeds";

    vector<FutureOrder> orders = move1->orders();
    EXPECT_TRUE(std::find(orders.begin(), orders.end(), FutureOrder(ally, 5, 1, 2, 4)) != orders.end())
        << "5:ally 1->2, at 4";
    EXPECT_TRUE(std::find(orders.begin(), orders.end(), FutureOrder(ally, 9, 0, 2, 3)) != orders.end())
        << "9:ally 0->2, at 3";

    orders = move2->orders();
    EXPECT_TRUE(std::find(orders.begin(), orders.end(), FutureOrder(ally, 3, 1, 3, 2)) != orders.end())
        << "3:ally 1->3, at 2";
    EXPECT_TRUE(std::find(orders.begin(), orders.end(), FutureOrder(ally, 4, 0, 3, 0)) != orders.end())
        << "4:ally 0->3, at 0";

    delete move1;
    delete move2;
}

TEST(AttackTest, WaitingAttack) {
    Move *move;
    vector<Move*> moves;
    MyBot bot;

    string state[] = {"P  3.0  3.0 1  0 7",
                      "P  3.0  6.0 1  0 6",
                      "P 11.0  6.0 2 10 2"};
    parse_state(bot, state, LENGTH(state));

    Simulator sim(bot.game(), bot.map(), 0);
    sim.simulate(15, true);
    vector<target_t> targets = sim.select_targets(ally);

    move = waiting_attack(sim, targets[0], ally);
    EXPECT_TRUE(move != NULL) << "waiting attack succeeds";
    vector<FutureOrder> orders = move->orders();
    EXPECT_TRUE(std::find(orders.begin(), orders.end(), FutureOrder(ally, 18, 1, 2, 4)) != orders.end())
        << "18:ally 1->2, at 4";
    EXPECT_TRUE(std::find(orders.begin(), orders.end(), FutureOrder(ally, 17, 0, 2, 3)) != orders.end())
        << "17:ally 0->2, at 3";
    delete move;

    move = waiting_attack(sim, target_t(2, enemy, 37, 13, expand), ally);
    EXPECT_TRUE(move != NULL) << "waiting attack succeeds";
    orders = move->orders();
    EXPECT_TRUE(std::find(orders.begin(), orders.end(), FutureOrder(ally, 19, 1, 2, 5)) != orders.end())
        << "19:ally 1->2, at 5";
    EXPECT_TRUE(std::find(orders.begin(), orders.end(), FutureOrder(ally, 18, 0, 2, 4)) != orders.end())
        << "18:ally 0->2, at 4";
    delete move;
}

TEST(AttackTest, FutureOrders) {
    Move *move = new Move();
    move->add(FutureOrder(ally,  1, 0, 1, 1));
    move->add(FutureOrder(ally,  1, 0, 1, 1)); // same
    move->add(FutureOrder(enemy, 1, 0, 1, 1));
    move->add(FutureOrder(ally,  2, 0, 1, 2));
    move->add(FutureOrder(ally,  2, 0, 1, 2)); // same
    move->add(FutureOrder(ally,  3, 0, 1, 1));
    move->add(FutureOrder(ally,  1, 0, 1, 3));
    move->add(FutureOrder(ally,  1, 1, 1, 3));
    move->add(FutureOrder(ally,  1, 0, 2, 3));
    move->add(FutureOrder(ally,  3, 0, 2, 3));

    vector<Move*> moves;
    moves.push_back(move);

    EXPECT_EQ((unsigned int)10, total_orders(moves)) << "10 orders total";
    EXPECT_EQ((unsigned int)8, unique_orders(moves)) << "8 unique orders";

    delete move;
}

TEST(AttackTest, MoveCompose) {
    MyBot bot;
    ifstream state("test/test3.log");
    parse_state(bot, state);

    Simulator sim(bot.game(), bot.map(), SIM_DEPTH);
    vector<target_t> targets = sim.select_targets(ally);

    Move m;
    vector<pair<Move, vector<target_t> > > ally_shuf_targets, enemy_shuf_targets;
    shuffle_targets(sim, m, targets, ally_shuf_targets, ally);

    for (vector<pair<Move, vector<target_t> > >::iterator i=ally_shuf_targets.begin();
                                                          i<ally_shuf_targets.end();
                                                          i++) {
        Move *a_move = create_move(sim, i->first, i->second.begin(), i->second.end(), ally);
        sim.simulate(*a_move, SIM_DEPTH, true);

        targets = sim.select_targets(enemy);
        enemy_shuf_targets.clear();
        shuffle_targets(sim, m, targets, enemy_shuf_targets, enemy);
        delete a_move;
    }

    EXPECT_NO_FATAL_FAILURE();
}

TEST(AttackTest, SimulatorBug) {
    MyBot bot;
    ifstream state("test/test5.log");
    parse_state(bot, state);

    Simulator sim(bot.game(), bot.map(), SIM_DEPTH);

    vector<target_t> targets;
    targets.push_back(target_t(0, ally, 76, 1, expand));
    targets.push_back(target_t(17, neutral, 64, 1, expand));
    targets.push_back(target_t(12, ally, 30, 1, expand));
    targets.push_back(target_t(19, ally, 34, 1, expand));
    targets.push_back(target_t(18, ally, 13, 1, expand));

    Move move;
    move.add(FutureOrder(ally, 51, 19, 0, 6));
    move.add(FutureOrder(ally, 25, 15, 0, 3));

    sim.simulate(move, SIM_DEPTH, true);

    Move *m = create_move(sim, move, targets.begin(), targets.end(), enemy);
    EXPECT_TRUE(m != NULL) << "create_move() succeeds";

    delete m;
}

TEST(AttackTest, SimulatorBug2) {
    MyBot bot;
    ifstream state("test/test4.log");
    parse_state(bot, state);

    Simulator sim(bot.game(), bot.map(), SIM_DEPTH);
    vector<target_t> targets = sim.select_targets(ally);

    Move empty;
    vector<pair<Move, vector<target_t> > > ally_shuf_targets, enemy_shuf_targets;
    shuffle_targets(sim, empty, targets, ally_shuf_targets, ally);

    vector<pair<Move, vector<target_t> > >::iterator i = ally_shuf_targets.begin();
    Move *a_move = create_move(sim, i->first, i->second.begin(), i->second.end(), ally);

    sim.simulate(*a_move, SIM_DEPTH, true);
    targets = sim.select_targets(enemy);

    std::sort(targets.begin(), targets.end(), simple_closeness(sim.game(), sim.pmap(), ally));
    std::sort(targets.begin(), targets.end(), target_kind_comp(sim.game(), sim.pmap()));
    Move *m = create_move(sim, *a_move, targets.begin(), targets.end(), enemy);

    delete m;
    delete a_move;
}

TEST(AttackTest, AttackTargetBug) {
    MyBot bot;
    ifstream state("test/test6.log");
    parse_state(bot, state);

    Move move;
    move.add(FutureOrder(ally, 30, 2, 5, 3));

    Simulator sim(bot.game(), bot.map(), 0);
    sim.simulate(move, SIM_DEPTH, true);

    vector<target_t> targets = sim.select_targets(ally);
    EXPECT_TRUE(std::find_if(targets.begin(), targets.end(), target_kind_comp(attack)) == targets.end());
}
