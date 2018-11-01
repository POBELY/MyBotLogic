#ifndef DEF_GOAP_PLANNER_H
#define DEF_GOAP_PLANNER_H

#include "action.h"
#include <vector>
#include <memory>

class GameManager;

namespace goap {

class Planner {
    using action_ptr = std::unique_ptr<Action>;

    struct ActionNode {
        action_ptr action = {};
        bool used = false;

        ActionNode() = default;
        ActionNode(action_ptr action, bool used = false) 
        : action{ std::move(action) }, used{ used } {

        }
    };

    std::vector<ActionNode> extract_actions_from_state(const PlanningState& state) const;
public:
    using Plan = void;
    Plan plan(const GameManager& current_game_state);
};

}

#endif