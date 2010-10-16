#ifndef RACE_H
#define RACE_H

#include "utils.h"

enum Race {
    neutral   = 0,
    ally      = 1,
    enemy     = 2,
    race_last = 3
};

std::string name(Race owner);
Race opposite(Race owner);

#endif // RACE_H
