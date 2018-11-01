#ifndef DEF_GOAP_PLANNING_STATE
#define DEF_GOAP_PLANNING_STATE

#include <limits>
#include <vector>

class GameManager;

namespace goap {

struct Npc {
    unsigned int id;
    unsigned int position;

    constexpr Npc(unsigned int id, unsigned int position) noexcept
    : id{ id }
    , position{ position } {
    }
};

struct Goal {
    unsigned int position;

    constexpr Goal(unsigned int position) noexcept
    : position{ position } {
    }
};

enum class DoorState {
    Open,
    Closed
};

static constexpr unsigned int INVALID_OBJECT_ID = std::numeric_limits<unsigned int>::max();

struct Plate {
    unsigned int id;
    std::vector<unsigned int> associated_objects;
    unsigned int position;
    
    Plate(unsigned int id, unsigned int position) noexcept
    : id{id}
    , position{position} {

    }
};

struct Door {
    unsigned int id;
    std::vector<unsigned int> activating_objects;
    unsigned int position_start, position_end;
    DoorState state;

    Door(unsigned int id, unsigned int position_start, unsigned int position_end, DoorState state) noexcept
    : id{ id }
    , position_start{ position_start }
    , position_end{ position_end }
    , state{ state } {

    }
};

struct PlanningState {
    std::vector<Npc> npcs;
    std::vector<Door> doors;
    std::vector<Plate> plates;
    std::vector<Goal> goals;

    PlanningState() = default;
    PlanningState(const GameManager& game);
};

}

#endif