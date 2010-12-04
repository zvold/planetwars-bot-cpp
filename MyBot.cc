#include "MyBot.h"

#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <algorithm>
#include <set>

#include "simulator.h"
#include "attacks.h"
#include "config.h"
#include "futureorder.h"
#include "sortfuncs.h"
#include "fastexpand.h"

using namespace std;

MyBot::MyBot() : BaseBot(), sim(&_game, &_map, 0) {
}

MyBot::~MyBot() {
}

void delete_moves(vector<Move*> &moves) {
    vector<Move*>::const_iterator j;
    for (j=moves.begin(); j<moves.end(); j++) {
        assert(*j != NULL);
        delete *j;
    }
}

void MyBot::fill_targets(Move &move, vector<pair<Move, vector<target_t> > > &shuf_targets, Race attacker) {
    sim.simulate_safe(move, SIM_DEPTH);
    vector<target_t> targets = sim.select_targets(attacker);
    shuffle_targets(sim, move, targets, shuf_targets, attacker);
}

void MyBot::force_defence(Move &move, vector<Move*> &pre_moves, Race attacker) {
    // sort all move's targeted planets by closeness
    set<plid_t> planets;
    vector<FutureOrder>::const_iterator i;
    for (i=move.orders().begin(); i<move.orders().end(); i++)
        planets.insert(i->to());
    vector<plid_t> vplanets(planets.begin(), planets.end());
    std::sort(vplanets.begin(), vplanets.end(), field_closeness(sim.game(), sim.pmap(), attacker));

    vector<target_t> targets;
    // force sneak defence for previous move
    sim.simulate_safe(move, SIM_DEPTH);
    targets = sim.select_targets(attacker);
    Move *m = sneak_defence(sim, move, targets, attacker);
    if (m != NULL) pre_moves.push_back(m);

    // repeat with subsequently reduced initial move
    for (unsigned int k=0; k<FALLBACK; k++) {
        if (vplanets.empty()) break;
        vector<plid_t>::iterator p = vplanets.end() - 1;
        move.remove_targeted(*p, attacker);
        vplanets.erase(p);

        sim.simulate_safe(move, SIM_DEPTH);
        targets = sim.select_targets(attacker);
        Move *m = sneak_defence(sim, move, targets, attacker);
        if (m != NULL) pre_moves.push_back(m);
    }

    // finally - force defence for completely discarded previous move
    if (!move.empty()) {
        move.clear();
        sim.simulate_safe(move, SIM_DEPTH);
        targets = sim.select_targets(attacker);
        Move *m = sneak_defence(sim, move, targets, attacker);
        if (m != NULL) pre_moves.push_back(m);
    }
}

void MyBot::do_turn() {
    if (VERBOSE)
        cerr << "# profile:  \t" << get_profile() << endl;
    if (_turn == 0) {
        FastExpand(*this, sim).execute();
        return;
    }

    vector<pair<Move, vector<target_t> > > ally_shuf_targets, enemy_shuf_targets;
    vector<target_t>  targets;

    // prepare pre-moves based on previous move and forced sneak defence commands
    vector<Move*> pre_moves;
    force_defence(_prev_move, pre_moves, ally);
    uint16_t def_num = (uint16_t)pre_moves.size();

    reinforce(sim, pre_moves, ally);
    if (VERBOSE)
        cerr << "# pre-moves:\t" << def_num << "(" << pre_moves.size() << ")" << endl;

    for (vector<Move*>::iterator m=pre_moves.begin(); m<pre_moves.end(); m++) {
        fill_targets(**m, ally_shuf_targets, ally);
        delete *m;
    }

    if (VERBOSE && !ally_shuf_targets.empty())
        cerr << "# targets:  \t" << ally_shuf_targets.size() << endl;

    Move best_move;
    bool bailout = false;
    pair<int32_t, profile> score_minmax  = make_pair<int32_t, profile>(-65535, profile());
    pair<int32_t, profile> score_min_row = make_pair<int32_t, profile>( 65535, profile());
    uint16_t num_variants = 0, num_rows = 0;

    vector<pair<Move, vector<target_t> > >::iterator a;
    for (a=ally_shuf_targets.begin(); a<ally_shuf_targets.end() && !bailout; a++) {
        num_rows++;
        score_min_row.first = 65535;

        Move *a_move = create_move(sim, a->first, a->second.begin(), a->second.end(), ally);

//        if (a_move->empty()) {
//            delete a_move;
//            continue;
//        }

        sim.simulate_safe(*a_move, SIM_DEPTH);

        targets = sim.select_targets(enemy);
        enemy_shuf_targets.clear();
        shuffle_targets(sim, _prev_move, targets, enemy_shuf_targets, enemy);

        bailout = timeout();

        vector<pair<Move, vector<target_t> > >::iterator e;
        for (e=enemy_shuf_targets.begin(); e<enemy_shuf_targets.end() && !bailout; e++) {
            for (int16_t flag=0; flag<=1 && !bailout; flag++) {
                num_variants++;
                bool safe = (flag == 1);

                Move *e_move = create_move(sim, *a_move, e->second.begin(), e->second.end(), enemy, safe);
                if ((bailout = timeout()) == true) {delete e_move; break;}

                sim.simulate_safe(*e_move, SIM_DEPTH);
                delete e_move;
                if ((bailout = timeout()) == true) break;

                int32_t score = sim.score(_turn);
                if (score < score_minmax.first) break;
                if (score < score_min_row.first) {
                    score_min_row.first = score;
                    score_min_row.second = sim.end_profile();
                }
            }
        }

        if (score_min_row.first != 65535 && score_min_row.first >= score_minmax.first) {
            score_minmax = score_min_row;
            best_move.clear();
            best_move.add(a_move);
        }
        delete a_move;
    }

    if (!best_move.empty()) {
        if (VERBOSE) {
            cerr << "# predict:  \t" << score_minmax.second << endl
                 << "# minmax:   \t"
                 << (score_minmax.first >=0 ? "+" : "")
                 << (int32_t)score_minmax.first
                 << " (" << num_variants << " in " << num_rows << " rows)" << endl;
        }
        _prev_move.clear();
        issue_orders(best_move, _prev_move);
        sim.set_score(score_minmax.first);
    } else
        log("no move selected");

    _prev_move.advance();
    bool invalid = _prev_move.sanitize();
    if (invalid) assert(!invalid);

    if (VERBOSE && _prev_move.size() != 0)
        cerr << "# TODO:     \t" << _prev_move.size() << " orders"
//        << ": " << &_prev_move
        << endl;
}

bool MyBot::timeout() {
    if (_timer.total() > (uint32_t)(TIMEOUT - THRESHOLD)) {
        log("timeout alert at " + str(_timer.total()) + " ms");
        return true;
    }
    return false;
}

void MyBot::issue_orders(const Move &move, Move &prev_move) const {
    vector<FutureOrder>::iterator j;
    vector<FutureOrder> orders = move.orders();
    for (j=orders.begin(); j<orders.end(); j++)
        if (j->owner() == ally) {
            if (j->turn() == 0)
                issue_order(j->from(), j->to(), j->ships());
            else
                prev_move.add(*j);
        }
}
