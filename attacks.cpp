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
                       const Simulator &sim,
                       bool safe) {
    ships_t ships_avail = 0;
    const vector<pair<plid_t, turn_t> > neighbors = sim.pmap()->neighbours(target._id);
    vector<pair<plid_t, turn_t> >::const_iterator i;
    for (i=neighbors.begin(); i<neighbors.end(); i++) {
        plid_t src_id = i->first;
        turn_t eta = i->second;
        if (eta <= target._turn) {
            ships_t ships = safe ? sim.ships_avail_safe(src_id, target._turn - eta, attacker) :
                                   sim.ships_avail(src_id, target._turn - eta, attacker);
            if (ships != 0) sources.push_back(make_pair<plid_t, ships_t>(src_id, ships));
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
                    bool safe) {
    vector<pair<plid_t, ships_t> > sources;
    sources.reserve(4);
    ships_t ships_avail = select_planets(sources, attacker, target, sim, safe);
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
    return move;
}

ships_t calc_ships_needed(const Simulator &sim,
                          const target_t &target,
                          const vector<pair<plid_t, turn_t> > &sources,
                          turn_t ally_eta,
                          turn_t attack_turn,
                          Race defender) {
    ships_t ret = sim.ships_avail(target._id, attack_turn, defender);
    vector<pair<plid_t, turn_t> >::const_iterator src;
    for (src=sources.begin(); src<sources.end(); src++) {
        turn_t eta = src->second;
        if (eta >= ally_eta) continue;
        // attacker's fleet arrives at attack_turn, so we have eta turns for accumulating ships
        turn_t dep_turn = attack_turn - eta + (target._owner == neutral ? 1 : 0);
        ships_t ships_avail = sim.ships_avail(src->first, dep_turn, defender);
        if (ships_avail != 0) ret += ships_avail;
    }
    return ret + 1;
}

turn_t is_enough_ships(const vector<pair<plid_t, turn_t> > &sources, int16_t end_src,
                       const Simulator &sim,
                       const target_t &target,
                       Race attacker,
                       ships_t &ships_avail,
                       ships_t &ships_needed,
                       bool safe = true) {
    assert(!sources.empty());
    for (turn_t att_turn=sources[0].second; att_turn<sim.turns(); att_turn++) {
        plstate_t target_state = sim.planet_states(target._id)->at(att_turn);
        if (target_state._owner == attacker) continue;

        turn_t  nearest_ally = -1;
        ships_avail = 0;
        ships_needed = target_state._ships + 1;
        for (int16_t idx=0; idx<=end_src; idx++) {
            ships_t ships;
            plid_t src_id = sources[idx].first;
            turn_t eta = sources[idx].second;
            if (eta > att_turn || eta > SRC_RADIUS) continue;

            turn_t dep_turn = att_turn - eta;
            if (safe) ships = sim.ships_avail_safe(src_id, dep_turn, attacker);
            else ships = sim.ships_avail(src_id, dep_turn, attacker);
            if (ships != 0) {
                if (safe && eta < nearest_ally) {
                    nearest_ally = eta;
                    ships_needed = max((ships_t)(target_state._ships + 1),
                                       calc_ships_needed(sim, target, sources,
                                                         nearest_ally, att_turn, opposite(attacker)));
                }
                ships_avail += ships;
                if (ships_avail >= ships_needed)
                    return att_turn;
            }
        }
    }
    return -1;
}

Move * waiting_attack(const Simulator &sim,
                      const target_t &target,
                      Race attacker,
                      bool safe) {
    vector<pair<plid_t, turn_t> > sources = sim.pmap()->neighbours(target._id);
    turn_t min_attack_turn = -1;
    int16_t idx_end = -1;
    ships_t ships_avail = 0;
    ships_t ships_needed = -1;
    if (!sources.empty()) {
        int16_t idx = (int16_t)sources.size() - 1;
        turn_t enough_wait = is_enough_ships(sources, idx, sim, target, attacker,
                                             ships_avail, ships_needed, safe);
        if (enough_wait < min_attack_turn) {
            min_attack_turn = enough_wait;
            idx_end = idx;
        }
    }
    if (min_attack_turn != (turn_t)-1) {
        assert(idx_end != (turn_t)-1);
        assert(ships_avail >= 0);

        ships_t ships_sent = 0;
        double error = 0.0;
        Move *move = new Move();
        for (uint16_t idx=0; idx<=idx_end; idx++) {
            plid_t src_id = sources[idx].first;
            turn_t eta = sources[idx].second;
            if (eta > min_attack_turn || eta > SRC_RADIUS) continue;

            turn_t dep_turn = min_attack_turn - eta;
            ships_t src_ships;

            if (safe) src_ships = sim.ships_avail_safe(src_id, dep_turn, attacker);
            else src_ships = sim.ships_avail(src_id, dep_turn, attacker);

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
        return move;
    }
    return NULL;
}

Move * create_move(Simulator &sim,
                   const Move &existing,
                   const vector<target_t>::const_iterator &begin,
                   const vector<target_t>::const_iterator &end,
                   Race attacker,
                   bool safe) {
    bool changed = true;
    unsigned int expands = 0;

    Move *move = new Move(existing); // move attacking all possible targets from given range
    sim.clear_evacuation_marks();

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
                m = exact_attack(sim, *i, attacker);
                break;
            case defence:
                m = exact_attack(sim, *i, attacker, false);
                if (m == NULL) // defence is pointless, evacuate all ships if needed
                    sim.mark_for_evacuation(i->_id, attacker);
                break;
            case expand:
                m = (expands++ > MAX_EXPANDS) ? NULL : waiting_attack(sim, *i, attacker, safe && sim.winning(attacker));
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

void cutoff(vector<target_t> &targets, const simple_closeness &sorter) {
    if (targets.size() < 3) return;
    std::sort(targets.begin(), targets.end(), sorter);
    assert(sorter.score(targets.front()._id) <= sorter.score(targets.back()._id));
    double min = +1e+10;
    double max = -1e+10;
    vector<target_t>::iterator i;
    for (i=targets.begin(); i<targets.end(); i++) {
        double score = sorter.score(i->_id);
        if (score > max) max = score;
        if (score < min) min = score;
    }
    for (i=targets.begin(); i<targets.end(); i++) {
        double score = sorter.score(i->_id);
        if ((score - min) / (max - min) > EXP_CUTOFF)
            break;
    }
    targets.erase(i, targets.end());
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
    if (attacker == enemy && end - targets.begin() > 2) {
        for (uint16_t i=0; i<MAX_PERMS; i++) {
            shuf_targets.push_back(make_pair<Move, vector<target_t> >
                                   (existing, vector<target_t>(targets.begin(), end)));
            vector<target_t> *added = &(shuf_targets.end() - 1)->second;
            std::random_shuffle(added->begin(), added->end());
        }
    }

    // "do nothing" target
    if (attacker == ally)
        shuf_targets.push_back(make_pair<Move, vector<target_t> >(existing, vector<target_t>()));

    // prepare neutral expands
    vector<target_t> ex_neutral(end, targets.end());
    remove_all_of_kind(ex_neutral, ally);
    remove_all_of_kind(ex_neutral, enemy);

    // prepare enemy expands
    vector<target_t> ex_enemy(end, targets.end());
    remove_all_of_kind(ex_enemy, neutral);
    remove_all_of_kind(ex_enemy, attacker);

    if (attacker == enemy)
        for (uint16_t i=0; i<MAX_PERMS; i++) {
            std::random_shuffle(ex_neutral.begin(), ex_neutral.end());
            shuf_targets.push_back(make_pair<Move, vector<target_t> >(existing, ex_neutral));
            std::random_shuffle(ex_enemy.begin(), ex_enemy.end());
            shuf_targets.push_back(make_pair<Move, vector<target_t> >(existing, ex_enemy));
        }

    simple_closeness distance_sorter(sim.game(), sim.pmap(), attacker);
    // add neutral expands only, sorted by closeness
    cutoff(ex_neutral, distance_sorter);
    std::sort(ex_neutral.begin(), ex_neutral.end(), closeness_sorter);
    ex_neutral.insert(ex_neutral.begin(), targets.begin(), end);
    shuf_targets.push_back(make_pair<Move, vector<target_t> >(existing, ex_neutral));

    // add enemy expands only, sorted by reverse closeness
    cutoff(ex_enemy, distance_sorter);
    std::sort(ex_enemy.begin(), ex_enemy.end(), closeness_sorter);
    std::reverse(ex_enemy.begin(), ex_enemy.end());
    ex_enemy.insert(ex_enemy.begin(), targets.begin(), end);
    shuf_targets.push_back(make_pair<Move, vector<target_t> >(existing, ex_enemy));

    vector<target_t> *added;
    shuf_targets.push_back(make_pair<Move, vector<target_t> >(existing, ex_neutral));
    added = &(shuf_targets.end() - 1)->second;
    added->insert(added->begin(), ex_enemy.begin(), ex_enemy.end());

    shuf_targets.push_back(make_pair<Move, vector<target_t> >(existing, ex_enemy));
    added = &(shuf_targets.end() - 1)->second;
    added->insert(added->begin(), ex_neutral.begin(), ex_neutral.end());
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
    return (unsigned int)orders.size();
}
