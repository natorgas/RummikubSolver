#include "GameTypes.hpp"
#include <iostream>
#include <cassert>
#include <string>

/********************************** Tile ************************************/

Tile::Tile(int v, Color c, bool joker) : value(v), color(c), isJoker(joker) {}

void Tile::print() const {
  if (isJoker) {
    std::cout << "[Joker]" << std::endl;
  }
  else {
    std::string col;

    switch (color) {
      case Color::Black:      col = "Black";  break;
      case Color::Red:        col = "Red";    break;
      case Color::Orange:     col = "Orange"; break;
      case Color::Blue:       col = "Blue";   break;
      default: assert(false && "Tile not joker and color not valid.");
    }

    std::cout << "[" << value << " " << col << "]" << std::endl;
  }
}






