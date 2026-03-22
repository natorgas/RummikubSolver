#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include <vector>

enum class Color { Black, Blue, Red, Orange, None };

enum class SetType { Run, Group };

enum class Move { CreateGroup, CreateRun, RemoveGroup, RemoveRun };


/**************************** Tile **************************/

struct Tile {
  Tile() = delete;

  Tile(int v, Color c, bool joker = false);

  void print() const;

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

  void print() const;

  int size() const;

  // Returns true if *this is valid
  bool valid() const;

  SetType type;
  std::vector<Tile> tiles;
};

/**********************************************************/


/**************************** Board ***********************/

class Board {
  public:
    Board();

    void print() const;

    // True <=> adding set to board was successful
    bool add_set(const Set& set);

    std::vector<Tile> tiles_on_board() const;

    int size() const;

    std::vector<Set> runs;
    std::vector<Set> groups;
};

/*********************************************************/

#endif




