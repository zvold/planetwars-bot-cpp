#include "race.h"

#include "utils.h"

std::string name(Race owner) {
    switch (owner) {
        case ally:    return "ally";
        case enemy:   return "enemy";
        case neutral: return "neutral";
        default:
            assert(false);
            return "none";
    }
}

Race opposite(Race owner) {
    switch (owner) {
        case ally:    return enemy;
        case enemy:   return ally;
        case neutral: return neutral;
        default:
            assert(false);
            return neutral;
    }
}
