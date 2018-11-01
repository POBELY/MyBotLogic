#ifndef DEF_GOAP_PLANNER_H
#define DEF_GOAP_PLANNER_H

#include "action.h"
#include <vector>
#include <memory>

class GameManager;

namespace goap {

class Planner {
    using action_ptr = std::unique_ptr<Action>;

    std::vector<action_ptr> extract_actions_from_state(const PlanningState& state) const;
public:
    using Plan = void;
    Plan plan(const GameManager& current_game_state);
};

}

#endif