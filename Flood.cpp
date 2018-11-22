#include "Flood.h"
#include "Map.h"

#include <cassert>

Flood::Flood(const Map& map)
: tiles_(map.getTailleTotal(), false) {

}

Flood::Flood(const Map& map, int inital_tile)
: Flood(map) {
    tiles_[inital_tile] = true;
}

void Flood::reduce_to_accessibles(const Map& map) {
    for (std::size_t i = 0; i < tiles_.size(); ++i) {
        // Les tuiles qui ne sont plus accessibles sont éteints
        if (tiles_[i] && !map.getTile(i).isAccessible()) {
            tiles_[i] = false;
        }
    }
}

void Flood::try_to_add_neighbors_of(const Map& map, std::size_t tile_id, std::vector<std::size_t>& out_added) const {
    // Récupère tous les voisins accessibles
    for (int voisin : map.getTile(tile_id).getVoisinsAccessibles()) {
        if(map.getTile(voisin).existe()) {
            if(!tiles_[voisin]) {
                out_added.push_back(voisin);
            }
        }
    }
}

bool Flood::grow_from_neighbors(const Map& map) {
    std::vector<std::size_t> to_adds;
    for (std::size_t i = 0; i < tiles_.size(); ++i) {
        //if(!tiles_[i]) {
        //    to_adds.push_back(i);
        //}
        if(tiles_[i]) {
            try_to_add_neighbors_of(map, i, to_adds);
        }
    }

    for(std::size_t to_add : to_adds) {
        tiles_[to_add] = true;
    }

    return !to_adds.empty();
}

void Flood::grow(const Map& map) {
    bool has_grown = false;
    do {
        has_grown = grow_from_neighbors(map);
    } while(has_grown);
}

bool Flood::is_flooded(int tile) const noexcept {
    return tiles_[tile];
}

bool Flood::intersects(const Flood& other) const noexcept {
    assert(tiles_.size() == other.tiles_.size());

    for (std::size_t i = 0; i < tiles_.size(); ++i) {
        if(tiles_[i] && other.tiles_[i]) return true;
    }

    return false;
}

std::vector<int> Flood::tiles() const noexcept {
    std::vector<int> all_tiles;
    all_tiles.reserve(tiles_.size());

    for (std::size_t i = 0; i < tiles_.size(); ++i) {
        if (tiles_[i]) {
            assert(i <= std::numeric_limits<int>::max());
            all_tiles.push_back(static_cast<int>(i));
        }
    }

    return all_tiles;
}

void Flood::reset(std::size_t tile_id) {
    for (std::size_t i = 0; i < tiles_.size(); ++i) {
        tiles_[i] = false;
    }
    tiles_[tile_id] = true;
}