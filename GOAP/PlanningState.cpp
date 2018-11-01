#include "PlanningState.h"

#include "../GameManager.h"
#include <algorithm>
#include <iterator>

namespace goap {

PlanningState::PlanningState(const GameManager& game) {
    const Map& map = game.m;
    plates.reserve(map.getActivateurs().size());
    doors.reserve(map.getPortes().size());
    goals.reserve(map.getObjectifs().size());
    npcs.reserve(game.getNpcs().size());

    // Load plates from map
    std::transform(std::begin(map.getActivateurs()), std::end(map.getActivateurs()), std::back_inserter(plates), [](const std::pair<const unsigned int, ObjectInfo>& pair) {
        Plate plate{pair.second.objectID, pair.second.tileID};
        plate.associated_objects.resize(pair.second.connectedTo.size());
        std::copy(std::begin(pair.second.connectedTo), std::end(pair.second.connectedTo), 
                  std::begin(plate.associated_objects));

        return plate;
    });

    // Load doors from map
    std::transform(std::begin(map.getPortes()), std::end(map.getPortes()), std::back_inserter(doors), [&map](const std::pair<const unsigned int, ObjectInfo>& pair) {
        const DoorState state = pair.second.objectStates.find(Object::ObjectState_Opened) != std::end(pair.second.objectStates) ? DoorState::Open : DoorState::Closed;
        
        const unsigned int door_start = pair.second.tileID;
        const unsigned int door_end = map.getAdjacentTileAt(door_start, pair.second.position);

        Door door{pair.second.objectID, door_start, door_end, state};
        door.activating_objects.resize(pair.second.connectedTo.size());
        std::copy(std::begin(pair.second.connectedTo), std::end(pair.second.connectedTo), 
                  std::begin(door.activating_objects));

        return door;
    });

    // Load goals from map
    std::transform(std::begin(map.getObjectifs()), std::end(map.getObjectifs()), std::back_inserter(goals), [](const unsigned int goal) {
        return Goal(goal);
    });

    // Load NPCs
    std::transform(std::begin(game.getNpcs()), std::end(game.getNpcs()), std::back_inserter(npcs), [](const ::Npc& npc) {
        return goap::Npc(npc.getId(), npc.getTileId());
    });
}

}