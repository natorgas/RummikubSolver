#ifndef UTILS_H
#define UTILS_H

#include "GameTypes.hpp"
#include "Constants.hpp"
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>

inline void to_lower(std::string& str) {
  std::transform(str.begin(), str.end(), str.begin(),
      [](unsigned char c) { return std::tolower(c); });
}

inline Color str_to_color(std::string& str) {
  to_lower(str);
  if (str == "black")       return Color::Black;
  else if (str == "blue")   return Color::Blue;
  else if (str == "red")    return Color::Red;
  else if (str == "orange") return Color::Orange;
  else {
    std::cout << str << " cannot be converted to type Color. Returned Color::None.\n";
    return Color::None;
  }
}

inline std::string color_to_str(Color col) {
  switch (col) {
    case Color::Black:   return "Black"; 
    case Color::Red:     return "Red";    
    case Color::Orange:  return "Orange";
    case Color::Blue:    return "Blue";   
    default: 
                         std::cout << "There was a color->str conversion error!\n";
                         return "ConversionError";
  }
}

inline bool joker_check(std::vector<Tile>& vec) {
  std::cout << "How many jokers did you use to create this new Set? ";
  int jokersUsed;
  std::cin >> jokersUsed;

  if (jokersUsed < 0 || jokersUsed > INDIVIDUAL_TILE_FREQ) {
    std::cout << "This is an impossible amount of jokers to use. Try again.\n";
    return false;
  }

  for (int i = 0; i < jokersUsed; ++i) {
    std::cout << "Enter the tile you want to replace in the format 'value color'.\n";

    int val;
    std::string c;
    std::cin >> val >> c;

    Color col = str_to_color(c);

    // Search for the tile in the vector
    auto it = std::find(vec.begin(), vec.end(), Tile(val, col));
    // If not found, tell user to try again
    if (it == vec.end()) {
      std::cout << "This tile is not needed for what you want to create. Try again.\n";
      i--;
      continue; 
    }
    // If found, turn into joker
    it->isJoker = true;
  }
  return true;
}

#endif







