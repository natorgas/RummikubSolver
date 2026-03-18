#ifndef TILES_BAG_HPP
#define TILES_BAG_HPP

#include <array>
#include "Constants.hpp"
#include "GameTypes.hpp"

class TilesBag {
  public:
    TilesBag();

    bool is_empty() const;

    // This returning a nonzero value does not imply that tile is inside of TilesBag
    int n_tiles_left_specific(const Tile& tile) const;

    // Returns the actual number of tiles left inside the bag (info available to everyone in the game)
    int n_tiles_left_total() const;

    // Remove a specific tile (not useful when opponent draws)
    void remove_tile(const Tile& tile);

    // Decreases number of tiles inside the bag (called whenever someone draws from bag)
    void draw();

  private:
    std::array<std::array<int, NUM_COLORS>, MAX_TILE_VALUE> unknownTilesLeft;
    int jokersLeft;
    int totalTilesLeft;
};

#endif
