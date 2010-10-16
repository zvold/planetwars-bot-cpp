
#include "../game.h"
#include <gtest/gtest.h>

#include <stdint.h>
#include <string>

#include "../MyBot.h"
#include "testutils.h"

TEST(GameTest, EmptyGame) {
    Game game;
    EXPECT_EQ(0, game.num_planets()) << "no planets";
}


TEST(GameTest, BasicParsing) {
    MyBot bot;
    string state[] = {"P  3.4  4.3 0 53 1",
                      "P 10.5 5.15 2  1 3",
                      "P 11.4 13.5 1  1 3"};
    parse_state(bot, state, LENGTH(state));
    EXPECT_EQ((unsigned int)3, bot.game()->num_planets())         << "3 planets parsed";
    EXPECT_EQ((unsigned int)0, bot.game()->fleets()->size())      << "zero fleets so far";

    string state2[] = {"F 1 44 1  9  9 1",
                       "F 2 20 2 13 10 2",
                       "F 2 44 2 10  9 1"};
    parse_state(bot, state2, LENGTH(state2));
    EXPECT_EQ((unsigned int)3, bot.game()->num_planets())                    << "3 planets parsed";
    EXPECT_EQ((unsigned int)2, bot.game()->fleets()->size())                 << "2 arrival turns parsed";
    EXPECT_EQ((unsigned int)2, bot.game()->fleets()->find(1)->second.size()) << "2 fleets at turn 1";
    EXPECT_TRUE(bot.game()->fleets()->find(10) == bot.game()->fleets()->end()) << "check fleets searching";
}

TEST(GameTest, BasicParsing2) {
    MyBot bot;
    string state[] = {"P 14.4  3.3 0 41 1",
                      "P 19.4 20.0 0 35 5",
                      "P  3.7  3.2 0 35 5",
                      "F 1 50 1 0 21 20",
                      "F 2 50 2 1  5  4"};
    parse_state(bot, state, LENGTH(state));
    EXPECT_EQ((unsigned int)3, bot.game()->num_planets())         << "3 planets parsed";
    EXPECT_EQ((unsigned int)2, bot.game()->fleets()->size())      << "2 fleets parsed";

    EXPECT_EQ((unsigned int)1, count_fleets(bot.game()->fleets(), ally))  << "1 allied fleet";
    EXPECT_EQ((unsigned int)1, count_fleets(bot.game()->fleets(), enemy)) << "1 enemy fleet";
}

TEST(GameTest, BasicParsingUpdate) {
    MyBot bot;
    string state[] = {"P 14.4  3.3 0 41 1",
                      "P 19.4 20.0 0 35 5",
                      "P  3.7  3.2 0 35 5",
                      "F 1 50 1 0 21 20",
                      "F 2 50 2 1  5  4"};
    string state2[] = {"P 14.4  3.3 2 41 1",
                       "P 19.4 20.0 1 10 5",
                       "P  3.7  3.2 2 40 5",
                       "F 1 50 1 1 21 19",
                       "F 2 50 2 1  5  3"};

    parse_state(bot, state, LENGTH(state));
    parse_state(bot, state2, LENGTH(state2));

    EXPECT_EQ(enemy, bot.game()->owner(0)) << "planet 0 owner";
    EXPECT_EQ(   10, bot.game()->ships(1)) << "planet 1 ships";
    EXPECT_EQ(    5, bot.map()->growth(2)) << "planet 2 growth";

    EXPECT_EQ((unsigned int)1, count_fleets(bot.game()->fleets(), 1, ally))  << "1 allied fleet";
    EXPECT_EQ((unsigned int)1, count_fleets(bot.game()->fleets(), 1, enemy)) << "1 enemy fleet";
}

TEST(GameTest, BasicParsingFleetUpdate) {
    MyBot bot;
    string state[] = {"P 14.4  3.3 0 41 1",
                      "P 19.4 20.0 0 35 5",
                      "P  3.7  3.2 0 35 5",
                      "F 1 50 1 0 21 20",
                      "F 2 50 2 1  5  4"};
    string state2[] = {"P 14.4  3.3 0 41 1",
                       "P 19.4 20.0 0 35 5",
                       "P  3.7  3.2 0 35 5",
                       "F 1 50 3 0 21 19",
                       "F 2 50 4 0  5  3",
                       "F 1 50 1 0 21 19",
                       "F 2 50 2 1  5  3"};

    parse_state(bot, state, LENGTH(state));
    parse_state(bot, state2, LENGTH(state2));

    EXPECT_EQ((unsigned int)2, bot.game()->fleets()->size())                 << "2 distinct arrivals";
    EXPECT_EQ((unsigned int)2, count_fleets(bot.game()->fleets(), 0, ally))  << "1 allied fleet";
    EXPECT_EQ((unsigned int)1, count_fleets(bot.game()->fleets(), 1, enemy)) << "1 enemy fleet";
}

TEST(GameTest, BasicParsingUpdateNoFleets) {
    MyBot bot;
    string  state[] = {"P 14.4  3.3 0 41 1",
                       "P 19.4 20.0 0 35 5",
                       "P  3.7  3.2 0 35 5",
                       "F 1 50 1 0 21 20",
                       "F 2 50 2 1  5  4"};
    string state2[] = {"P 14.4  3.3 0 41 1",
                       "P 19.4 20.0 0 35 5",
                       "P  3.7  3.2 0 35 5",
                       "F 1 50 3 0 21 19",
                       "F 2 50 4 0  5  3",
                       "F 1 50 1 0 21 19",
                       "F 2 50 2 1  5  3"};
    string state3[] = {"P 14.4  3.3 0 41 1",
                       "P 19.4 20.0 0 35 5",
                       "P  3.7  3.2 0 35 5"};

    parse_state(bot, state, LENGTH(state));
    parse_state(bot, state2, LENGTH(state2));
    parse_state(bot, state3, LENGTH(state3));

    EXPECT_EQ((unsigned int)0, bot.game()->fleets()->size())                 << "0 distinct arrivals";
    EXPECT_EQ((unsigned int)11, sum_growth(neutral, bot.game(), bot.map()))  << "11 neutral growth";
    EXPECT_EQ((unsigned int)0, sum_growth(ally, bot.game(), bot.map()))      << "0 ally growth";
    EXPECT_EQ((unsigned int)0, sum_growth(enemy, bot.game(), bot.map()))     << "0 enemy growth";
}
