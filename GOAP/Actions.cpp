#include "action.h"

#include <algorithm>

namespace goap {

PlanningState GoToAction::apply(const PlanningState& state) const {
    return state;
}

unsigned int TraverseDoorAction::position_effect() const {
    throw 5;
}

PlanningState TraverseDoorAction::apply(const PlanningState& state) const {
    // TODO: Move the npc at the other side of the door
    return state;
}

unsigned int PressPlateAction::position_effect() const {
    throw 5;
}

PlanningState PressPlateAction::apply(const PlanningState& state) const {
    PlanningState new_state = state;

    auto plate_it = std::find_if(state.plates.begin(), state.plates.end(), [this](const Plate& plate) {
        return plate.id == this->plate();
    });

    std::vector<Door*> doors_to_open;
    doors_to_open.reserve(plate_it->associated_objects.size());
    for (unsigned int associated_object_id : plate_it->associated_objects) {
        auto door_it = std::find_if(new_state.doors.begin(), new_state.doors.end(), [associated_object_id](const Door& door) {
            return door.id == associated_object_id;
        });

        if (door_it != state.doors.end()) {
            doors_to_open.push_back(&(*door_it));
        }
    }

    // Pressing a plate, open all it's associated doors
    for (Door* door : doors_to_open) {
        door->state = DoorState::Open;
    }

    return new_state;
}

}