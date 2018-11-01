#include "Planner.h"
#include "../Profileur.h"

namespace goap {

std::vector<Planner::action_ptr> Planner::extract_actions_from_state(const PlanningState& state) const {
    std::vector<action_ptr> actions;

    // GoToGoal actions
    for (const goap::Goal goal : state.goals) {
        for (const goap::Npc npc : state.npcs) {
            actions.push_back(std::make_unique<GoToGoalAction>(npc.id, goal.position));
        }
    }

    // Press plate actions
    for (const goap::Plate& plate : state.plates) {
        for (const goap::Npc npc : state.npcs) {
            actions.push_back(std::make_unique<PressPlateAction>(npc.id, plate.id));
            actions.push_back(std::make_unique<GoToAction>(npc.id, plate.position));
        }
    }

    // Traverse door actions
    for (const goap::Door& door : state.doors) {
        for (const goap::Npc npc : state.npcs) {
            actions.push_back(std::make_unique<TraverseDoorAction>(npc.id, door.id));
            actions.push_back(std::make_unique<GoToAction>(npc.id, door.position_start));
            actions.push_back(std::make_unique<GoToAction>(npc.id, door.position_end));
        }
    }

    return actions;
}

Planner::Plan Planner::plan(const GameManager& current_game_state) {
    PROFILE_SCOPE("GOAP Planner");
    PlanningState initial_state(current_game_state);

    const auto actions_set = extract_actions_from_state(initial_state);
}

}