#ifndef DEF_FLOOD_H
#define DEF_FLOOD_H

#include <vector>

class Map;

class Flood {
    std::vector<bool> tiles_;

    bool grow_from_neighbors(const Map& map);
    void try_to_add_neighbors_of(const Map& map, std::size_t tile_id, std::vector<std::size_t>& out_added) const;
public:
    Flood(const Map& map);
    Flood(const Map& map, int inital_tile);

    void reduce_to_accessibles(const Map& map);
    void grow(const Map& map);
    bool is_flooded(int tile) const noexcept;
    bool intersects(const Flood& other) const noexcept;
    std::vector<int> tiles() const noexcept;
    void reset(std::size_t tile_id);
};

#endif