#include "config.h"

#include <fstream>
#include <iostream>

using std::ifstream;
using std::istringstream;
using std::endl;
using std::cout;

Config::Config() {
    _max_expands = 10;
    _max_permuts = 1;
    _sim_depth   = 30;
    _verbose     = false;
    _timeout     = 1000;
    _threshold   = 100;
    _fallback    = 10;
    _fast_expand = 1.25f;
    _src_radius  = 17;
    _turn_limit  = 200;
    _reinf_thresh= 1.25f;
    _max_future  = 18;
    _exp_cutoff  = 0.5f;
}

void Config::parse(string file) {
    ifstream state(file.c_str());

    string line;
    while (getline(state, line))
        parse_line(line);

    state.close();
}

void Config::parse_line(string line) {
    istringstream stream(line);
    string dummy;
    if (line.find("max_expands", 0) == 0) {
        stream >> dummy >> _max_expands;
    } else if (line.find("max_permuts", 0) == 0) {
        stream >> dummy >> _max_permuts;
    } else if (line.find("sim_depth", 0) == 0) {
        stream >> dummy >> _sim_depth;
    } else if (line.find("verbose", 0) == 0) {
        string val;
        stream >> dummy >> val;
        _verbose = (val.compare("true") == 0);
    } else if (line.find("timeout", 0) == 0) {
        stream >> dummy >> _timeout;
    } else if (line.find("threshold", 0) == 0) {
        stream >> dummy >> _threshold;
    } else if (line.find("fallback", 0) == 0) {
        stream >> dummy >> _fallback;
    } else if (line.find("fast_expand", 0) == 0) {
        stream >> dummy >> _fast_expand;
    } else if (line.find("src_radius", 0) == 0) {
        stream >> dummy >> _src_radius;
    } else if (line.find("turn_limit", 0) == 0) {
        stream >> dummy >> _turn_limit;
    } else if (line.find("reinf_thresh", 0) == 0) {
        stream >> dummy >> _reinf_thresh;
    } else if (line.find("max_future", 0) == 0) {
        stream >> dummy >> _max_future;
    } else if (line.find("exp_cutoff", 0) == 0) {
        stream >> dummy >> _exp_cutoff;
    }
}

std::ostream & operator<<(std::ostream &out, const Config &cfg) {
    out << "# max_expands: " << cfg._max_expands << endl
        << "# max_permuts: " << cfg._max_permuts << endl
        << "# sim_depth  : " << cfg._sim_depth << endl
        << "# fallback   : " << cfg._fallback << endl
        << "# fast_expand: " << cfg._fast_expand << endl
        << "# src_radius : " << cfg._src_radius << endl
        << "# reinf_thresh: " << cfg._reinf_thresh << endl
        << "# max_future : " << cfg._max_future << endl
        << "# exp_cutoff : " << cfg._exp_cutoff << endl
        << "# turn_limit : " << cfg._turn_limit << endl
        << "# verbose    : " << (cfg._verbose ? "true" : "false") << endl
        << "# timeout    : " << cfg._timeout << endl
        << "# threshold  : " << cfg._threshold << endl;
    return out;
}

Config cfg;
