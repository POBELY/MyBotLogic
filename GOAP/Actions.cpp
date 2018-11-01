#include "action.h"
#include "../GameManager.h"

#include <algorithm>

namespace goap {

bool GoToAction::is_action_possible(const GameManager& gm, const PlanningState& state) const {
    auto npc_it = std::find_if(state.npcs.begin(), state.npcs.end(), [this](const goap::Npc& npc) {
        return npc.id == this->npc();
    });

    // To move, the npc must not be busy
    return !npc_it->busy && gm.m.aStar(npc_it->position, target_pos_).isAccessible();
}

PlanningState GoToAction::apply(const PlanningState& state) const {
    PlanningState new_state = state;
    auto npc_it = std::find_if(new_state.npcs.begin(), new_state.npcs.end(), [this](const goap::Npc& npc) {
        return npc.id == this->npc();
    });

    npc_it->position = target_pos_;

    return new_state;
}

void GoToAction::update_cost(const GameManager& gm, const PlanningState& state) {
    auto npc_it = std::find_if(state.npcs.begin(), state.npcs.end(), [this](const goap::Npc& npc) {
        return npc.id == this->npc();
    });

    set_cost(gm.m.getDistance(npc_it->position, target()) * 3);
}

void GoToGoalAction::update_cost(const GameManager& gm, const PlanningState& state) {
    auto npc_it = std::find_if(state.npcs.begin(), state.npcs.end(), [this](const goap::Npc& npc) {
        return npc.id == this->npc();
    });

    set_cost(gm.m.getDistance(npc_it->position, target()));
}

bool TraverseDoorAction::is_action_possible(const GameManager&, const PlanningState& state) const {
    auto npc_it = std::find_if(state.npcs.begin(), state.npcs.end(), [this](const goap::Npc& npc) {
        return npc.id == this->npc();
    });

    auto door_it = std::find_if(state.doors.begin(), state.doors.end(), [this](const goap::Door& door) {
        return door.id == this->door();
    });

    return !npc_it->busy
        && (npc_it->position == door_it->position_start 
         || npc_it->position == door_it->position_end)
        && door_it->state == DoorState::Open;
}

PlanningState TraverseDoorAction::apply(const PlanningState& state) const {
    PlanningState new_state = state;

    auto npc_it = std::find_if(new_state.npcs.begin(), new_state.npcs.end(), [this](const goap::Npc& npc) {
        return npc.id == this->npc();
    });

    auto door_it = std::find_if(new_state.doors.begin(), new_state.doors.end(), [this](const goap::Door& door) {
        return door.id == this->door();
    });

    if (npc_it->position == door_it->position_start) {
        npc_it->position = door_it->position_end;
    }
    else {
        npc_it->position = door_it->position_start;
    }

    return new_state;
}

void TraverseDoorAction::update_cost(const GameManager& gm, const PlanningState& state) {
    set_cost(3);
}

bool PressPlateAction::is_action_possible(const GameManager&, const PlanningState& state) const {
    auto npc_it = std::find_if(state.npcs.begin(), state.npcs.end(), [this](const goap::Npc& npc) {
        return npc.id == this->npc();
    });

    auto plate_it = std::find_if(state.plates.begin(), state.plates.end(), [this](const goap::Plate& plate) {
        return plate.id == this->plate();
    });

    return !npc_it->busy                                                              // Le npc n'est pas occupé
         && plate_it->state == (pressed ? PlateState::Released : PlateState::Pressed) // L'état de la plate n'est pas contradictoire à l'action
         && npc_it->position == plate_it->position;                                   // Le npc se trouve sur la tuile
}

PlanningState PressPlateAction::apply(const PlanningState& state) const {
    PlanningState new_state = state;

    auto npc_it = std::find_if(new_state.npcs.begin(), new_state.npcs.end(), [this](const goap::Npc& npc) {
        return npc.id == this->npc();
    });

    auto plate_it = std::find_if(new_state.plates.begin(), new_state.plates.end(), [this](const Plate& plate) {
        return plate.id == this->plate();
    });

    std::vector<Door*> doors_to_open;
    doors_to_open.reserve(plate_it->associated_objects.size());
    for (unsigned int associated_object_id : plate_it->associated_objects) {
        auto door_it = std::find_if(new_state.doors.begin(), new_state.doors.end(), [associated_object_id](const Door& door) {
            return door.id == associated_object_id;
        });

        if (door_it != new_state.doors.end()) {
            doors_to_open.push_back(&(*door_it));
        }
    }

    // Pressing a plate, open all it's associated doors
    for (Door* door : doors_to_open) {
        door->state = pressed ? DoorState::Open : DoorState::Closed;
    }

    // Le npc est maintenant occupé
    npc_it->busy = pressed;
    plate_it->state = (pressed ? PlateState::Pressed : PlateState::Released);

    return new_state;
}

void PressPlateAction::update_cost(const GameManager& gm, const PlanningState& state) {
    set_cost(3);
}

}