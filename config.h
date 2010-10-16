#ifndef CONFIG_H
#define CONFIG_H

#include "utils.h"

#define SIM_DEPTH       cfg._sim_depth
#define MAX_EXPANDS     cfg._max_expands
#define MAX_PERMS       cfg._max_permuts
#define VERBOSE         cfg._verbose
#define TIMEOUT         cfg._timeout
#define THRESHOLD       cfg._threshold
#define FALLBACK        cfg._fallback
#define FEXPAND_FACTOR  cfg._fast_expand
#define SRC_RADIUS      cfg._src_radius
#define TURN_LIMIT      cfg._turn_limit
#define REINF_THRESH    cfg._reinf_thresh
#define MAX_FUTURE      cfg._max_future

using std::string;

class Config {
public:
    uint16_t    _max_expands;
    uint16_t    _max_permuts;
    uint16_t    _sim_depth;
    bool        _verbose;
    time_t      _timeout;
    time_t      _threshold;
    uint16_t    _fallback;
    float       _fast_expand;
    uint16_t    _src_radius;
    uint16_t    _turn_limit;
    float       _reinf_thresh;
    uint16_t    _max_future;

    Config();
    void parse(string file);

private:
    void parse_line(string line);
};

std::ostream & operator<<(std::ostream &out, const Config &cfg);

extern Config cfg;

#endif // CONFIG_H
