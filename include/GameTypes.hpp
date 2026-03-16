#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include <vector>

enum class Color { Black, Blue, Red, Orange, None };

struct Tile {
  Tile() = delete;
  Tile(int v, Color c, bool joker = false);
  void print() const;

  int value;
  Color color;
  bool isJoker;
};

using Set = std::vector<Tile>;
using Board = std::vector<Set>;

#endif
