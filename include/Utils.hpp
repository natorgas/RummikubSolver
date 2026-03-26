#ifndef UTILS_H
#define UTILS_H

#include "GameTypes.hpp"
#include "Constants.hpp"
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>
#include <set>
#include <cassert>
#include <map>

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

inline bool joker_check(std::vector<Tile>& vec, bool madeFristMove) {
  while (true) {
    std::cout << "How many jokers did you use to create this new Set? ";
    int jokersUsed;
    std::cin >> jokersUsed;

    // Check for invalid input
    if (jokersUsed < 0 || jokersUsed > INDIVIDUAL_TILE_FREQ) {
      std::cout << "This is an impossible amount of jokers to use. Try again.\n";
      continue;
    }

    // Make sure no joker used on the first move
    if (jokersUsed > 0 && !madeFristMove) {
      std::cout << "You can't use a joker on your first move. Try again.\n";
      return false;
    }

    // Enforce that first move must be worth >= 30 points
    if (!madeFristMove) {
      int tileValueSum = 0;
      for (const Tile& t : vec) {
        tileValueSum += t.value;
      }
      if (tileValueSum < 30) {
        std::cout << "Your first move must be worth 30 points. Try again.\n";
        return false;
      }
    }

    for (int i = 0; i < jokersUsed; ++i) {
      assert(madeFristMove);
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
}

inline std::vector<Set> generate_groups(const std::vector<Tile>& tilesOfValue) {
  std::vector<Set> allGroups = {};

  std::set<Tile> tileSet(tilesOfValue.begin(), tilesOfValue.end());

  if (tileSet.size() < MIN_SET_SIZE) return allGroups; 

  else if (tileSet.size() == MIN_SET_SIZE) {
    Set groupOfThree(SetType::Group, std::vector<Tile>(tileSet.begin(), tileSet.end()));
    allGroups.push_back(groupOfThree);
  } 

  else {
    assert(tileSet.size() == MAX_GROUP_SIZE);
    std::vector<Tile> uniqueTiles(tileSet.begin(), tileSet.end());

    // Add the group of all 4 colors
    Set groupOfFour(SetType::Group, uniqueTiles);
    allGroups.push_back(groupOfFour);

    // Add the four different groups of 3 colors
    for (int i = 0; i < 4; ++i) {
      Set groupOfThree(SetType::Group, {});
      for (int j = 0; j < 4; ++j) {
        if (i != j) {
          groupOfThree.tiles.push_back(uniqueTiles[j]);
        }
      }
      allGroups.push_back(groupOfThree);
    }
  }
  return allGroups;
}

inline std::vector<Set> generate_runs(const std::vector<Tile>& tilesOfColor) {
  std::vector<Set> allRuns;
  if (tilesOfColor.size() < MIN_SET_SIZE) return allRuns;

  std::vector<Tile> uniqueTiles = tilesOfColor;

  // Sort by value ascending
  std::sort(uniqueTiles.begin(), uniqueTiles.end(),
            [](const Tile& a, const Tile& b) { return a.value < b.value; });

  // Remove duplicates
  auto last = std::unique(uniqueTiles.begin(), uniqueTiles.end(),
                          [](const Tile& a, const Tile& b) { return a.value == b.value; });
  uniqueTiles.erase(last, uniqueTiles.end());

  int n = uniqueTiles.size();

  // Generate all valid sub-sequences
  for (int i = 0; i < n; ++i) {
    std::vector<Tile> currentRun;
    
    // Start sequence at tile i
    currentRun.push_back(uniqueTiles[i]);

    for (int j = i + 1; j < n; ++j) {
      // If consecutive, add to the current run
      if (uniqueTiles[j].value == (uniqueTiles[j - 1].value + 1)) {
        currentRun.push_back(uniqueTiles[j]);

        // Once it hits size 3 or more, it's a valid Rummikub run. Save it!
        if (currentRun.size() >= 3) {
          allRuns.push_back(Set(SetType::Run, currentRun));
        }
      } 
      // Sequence broken, stop looking ahead
      else break;
    }
  }

  return allRuns;
}

inline std::vector<Set> generate_all_sets(const std::vector<Tile>& pool) {

  std::vector<Set> allSets;
  allSets.reserve(pool.size());

  // Create all groups
  for (int val = MIN_TILE_VALUE; val <= MAX_TILE_VALUE; ++val) {
    std::vector<Tile> tilesOfValue;
    tilesOfValue.reserve(NUM_COLORS * INDIVIDUAL_TILE_FREQ);

    for (const Tile& t : pool) {
      if (t.value == val) tilesOfValue.push_back(t);
    }

    std::vector<Set> allGroups = generate_groups(tilesOfValue);
    allSets.insert(allSets.end(), allGroups.begin(), allGroups.end());
  }

  // Create all runs
  for (Color col : ALL_COLORS) {
    std::vector<Tile> tilesOfColor;
    tilesOfColor.reserve(INDIVIDUAL_TILE_FREQ * MAX_TILE_VALUE);

    for (const Tile& t : pool) {
      if (t.color == col) tilesOfColor.push_back(t);
    }

    std::vector<Set> allRuns = generate_runs(tilesOfColor);
    allSets.insert(allSets.end(), allRuns.begin(), allRuns.end());
  }

  return allSets;
}

inline bool set_can_be_placed(const Set& set, std::map<Tile, int>& availableTiles) {
  for (const Tile& t : set.tiles) {
    if (availableTiles[t] == 0) {
      return false;
    }
  }
  return true;
}

inline bool all_original_tiles_placed(std::map<Tile, int>& boardTiles) {
  for (const auto& [tile, freq] : boardTiles) {
    if (freq != 0) return false;
  }
  return true;
}

#endif







