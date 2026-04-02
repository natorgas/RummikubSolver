#ifndef TILE_MAP_HH
#define TILE_MAP_HH

#include "Constants.hpp"
#include "GameTypes.hpp"
#include <array>

// map[-1] <-> [joker]
// map[(value-1)*NUM_COLORS + cast<int>(color)] <-> [value color]  

class TileMap {
  public:
    TileMap();

    int& operator[](const Tile& t);

    const int& operator[](const Tile& t) const;

  private:
    int index(const Tile& t) const;
    std::array<int, N_DIFF_TILES> map;
};

#endif
