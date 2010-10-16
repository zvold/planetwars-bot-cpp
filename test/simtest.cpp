#include "../game.h"
#include <gtest/gtest.h>

#include <stdint.h>
#include <string>

#include "../MyBot.h"
#include "../simulator.h"
#include "../race.h"
#include "../move.h"
#include "testutils.h"

TEST(SimulatorTest, Simulate) {
    MyBot bot;
    string state[] = {"P 14.4 3.3 0 10 1",
                      "F 1 10 42 0 10 3",
                      "F 2  1 42 0 10 4"};
    parse_state(bot, state, LENGTH(state));

    EXPECT_TRUE(bot.game()->fleets()->find(10) == bot.game()->fleets()->end()) << "check fleets searching";
    EXPECT_EQ((unsigned int)1, bot.game()->fleets()->find(3)->second.size()) << "1 fleet at turn 3";
    EXPECT_EQ((unsigned int)1, bot.game()->fleets()->find(4)->second.size()) << "1 fleet at turn 4";

    Simulator sim(bot.game(), bot.map(), 10);

    EXPECT_EQ((ships_t)0, sim.ships_avail(0, 3, enemy)) << "0 enemy ships at 3rd turn";
    EXPECT_EQ((ships_t)1, sim.ships_avail(0, 4, enemy)) << "1 enemy ships at 4th turn";
    EXPECT_EQ((ships_t)0, sim.ships_avail(0, 3, ally))  << "0 ally ships at 3rd turn";
    EXPECT_EQ((ships_t)0, sim.ships_avail(0, 4, ally))  << "0 ally ships at 4th turn";
}

TEST(SimulatorTest, Simulate2) {
    MyBot bot;
    string state[] = {"P 14.4 3.3 0 20 5",
                      "F 1 30 42 0 10 3",
                      "F 2 23 42 0 10 3"};
    parse_state(bot, state, LENGTH(state));

    Simulator sim(bot.game(), bot.map(), 10);

    EXPECT_EQ((ships_t)0, sim.ships_avail(0, 3, enemy)) << "0 enemy ships at 3rd turn";
    EXPECT_EQ((ships_t)0, sim.ships_avail(0, 4, enemy)) << "0 enemy ships at 4th turn";
    EXPECT_EQ((ships_t)0, sim.ships_avail(0, 2, ally))  << "0 ally ships at 2nd turn";
    EXPECT_EQ((ships_t)7, sim.ships_avail(0, 3, ally))  << "7 ally ships at 3rd turn";
}

TEST(SimulatorTest, Simulate3) {
    MyBot bot;
    string state[] = {"P 14.4 3.3 1 10 1",
                      "F 2 13 42 0 10 3",
                      "F 2  2 42 0 10 4",
                      "F 1  3 42 0 10 6",
                      "F 1  3 42 0 10 8"};
    parse_state(bot, state, LENGTH(state));

    Simulator sim(bot.game(), bot.map(), 10);

    vector<plstate_t> const * states = sim.planet_states(0);

    plstate_t s = states->at(4);
    EXPECT_EQ(enemy, s._owner) << "turn 4 ownership change to 1:enemy";
    EXPECT_EQ(1, s._ships)     << "turn 4 ownership change to 1:enemy";

    s = states->at(8);
    EXPECT_EQ(ally, s._owner)  << "turn 8 ownership change to 1:ally";
    EXPECT_EQ(1, s._ships)     << "turn 8 ownership change to 1:ally";
}

TEST(SimulatorTest, SimulateNoChange) {
    MyBot bot;
    string state[] = {"P 16.8 20.3 2 5 2",
                      "F 1 1  1 0 5 1",
                      "F 1 1  1 0 5 2",
                      "F 1 1  1 0 5 3",
                      "F 1 1 21 0 6 4",
                      "F 1 1  1 0 5 4",
                      "F 1 1 15 0 2 1"};
    parse_state(bot, state, LENGTH(state));

    Simulator sim(bot.game(), bot.map(), 10);

    vector<plstate_t> const * states = sim.planet_states(0);
    for (vector<plstate_t>::const_iterator i=states->begin(); i<states->end(); i++)
        EXPECT_EQ(enemy, i->_owner) << "ownership doesn't change";
}

TEST(SimulatorTest, SimulateNoChange2) {
    MyBot bot;
    string state[] = {"P 1.1 14.3 1 25 1",
                      "F 2 14  1 0 7 1",
                      "F 2 18  1 0 7 4",
                      "F 1  2 21 0 4 1",
                      "F 1  5  9 0 4 1",
                      "F 1  4 12 0 2 1"};
    parse_state(bot, state, LENGTH(state));

    Simulator sim(bot.game(), bot.map(), 50);

    vector<plstate_t> const * states = sim.planet_states(0);
    for (vector<plstate_t>::const_iterator i=states->begin(); i<states->end(); i++)
        EXPECT_EQ(ally, i->_owner) << "ownership doesn't change";

    sim.simulate(10, true);
    EXPECT_EQ((unsigned int)10, sim.planet_states(0)->size()) << "number of states after re-sim";
}

TEST(SimulatorTest, Simulate4) {
    MyBot bot;
    string state[] = {"P 10.8 11.5 0   1 1",
                      "P 19.4  6.6 1 110 5",
                      "P  2.2 16.3 2  33 5",
                      "F 2 50 2 0 10 8",
                      "F 2 27 2 0 10 9",
                      "F 1 51 2 1 10 8",
                      "F 1 51 2 1 10 8"};
    parse_state(bot, state, LENGTH(state));

    Simulator sim(bot.game(), bot.map(), 10);
    vector<plstate_t> const * states = sim.planet_states(0);

    plstate_t s = states->at(8);
    EXPECT_EQ(enemy, s._owner)  << "turn 8 state 49:enemy";
    EXPECT_EQ(49, s._ships)     << "turn 8 state 49:enemy";

    s = states->at(9);
    EXPECT_EQ(enemy, s._owner)  << "turn 9 state 77:enemy";
    EXPECT_EQ(77, s._ships)     << "turn 9 state 77:enemy";

    // re-simulation
    sim.simulate(9, true);
    states = sim.planet_states(0);
    s = states->at(8);
    EXPECT_EQ(enemy, s._owner)  << "turn 8 state 49:enemy after re-sim";
    EXPECT_EQ(49, s._ships)     << "turn 8 state 49:enemy after re-sim";
}

TEST(SimulatorTest, SimulateFuture) {
    MyBot bot;
    string state[] = {"P 14.425990 3.301771 0 10 1",
                      "P 14.290286 8.040786 0  0 0",
                      "F 1 10 42 0 10 3",
                      "F 2  1 42 0 10 4"};
    parse_state(bot, state, LENGTH(state));

    EXPECT_EQ(5, bot.map()->distance(0, 1)) << "check 0-1 distance";

    Simulator sim(bot.game(), bot.map(), 0); // no initial simulation
    Move move;
    move.add(FutureOrder(enemy, 1, 0, 1, 4)); // 1 enemy ship from 0 to 1 at turn 4

    sim.simulate(move, 10, true);

    EXPECT_TRUE(plstate_t(enemy, 5) == sim.planet_states(0)->at(9)) << "planet 0 - 5:enemy at turn 9";
    EXPECT_TRUE(plstate_t(enemy, 1) == sim.planet_states(1)->at(9)) << "planet 1 - 1:enemy at turn 9";

    sim.simulate(move, 21, true);
    EXPECT_TRUE(plstate_t(enemy, 1) == sim.planet_states(1)->at(19)) << "planet 1 - 1:enemy at turn 19";
}

TEST(SimulatorTest, SimulateFuture2) {
    MyBot bot;
    string state[] = {"P 14.42 3.30 0 10 1",
                      "P 14.29 8.04 0  0 0",
                      "F 1 10 42 0 10 3",
                      "F 2  1 42 0 10 4",
                      "F 1  2 42 0 10 5"};
    parse_state(bot, state, LENGTH(state));

    EXPECT_EQ(5, bot.map()->distance(0, 1)) << "check 0-1 distance";

    Simulator sim(bot.game(), bot.map(), 0); // no initial simulation
    Move move;
    move.add(FutureOrder(enemy, 1, 0, 1, 4)); // 1 enemy ship from 0 to 1 at turn 4

    sim.simulate(move, 10, true);

    EXPECT_TRUE(plstate_t(ally, 5) == sim.planet_states(0)->at(9))  << "planet 0 - 5:ally at turn 9";
    EXPECT_TRUE(plstate_t(enemy, 1) == sim.planet_states(1)->at(9)) << "planet 1 - 1:enemy at turn 9";

    sim.simulate(move, 21, true);
    EXPECT_TRUE(plstate_t(enemy, 1) == sim.planet_states(1)->at(19)) << "planet 1 - 1:enemy at turn 19";
}

