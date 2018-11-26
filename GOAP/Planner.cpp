#include "Planner.h"
#include "../Profileur.h"

#include <algorithm>
#include <numeric>

namespace goap {

std::vector<Planner::ActionNode> Planner::extract_actions_from_state(const PlanningState& state) const {
    PROFILE_SCOPE("Planner::extract_actions_from_state");
    std::vector<ActionNode> actions;

    // GoToGoal actions
    for (const goap::Goal goal : state.goals) {
        for (const goap::Npc npc : state.npcs) {
            actions.emplace_back(std::make_unique<GoToGoalAction>(npc.id, goal.position));
        }
    }

    // Press plate actions
    for (const goap::Plate& plate : state.plates) {
        for (const goap::Npc npc : state.npcs) {
            actions.emplace_back(std::make_unique<PressPlateAction>(npc.id, plate.id, true));
            actions.emplace_back(std::make_unique<PressPlateAction>(npc.id, plate.id, false));
            actions.emplace_back(std::make_unique<GoToAction>(npc.id, plate.position));
        }
    }

    // Traverse door actions
    for (const goap::Door& door : state.doors) {
        for (const goap::Npc npc : state.npcs) {
            actions.emplace_back(std::make_unique<TraverseDoorAction>(npc.id, door.id));
            actions.emplace_back(std::make_unique<GoToAction>(npc.id, door.position_start));
            actions.emplace_back(std::make_unique<GoToAction>(npc.id, door.position_end));
        }
    }

    return actions;
}

Planner::Plan Planner::plan(const GameManager& current_game_state) {
    PROFILE_SCOPE("Planner::plan");
    PlanningState initial_state(current_game_state);

    auto actions_set = extract_actions_from_state(initial_state);

    struct PlanningStateNode {
        PlanningState state;
        std::vector<ActionNode*> actions_attempts;

        PlanningStateNode(PlanningState state)
        : state{ state } {

        }
    };

    // Calculates every possible plans
    std::vector<PlanningStateNode> planning_state_stack;
    std::vector<ActionNode*> actions_stack;
    std::vector<int> plan_score;
    std::vector<std::vector<ActionNode*>> valid_plans;
    actions_stack.reserve(actions_set.size());
    planning_state_stack.push_back(initial_state);

    int plan_score_acc = 0;
    // Here we search for the first plan (should search for best plan)
    while (!planning_state_stack.empty()) {
        PlanningStateNode& current_state = planning_state_stack.back();

        if (current_state.state.goal_achieved()) {
            // Save the plan
            valid_plans.push_back(std::move(actions_stack));
            plan_score.push_back(plan_score_acc);

            // Reset all actions
            for (ActionNode& action : actions_set) {
                action.used = false;
            }

            while (planning_state_stack.size() > 1) {
                planning_state_stack.pop_back();
            }

            plan_score_acc = 0;
            continue;
        }

        bool found_compatible_action = false;
        for (ActionNode& action : actions_set) {
            if (!action.used 
             && std::find(current_state.actions_attempts.begin(), current_state.actions_attempts.end(), &action) == current_state.actions_attempts.end() 
             && action.action->is_action_possible(current_game_state, current_state.state)) {
                found_compatible_action = true;
                
                action.action->update_cost(current_game_state, current_state.state);
                plan_score_acc += action.action->cost();

                action.used = true;
                current_state.actions_attempts.push_back(&action); // Add this attempt
                actions_stack.push_back(&action);
                planning_state_stack.push_back(action.action->apply(current_state.state));
                break;
            }
        }
        if (!found_compatible_action) {
            planning_state_stack.pop_back();

            if(!actions_stack.empty()) {
                actions_stack.back()->used = false;
                plan_score_acc -= actions_stack.back()->action->cost();
                actions_stack.pop_back();
            }
        }
    }

    // Find best plan
    auto best_plan_it = std::min_element(plan_score.begin(), plan_score.end());
    const std::size_t best_plan_index = std::distance(plan_score.begin(), best_plan_it);

    std::vector<ActionNode*> best_plan = valid_plans[best_plan_index];
    Plan best_plan_to_return;
    best_plan_to_return.reserve(best_plan.size());
    std::transform(best_plan.begin(), best_plan.end(), std::back_inserter(best_plan_to_return), [](ActionNode* action) {
        return std::move(action->action);
    });

    return best_plan_to_return;
}

}