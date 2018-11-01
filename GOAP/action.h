#ifndef DEF_GOAP_ACTIONS_H
#define DEF_GOAP_ACTIONS_H

#include "PlanningState.h"

class GameManager;

namespace goap {

class Action {
    unsigned int npc_;
    int cost_;
public:
    constexpr Action(unsigned npc) noexcept
    : npc_{npc}
    , cost_{0} { }

    unsigned int npc() const noexcept { return npc_; }

    virtual bool is_action_possible(const GameManager& gm, const PlanningState& state) const = 0;
    virtual PlanningState apply(const PlanningState& state) const = 0;
    virtual void update_cost(const GameManager& gm, const PlanningState& state) = 0;

    int cost() const noexcept { return cost_; };
    void set_cost(int new_cost) { cost_ = new_cost; }
};

// Requires the NPC to find a path to the destination
// Result the NPC is at specific location
class GoToAction : public Action {
    unsigned int target_pos_;
public:
    constexpr GoToAction(unsigned int npc, unsigned int target) noexcept
    : Action{npc}
    , target_pos_{target} {}

    unsigned int target() const noexcept { return target_pos_; }

    bool is_action_possible(const GameManager& gm, const PlanningState& state) const override;
    PlanningState apply(const PlanningState& state) const override;
    void update_cost(const GameManager& gm, const PlanningState& state) override;
};

class GoToGoalAction : public GoToAction {
public:
    constexpr GoToGoalAction(unsigned int npc, unsigned int goal_tile) noexcept
    : GoToAction(npc, goal_tile) {}

    void update_cost(const GameManager& gm, const PlanningState& state) override;
};

// Requires the NPC to be next to the door
// Results the NPC traverse the door
class TraverseDoorAction : public Action {
    unsigned int door_id_;
public:
    constexpr TraverseDoorAction(unsigned int npc, unsigned int door) noexcept
    : Action{ npc }
    , door_id_{ door } {

    }

    unsigned int door() const noexcept { return door_id_; }

    bool is_action_possible(const GameManager& gm, const PlanningState& state) const override;
    PlanningState apply(const PlanningState& state) const override;
    void update_cost(const GameManager& gm, const PlanningState& state) override;
};

// Requires the NPC to be on the plate
// Result opens associated doors
class PressPlateAction : public Action {
    unsigned int plate_id_;
    bool pressed;
public:
    constexpr PressPlateAction(unsigned int npc, unsigned int plate, bool press) noexcept
    : Action{npc}
    , plate_id_{plate}
    , pressed{press} {
    }

    unsigned int plate() const noexcept { return plate_id_; }
    bool is_pressed() const noexcept { return pressed; }

    bool is_action_possible(const GameManager& gm, const PlanningState& state) const override;
    PlanningState apply(const PlanningState& state) const override;
    void update_cost(const GameManager& gm, const PlanningState& state) override;
};

}

#endif