#include "../game.h"
#include <gtest/gtest.h>

#include <stdint.h>
#include <string>
#include <algorithm>

#include "../MyBot.h"
#include "../simulator.h"
#include "../race.h"
#include "../sortfuncs.h"
#include "testutils.h"

TEST(TargetTest, TargetSelect) {
    MyBot bot;
    string state[] = {"P 14.4 3.3 0 20 5",
                      "F 1 30 42 0 10 3",
                      "F 2 23 42 0 10 3"};
    parse_state(bot, state, LENGTH(state));

    Simulator sim(bot.game(), bot.map(), 10);
    vector<target_t> targets = sim.select_targets(enemy);
    EXPECT_TRUE(std::find(targets.begin(), targets.end(), target_t(0, ally, 13, 4, attack)) != targets.end())
        << "sneak attack for enemy";
}

TEST(TargetTest, TargetSelect2) {
    MyBot bot;
    string state[] = {"P 14.4 3.3 1 10 1",
                      "F 2 13 42 0 10 3",
                      "F 2  2 42 0 10 4",
                      "F 1  3 42 0 10 6",
                      "F 1  3 42 0 10 8"};
    parse_state(bot, state, LENGTH(state));

    Simulator sim(bot.game(), bot.map(), 10);

    vector<target_t> targets = sim.select_targets(enemy);
    cerr << targets;
    EXPECT_TRUE(std::find(targets.begin(), targets.end(), target_t(0, enemy, 1, 8, defence)) != targets.end())
        << "sneak defence for enemy";

    targets = sim.select_targets(ally);
    EXPECT_TRUE(std::find(targets.begin(), targets.end(), target_t(0, ally, 1, 4, defence)) != targets.end())
        << "sneak defence for ally";
}

TEST(TargetTest, TargetSelect3) {
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

    vector<target_t> targets = sim.select_targets(enemy);
    EXPECT_TRUE(std::find(targets.begin(), targets.end(), target_t(1, ally, 116, 1, expand)) != targets.end())
        << "earliest attack on opposite";

    targets = sim.select_targets(ally);
    EXPECT_TRUE(std::find(targets.begin(), targets.end(), target_t(0, enemy, 78, 9, attack)) != targets.end())
        << "ally sneak attack";
    EXPECT_TRUE(std::find(targets.begin(), targets.end(), target_t(2, enemy, 39, 1, expand)) != targets.end())
        << "earliest attack on opposite";
}
