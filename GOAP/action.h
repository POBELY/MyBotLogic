#ifndef DEF_GOAP_ACTIONS_H
#define DEF_GOAP_ACTIONS_H

#include "PlanningState.h"

namespace goap {

class Action {
    unsigned int npc_;
public:
    constexpr Action(unsigned npc) noexcept
    : npc_{npc} {}

    unsigned int npc() const noexcept { return npc_; }

    // Exposes where the npc will be after this action
    virtual unsigned int position_effect() const = 0;
    virtual bool win_effect() const = 0;

    virtual PlanningState apply(const PlanningState& state) const = 0;
};

// Requires the NPC to find a path to the destination
// Result the NPC is at specific location
class GoToAction : public Action {
    unsigned int target_pos_;
public:
    constexpr GoToAction(unsigned int npc, unsigned int target) noexcept
    : Action{npc}
    , target_pos_{target} {}

    unsigned int position_effect() const override { return target(); }
    bool win_effect() const override { return false; }

    unsigned int target() const noexcept { return target_pos_; }

    PlanningState apply(const PlanningState& state) const override;
};

class GoToGoalAction : public GoToAction {
public:
    constexpr GoToGoalAction(unsigned int npc, unsigned int goal_tile) noexcept
    : GoToAction(npc, goal_tile) {}

    bool win_effect() const override { return true; }
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

    unsigned int position_effect() const override;
    bool win_effect() const override { return false; }

    unsigned int door() const noexcept { return door_id_; }
    PlanningState apply(const PlanningState& state) const override;
};

// Requires the NPC to be on the plate
// Result opens associated doors
class PressPlateAction : public Action {
    unsigned int plate_id_;
public:
    constexpr PressPlateAction(unsigned int npc, unsigned int plate) noexcept
    : Action{npc}
    , plate_id_{plate} {}

    unsigned int plate() const noexcept { return plate_id_; }

    // The action did not move the npc
    unsigned int position_effect() const override;
    bool win_effect() const override { return false; }

    PlanningState apply(const PlanningState& state) const override;
};

}

#endif