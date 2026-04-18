#include "GameTypes.hpp"
#include "Constants.hpp"
#include "Utils.hpp"
#include <iostream>
#include <cassert>
#include <ostream>
#include <vector>
#include <set>

/********************************** Tile ************************************/

Tile::Tile(int v, Color c, bool joker) : value(v), color(c), isJoker(joker) {}

void Tile::print() const {
  if (isJoker) {
    std::cout << "[Joker]" << " ";
  }
  else {
    std::cout << "[" << value << " " << color_to_str(color) << "]" << " ";
  }
}

bool Tile::operator<(const Tile& other) const {
  if (isJoker != other.isJoker) {
    return isJoker < other.isJoker;
  }
  if (color != other.color) {
    return color < other.color;
  }
  return value < other.value;
}

bool Tile::operator==(const Tile& other) const {
  return (value == other.value) && (color == other.color) && (isJoker == other.isJoker);
}

/****************************************************************************/


/********************************** Set ************************************/

Set::Set(SetType tp, std::vector<Tile> t) : type(tp), tiles(t) {}

void Set::print() const {
  for (const Tile& tile : tiles) {
    tile.print();
  }
  std::cout << std::endl;
}

int Set::size() const {
  return tiles.size();
}

bool Set::valid() const {
  if (type == SetType::Group) {

    if (size() < MIN_SET_SIZE || size() > MAX_GROUP_SIZE) {
      std::cout << size() << " is not a valid group-size. Try again.\n";
      return false;
    }

    std::set<Color> seenColors = {tiles[0].color};

    // Tile values in a group must be constant and colors must be different
    for (int i = 1; i < tiles.size(); ++i) {
      seenColors.insert(tiles[i].color);
      if (tiles[i].value != tiles[i-1].value) {
        std::cout << "Values within a group must be constant. Try again.\n";
        return false;
      }
    }

    if (seenColors.size() != tiles.size()) return false;
  }

  else if (type == SetType::Run) {

    if (size() < MIN_SET_SIZE || size() > MAX_TILE_VALUE) {
      std::cout << size() << " is not a valid run-size. Try again.\n";
      return false;
    }

    Color runColor = tiles[0].color;

    // 1-step increasing tiles of same color
    for (int i = 1; i < tiles.size(); ++i) {
      if ((tiles[i].value - tiles[i-1].value != 1) ||
          (tiles[i].color != runColor)) {
        std::cout << "This was not a valid run. Try again.\n";
        return false;
      }
    }
  }

  return true;
}

/***************************************************************************/


/********************************* Board ***********************************/

Board::Board() : runs({}), groups({}) {}

void Board::print() const {

  // Print all groups
  std::cout << "##################"
            << " Groups "
            << "##################"
            << std::endl;

  for (int i = 0; i < groups.size(); ++i) {
    std::cout << i << ": ";
    groups[i].print();
  }

  std::cout << std::endl;

  // Print all Runs
  std::cout << "###################"
            << " Runs "
            << "###################"
            << std::endl;

  for (int i = 0; i < runs.size(); ++i) {
    std::cout << i << ": ";
    runs[i].print();
  }
}

bool Board::add_set(const Set& set) {
  // Only add set to board if it is valid
  if (!set.valid()) return false;
  if (set.type == SetType::Group) groups.push_back(set);
  else runs.push_back(set);
  return true;
}

std::vector<Tile> Board::tiles_on_board() const {
  std::vector<Tile> allTiles;
  allTiles.reserve(MAX_GROUP_SIZE * groups.size() + MAX_TILE_VALUE * runs.size());

  for (const Set& run : runs) {
    for (const Tile& tile : run.tiles) {
      allTiles.push_back(tile);
    }
  }

  for (const Set& group : groups) {
    for (const Tile& tile : group.tiles) {
      allTiles.push_back(tile);
    }
  }

  return allTiles;
}

int Board::size() const {
  int size = 0;
  for (const Set& g : groups) {
    size += g.size();
  }
  for (const Set& r : runs) {
    size += r.size();
  }
  return size;
}

/***************************************************************************/

