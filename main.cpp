#include <iostream>
#include <stdlib.h>
#include <errno.h>

#include "MyBot.h"
#include "config.h"

using namespace std;

void print_usage_and_exit();

int main(int argc, char **argv) {

    if (argc > 1)
        cfg.parse(argv[1]);
    else
        cfg.parse("default.cfg");

    if (cfg._verbose)
        cerr << cfg;

    MyBot bot;
    bot.run();

    return 0;
}

void print_usage_and_exit() {
    cout << "Usage: ./bot <timeout> <true>" << endl;
    exit(1);
}
