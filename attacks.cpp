#include "attacks.h"

#include "sortfuncs.h"
#include "config.h"

#include <algorithm>
#include <set>

using std::set;

bool is_expand(const target_t &t) {
    return t._kind == expand;
}

bool is_not_defence(const target_t &t) {
    return t._kind != defence;
}

// select source planets to attack target from, writing them to sources vector
// returns total ships available if there were enough ships to attack, or 0 otherwise
ships_t select_planets(vector<pair<plid_t, ships_t> > &sources, Race attacker,
                       const target_t &target,
                       const Simulator &sim) {
    ships_t ships_avail = 0;
    const vector<pair<plid_t, turn_t> > neighbors = sim.pmap()->neighbours(target._id);
    vector<pair<plid_t, turn_t> >::const_iterator i;
    for (i=neighbors.begin(); i<neighbors.end(); i++) {
        plid_t src_id = i->first;
        turn_t eta = i->second;
        ships_t ships;
        if (eta <= target._turn &&
            (ships = sim.ships_avail(src_id, target._turn - eta, attacker)) != 0) {
            sources.push_back(make_pair<plid_t, ships_t>(src_id, ships));
            ships_avail += ships;
        }
        if (ships_avail >= target._ships)
            return ships_avail;
    }
    return 0;
}

Move * exact_attack(const Simulator &sim,
                    const target_t &target,
                    Race attacker,
                    bool verbose) {
    vector<pair<plid_t, ships_t> > sources;
    sources.reserve(4);
    ships_t ships_avail = select_planets(sources, attacker, target, sim);
    if (ships_avail==0 || sources.empty()) return NULL;

    ships_t ships_sent = 0;
    double error = 0.0;
    Move *move = new Move();
    vector<pair<plid_t, ships_t> >::iterator i;
    for (i=sources.begin(); i<sources.end(); i++) {
        double frac = (double)target._ships * (double)i->second / (double)ships_avail;
        ships_t to_send = (int)frac;
        error += frac - (double)to_send;
        if (error > 0.9999) {
            error -= 0.9999;
            to_send++;
        }
        assert(to_send <= i->second);
        if (to_send == 0) continue; // don't bother to send
        plid_t src_id = i->first;
        FutureOrder order(attacker, to_send, src_id, target._id,
                          target._turn - sim.pmap()->distance(src_id, target._id));
        move->add(order);
        ships_sent += to_send;
    }
    assert(ships_sent == target._ships);
    if (verbose)
        cerr << "# " << target._id << " attacked with " << ships_sent
             << " ships from " << sources.size() << " sources" << endl;
    return move;
}

turn_t is_enough_ships(const vector<pair<plid_t, turn_t> > &sources, int16_t end_src,
                       const Simulator &sim,
                       const target_t &target,
                       Race attacker,
                       ships_t &ships_avail) {
    for (turn_t att_turn=target._turn; att_turn<sim.turns(); att_turn++) {
        plstate_t target_state = sim.planet_states(target._id)->at(att_turn);
        //assert(target_state._owner != attacker);
        if (target_state._owner == attacker) continue;

        ships_avail = 0;
        for (int16_t idx=0; idx<=end_src; idx++) {
            ships_t ships;
            plid_t src_id = sources[idx].first;
            turn_t eta = sources[idx].second;
            if (eta > att_turn || eta > SRC_RADIUS) continue;

            turn_t dep_turn = att_turn - eta;
            if ((ships = sim.ships_avail(src_id, dep_turn, attacker)) != 0) {
                ships_avail += ships;
                if (ships_avail >= target_state._ships + 1)
                    return att_turn;
            }
        }
    }
    return -1;
}

Move * waiting_attack(const Simulator &sim,
                      const target_t &target,
                      Race attacker,
                      bool verbose) {
    if (verbose)
        cerr << "# waiting attack on planet " << target._id << endl;
    vector<pair<plid_t, turn_t> > sources = sim.pmap()->neighbours(target._id);
    turn_t min_attack_turn = -1;
    int16_t idx_end = -1;
    ships_t ships_avail = 0;
    if (!sources.empty()) {
        int16_t idx = (int16_t)sources.size() - 1;
        turn_t enough_wait = is_enough_ships(sources, idx, sim, target, attacker, ships_avail);
        if (enough_wait < min_attack_turn) {
            min_attack_turn = enough_wait;
            idx_end = idx;
        }
    }
    if (min_attack_turn != (turn_t)-1) {
        assert(idx_end != (turn_t)-1);
        if (verbose)
            cerr << "#\t enough ships (" << ships_avail << ") found, attack turn "
                 << min_attack_turn << ", from " << (idx_end+1) << " sources" << endl;

        assert(ships_avail >= 0);
        ships_t ships_needed = sim.planet_states(target._id)->at(min_attack_turn)._ships + 1;

        ships_t ships_sent = 0;
        double error = 0.0;
        Move *move = new Move();
        for (uint16_t idx=0; idx<=idx_end; idx++) {
            plid_t src_id = sources[idx].first;
            turn_t eta = sources[idx].second;
            if (eta > min_attack_turn || eta > SRC_RADIUS) continue;

            turn_t dep_turn = min_attack_turn - eta;
            ships_t src_ships = sim.ships_avail(src_id, dep_turn, attacker);

            if (src_ships == 0) continue;
            double frac = (double)ships_needed * (double)src_ships / (double)ships_avail;
            ships_t to_send = (int)frac;
            error += frac - (double)to_send;
            if (error > 0.9999) {
                error -= 0.9999;
                to_send++;
            }
            assert(to_send <= src_ships);
            if (to_send == 0) continue; // don't bother to send

            FutureOrder order(attacker, to_send, src_id, target._id, dep_turn);
            move->add(order);
            ships_sent += to_send;

            if (ships_sent >= ships_needed)
                break;
        }
        assert(ships_sent == ships_needed);
        if (verbose)
            cerr << "# planet " << target._id << " attacked with " << ships_sent
                 << " ships from " << (idx_end+1) << " sources" << endl;
        return move;
    }
    return NULL;
}

Move * create_move(Simulator &sim,
                   const Move &existing,
                   const vector<target_t>::const_iterator &begin,
                   const vector<target_t>::const_iterator &end,
                   Race attacker) {
    bool changed = true;
    unsigned int expands = 0;

    Move *move = new Move(existing); // move attacking all possible targets from given range

    for (vector<target_t>::const_iterator i=begin; i<end; i++) {
        // re-simulate if new moves added
        if (changed) {
            // newly generated moves may discard existing ones
            sim.simulate_safe(*move, SIM_DEPTH);
            changed = false;
        }
        Move *m;
        switch (i->_kind) {
            case attack:
            case defence:
                m = exact_attack(sim, *i, attacker);
                break;
            case expand:
                m = (expands++ > MAX_EXPANDS) ? NULL : waiting_attack(sim, *i, attacker);
                break;
            default:
                m = NULL;
                assert(false);
                break;
        }
        if (m != NULL) {
            changed = true;
            move->add(m);
            delete m;
        } else {
            // attack can't be done, remove all future orders bound to target
            move->remove_targeted(i->_id, attacker);
            changed = true;
        }
    }
    return move;
}

Move * sneak_defence(Simulator &sim,
                     const Move &existing,
                     vector<target_t> &targets,
                     Race attacker) {
    vector<target_t>::iterator end = std::remove_if(targets.begin(), targets.end(), is_not_defence);
    return create_move(sim, existing, targets.begin(), end, attacker);
}

void report(vector<target_t> &targets, field_closeness &sorter) {
    for (vector<target_t>::const_iterator t=targets.begin(); t<targets.end(); t++)
        cerr << "score: " << sorter.score(t->_id) << ", " << *t;
}

void remove_all_of_kind(vector<target_t> &targets, Race owner) {
    targets.erase(std::remove_if(targets.begin(), targets.end(), target_kind_comp(owner)),
                  targets.end());
}

void shuffle_targets(const Simulator &sim,
                     const Move &existing,
                     vector<target_t> &targets,
                     vector<pair<Move, vector<target_t> > > &shuf_targets,
                     Race attacker) {
    field_closeness closeness_sorter(sim.game(), sim.pmap(), attacker);
    target_kind_comp kind_sorter(sim.game(), sim.pmap());

    // highest priority - attack/defence targets
    std::sort(targets.begin(), targets.end(), kind_sorter);
    vector<target_t>::iterator end = std::find_if(targets.begin(), targets.end(), is_expand);

    // add attack/defence targets only, sorted by closeness
    std::sort(targets.begin(), end, closeness_sorter);
    shuf_targets.push_back(make_pair<Move, vector<target_t> >
                               (existing, vector<target_t>(targets.begin(), end)));

    // "do nothing" target
    if (attacker == ally)
        shuf_targets.push_back(make_pair<Move, vector<target_t> >(existing, vector<target_t>()));

    // add neutral expands only, sorted by closeness
    vector<target_t> ex_neutral(end, targets.end());
    remove_all_of_kind(ex_neutral, ally);
    remove_all_of_kind(ex_neutral, enemy);
    std::sort(ex_neutral.begin(), ex_neutral.end(), closeness_sorter);
    ex_neutral.insert(ex_neutral.begin(), targets.begin(), end);
    shuf_targets.push_back(make_pair<Move, vector<target_t> >(existing, ex_neutral));

    // add enemy expands only, sorted by reverse closeness
    vector<target_t> ex_enemy(end, targets.end());
    remove_all_of_kind(ex_enemy, neutral);
    remove_all_of_kind(ex_enemy, attacker);
    std::sort(ex_enemy.begin(), ex_enemy.end(), closeness_sorter);
    std::reverse(ex_enemy.begin(), ex_enemy.end());
    ex_enemy.insert(ex_enemy.begin(), targets.begin(), end);
    shuf_targets.push_back(make_pair<Move, vector<target_t> >(existing, ex_enemy));
}

turn_t nearest_enemy(const Game &game, const PlanetMap &pmap, plid_t id) {
    Race enemy_race = opposite(game.owner(id));
    for (vector<pair<plid_t, turn_t> >::const_iterator i=pmap.neighbours(id).begin();
                                                       i<pmap.neighbours(id).end();
                                                       i++)
        if (game.owner(i->first) == enemy_race)
            return i->second;
    return -1;
}

void reinforce(Simulator &sim,
               vector<Move *> &moves,
               Race attacker) {
    vector<plid_t> allies;
    for (plid_t id=0; id<sim.game()->num_planets(); id++)
        if (sim.game()->owner(id) == attacker) allies.push_back(id);
    simple_closeness enemy_closeness(sim.game(), sim.pmap(), opposite(attacker));
    std::sort(allies.begin(), allies.end(), enemy_closeness);
    std::reverse(allies.begin(), allies.end());

    vector<Move *> reinforcing_moves;
    set<plid_t> visited;

    for (vector<Move*>::iterator m=moves.begin(); m<moves.end(); m++) {
        Move *reinforce_move = new Move(*m);
        sim.simulate_safe(**m, SIM_DEPTH);
        visited.clear();
        for (vector<plid_t>::iterator dst=allies.begin(); dst<allies.end(); dst++) {
            visited.insert(*dst);
            turn_t nearest = nearest_enemy(*sim.game(), *sim.pmap(), *dst);
            for (vector<plid_t>::iterator src=allies.begin(); src<allies.end(); src++)
                if (visited.find(*src) == visited.end()){
                    ships_t ships_avail = sim.ships_avail(*src, 0, attacker);
                    if (ships_avail > 0 &&
                        sim.pmap()->distance(*src, *dst) < nearest * REINF_THRESH) {
                        visited.insert(*src);
                        reinforce_move->add(FutureOrder(attacker, ships_avail, *src, *dst, 0));
                    }
                }
        }
        if (!reinforce_move->empty()) {
            reinforcing_moves.push_back(reinforce_move);
        }
    }
    moves.insert(moves.end(), reinforcing_moves.begin(), reinforcing_moves.end());
}

unsigned int total_orders(const vector<Move*> &moves) {
    unsigned int ret = 0;
    vector<Move*>::const_iterator j;
    for (j=moves.begin(); j<moves.end(); j++)
        ret += (*j)->size();
    return ret;
}

unsigned int unique_orders(const vector<Move*> &moves) {
    vector<Move*>::const_iterator j;
    vector<FutureOrder>::const_iterator k;

    set<FutureOrder, order_less> orders;
    for (j=moves.begin(); j<moves.end(); j++)
        for (k=(*j)->orders().begin(); k<(*j)->orders().end(); k++)
            orders.insert(*k);
    return orders.size();
}
