#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include <vector>

enum class Color { Black, Blue, Red, Orange, None };

enum class SetType { Run, Group };

/**************************** Tile **************************/

struct Tile {
  Tile() = delete;
  Tile(int v, Color c, bool joker = false);
  bool operator==(const Tile& other) const;
  bool operator<(const Tile& other) const;

  int value;
  Color color;
  bool isJoker;
};

/***********************************************************/


/**************************** Set **************************/

struct Set {
  Set() = default;
  Set(SetType tp, std::vector<Tile> t);
  int size() const;
  // Returns true if *this is a valid set according to Rummikub rules
  bool valid() const;

  SetType type;
  std::vector<Tile> tiles;
};

/**********************************************************/


/**************************** Board ***********************/

class Board {
  public:
    Board();
    // True <=> adding set to board was successful
    bool add_set(const Set& set);
    std::vector<Tile> tiles_on_board() const;
    int size() const;

    std::vector<Set> runs;
    std::vector<Set> groups;
};

/*********************************************************/

#endif
