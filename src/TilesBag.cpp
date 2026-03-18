#include "TilesBag.hpp"
#include "Constants.hpp"
#include "GameTypes.hpp"
#include <cassert>

TilesBag::TilesBag() {
  // Put the right amount of normal tiles into the bag
  for (int val = 0; val < MAX_TILE_VALUE; ++val) {
    for (int col = 0; col < NUM_COLORS; ++col) {
      unknownTilesLeft[val][col] = INDIVIDUAL_TILE_FREQ;
    }
  }

  // Put jokers into the bag
  jokersLeft = INDIVIDUAL_TILE_FREQ;

  // Add 1 because joker is also contained
  totalTilesLeft = MAX_TILE_VALUE  * NUM_COLORS * INDIVIDUAL_TILE_FREQ + INDIVIDUAL_TILE_FREQ; // Normal + Jokers
}

bool TilesBag::is_empty() const { 
  assert(totalTilesLeft >= 0 && "TilesBag contained less than 0 tiles");
  return totalTilesLeft == 0;
}

int TilesBag::n_tiles_left_specific(const Tile& tile) const {
  // If tile is a joker we don't have to look up anything in the matrix
  if (tile.isJoker) {
    assert(jokersLeft >= 0 && "Found a negative amount of jokers.");
    return jokersLeft;
  }

  // Tile values are in {1, ..., MAX_TILE_VALUE} => shift index back by 1
  int valueIndex = tile.value - 1;

  // Convert color to int
  // First color mapped to 0 so no index shift needed
  int colorIndex = static_cast<int>(tile.color);
  assert(0 <= colorIndex && colorIndex < NUM_COLORS && "Color -> int conversion failed.");

  int nTilesLeft = unknownTilesLeft[valueIndex][colorIndex];

  assert(nTilesLeft >= 0 && "Found a negative amount of a certain tile.");

  return nTilesLeft;
}

int TilesBag::n_tiles_left_total() const { return totalTilesLeft; }

void TilesBag::remove_tile(const Tile& tile) {
  // If tile is a joker we don't have to look up anything in the matrix
  if (tile.isJoker) {
    --jokersLeft;
    --totalTilesLeft;
    assert(jokersLeft >= 0 && "Found a negative amount of jokers.");
    return;
  }

  // Tile values are in {1, ..., MAX_TILE_VALUE} => shift index back by 1
  int valueIndex = tile.value - 1;

  // Convert color to int
  // First color mapped to 0 so no index shift needed
  int colorIndex = static_cast<int>(tile.color);
  assert(0 <= colorIndex && colorIndex < NUM_COLORS && "Color -> int conversion failed.");

  int nTilesLeft = --unknownTilesLeft[valueIndex][colorIndex];

  assert(nTilesLeft >= 0 && "Tried to remove a tile that was not in the bag.");

  --totalTilesLeft;
}

void TilesBag::draw() {
  --totalTilesLeft;
  assert(totalTilesLeft >= 0 && "Total tiles number in bag can't be negative.");
}

