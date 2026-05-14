#ifndef TILE_MAP_HH
#define TILE_MAP_HH

#include "Constants.hpp"
#include "GameTypes.hpp"
#include <array>
#include <vector>
#include <cassert>

// Custom map that maps a Tile to the generic type T

template <typename T>
class GenericTileMap {
  public:
    GenericTileMap() : map{} {}

    int index(const Tile& t) const {
      if (t.isJoker) return N_DIFF_TILES-1;
      if (t.color == Color::None) return N_DIFF_TILES-1;
      return (t.value - 1) * NUM_COLORS + static_cast<int>(t.color);
    }

    T& operator[](const Tile& t) {
      return map[index(t)];
    }

    const T& operator[](const Tile& t) const {
      return map[index(t)];
    }

    auto begin() { return map.begin(); }
    auto end()   { return map.end();   }

  private:
    std::array<T, N_DIFF_TILES> map;
};

using TileMap = GenericTileMap<int>;
using TileSetsMap = GenericTileMap<std::vector<int>>;

#endif
