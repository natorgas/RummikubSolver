#include "TileMap.hpp"
#include "Constants.hpp"
#include "GameTypes.hpp"
#include <cassert>

TileMap::TileMap() : map{} {}

int TileMap::index(const Tile& t) const {
  if (t.isJoker) return N_DIFF_TILES-1;

  assert(t.color != Color::None && "Non-Joker tile can't have Color == None");

  return (t.value - 1) * NUM_COLORS + static_cast<int>(t.color);
}

int& TileMap::operator[](const Tile& t) {
  return map[index(t)];
}

const int& TileMap::operator[](const Tile& t) const {
  return map[index(t)];
}
