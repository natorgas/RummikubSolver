#ifndef TILES_BAG_HPP
#define TILES_BAG_HPP

#include <array>
#include "Constants.hpp"
#include "GameTypes.hpp"

class TilesBag {
  public:
    TilesBag();

    bool is_empty() const;

    int n_tiles_left(const Tile& tile) const;

    void remove_tile(const Tile& tile);

  private:
    std::array<std::array<int, NUM_COLORS>, MAX_TILE_VALUE> tilesLeft;
    int jokersLeft;
    int totalTilesLeft;
};

#endif
