#include "TilesBag.hpp"
#include "Constants.hpp"
#include <cassert>

TilesBag::TilesBag() {
  totalTilesLeft = MAX_TILE_VALUE  * NUM_COLORS * INDIVIDUAL_TILE_FREQ + INDIVIDUAL_TILE_FREQ; // Normal + Jokers
}

bool TilesBag::is_empty() const { 
  assert(totalTilesLeft >= 0 && "TilesBag contained less than 0 tiles");
  return totalTilesLeft == 0;
}

int TilesBag::n_tiles_left_total() const { return totalTilesLeft; }

void TilesBag::draw() {
  --totalTilesLeft;
  assert(totalTilesLeft >= 0 && "Total tiles number in bag can't be negative.");
}

