#ifndef UTILS_H
#define UTILS_H

#include "GameTypes.hpp"
#include "Constants.hpp"
#include "Player.hpp"
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>
#include <set>
#include <cassert>
#include <map>
#include <memory>

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

    for (int i = 0; i < jokersUsed; ++i) {
      assert(madeFristMove);
      std::cout << "Enter the tile you want to replace with a joker in the format 'value color'.\n";

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

inline std::vector<Set> generate_groups(const std::vector<Tile>& tilesOfValue, const int nJokers) {
  std::vector<Set> allGroups;
  if (tilesOfValue.empty()) return allGroups;

  // All tiles in this vector share the same number.
  int numberValue = tilesOfValue[0].value;

  // Using a map ensures we automatically drop duplicate colors.
  std::map<Color, Tile> colorToTile; 
  for (const auto& tile : tilesOfValue) {
    if (!tile.isJoker) {
      colorToTile.insert_or_assign(tile.color, tile);
    }
  }

  // Perfect group templates using your Color enum directly.
  std::vector<std::vector<Color>> perfectGroups = {
    {Color::Red, Color::Blue, Color::Black, Color::Orange}, // Group of 4
    {Color::Red, Color::Blue, Color::Black},                // Group of 3 (Missing ORANGE)
    {Color::Red, Color::Blue, Color::Orange},               // Group of 3 (Missing BLACK)
    {Color::Red, Color::Black, Color::Orange},              // Group of 3 (Missing BLUE)
    {Color::Red, Color::Black, Color::Orange}               // Group of 3 (Missing RED)
  };

  // Try to build each perfect group
  for (const auto& groupTemplate : perfectGroups) {
    std::vector<Tile> currentGroup;
    int jokersNeeded = 0;

    // Iterate directly through the requested Colors
    for (Color colorNeeded : groupTemplate) {

      if (colorToTile.count(colorNeeded)) {
        // We have the physical tile, add it using .at() to avoid default constructor errors
        currentGroup.push_back(colorToTile.at(colorNeeded));
      } else {
        // Else use a joker
        jokersNeeded++;

        currentGroup.emplace_back(Tile(numberValue, colorNeeded, true));
      }
    }

    // If we didn't use too many jokers, add this group to allGroups
    assert(nJokers <= INDIVIDUAL_TILE_FREQ);
    if (jokersNeeded <= nJokers && jokersNeeded < groupTemplate.size()) {
      allGroups.push_back(Set(SetType::Group, currentGroup));
    }
  }

  return allGroups;
}

inline std::vector<Set> generate_runs(const std::vector<Tile>& tilesOfColor, const int nJokers) {
  std::vector<Set> allRuns;
  if (tilesOfColor.empty()) return allRuns;

  Color runColor = tilesOfColor[0].color;

  std::map<int, Tile> valueToTile; 
  for (const auto& tile : tilesOfColor) {
    if (tile.value >= MIN_TILE_VALUE && tile.value <= MAX_TILE_VALUE) {
      valueToTile.insert_or_assign(tile.value, tile); 
    }
  }

  // Top-down generation with early pruning
  for (int start = MIN_TILE_VALUE; start <= MAX_TILE_VALUE - MIN_SET_SIZE + 1; ++start) {
    std::vector<Tile> currentRun;
    int jokersNeeded = 0;

    // Pre-fill the first two tiles of the sequence (a run needs at least 3)
    for (int v = start; v < start + 2; ++v) {
      if (valueToTile.count(v)) {
        currentRun.push_back(valueToTile.at(v));
      } else {
        jokersNeeded++;
        currentRun.push_back(Tile(v, runColor, true));
      }
    }

    // Now expand the run one tile at a time
    for (int end = start + 2; end <= 13; ++end) {

      // Add the next tile to our rolling sequence
      if (valueToTile.count(end)) {
        currentRun.push_back(valueToTile.at(end));
      } else {
        jokersNeeded++;
        currentRun.push_back(Tile(end, runColor, true));
      }

      // Any longer sequence starting at this 'start' will also be invalid.
      if (jokersNeeded > nJokers) {
        break; 
      }

      // 4. If we made it here, it's a valid Rummikub run. Save a copy!
      if (jokersNeeded < currentRun.size()) {
        allRuns.push_back(Set(SetType::Run, currentRun));
      }
    }
  }
  return allRuns;
}

inline std::vector<Set> generate_all_sets(const std::vector<Tile>& pool) {
  std::vector<Set> allSets;
  allSets.reserve(pool.size());

  // Create all groups
  for (int val = MIN_TILE_VALUE; val <= MAX_TILE_VALUE; ++val) {
    int nJokers = 0;
    std::vector<Tile> tilesOfValue;
    tilesOfValue.reserve(NUM_COLORS * INDIVIDUAL_TILE_FREQ);

    for (const Tile& t : pool) {
      if (t.isJoker) {
        nJokers++;
        continue;
      }
      if (t.value == val) tilesOfValue.push_back(t);
    }

    std::vector<Set> allGroups = generate_groups(tilesOfValue, nJokers);
    allSets.insert(allSets.end(), allGroups.begin(), allGroups.end());
  }

  // Create all runs
  for (Color col : ALL_COLORS) {
    int nJokers = 0;
    std::vector<Tile> tilesOfColor;
    tilesOfColor.reserve(INDIVIDUAL_TILE_FREQ * MAX_TILE_VALUE);

    for (const Tile& t : pool) {
      if (t.isJoker) {
        nJokers++;
        continue;
      }
      if (t.color == col) tilesOfColor.push_back(t);
    }

    std::vector<Set> allRuns = generate_runs(tilesOfColor, nJokers);
    allSets.insert(allSets.end(), allRuns.begin(), allRuns.end());
  }

  return allSets;
}

inline bool set_can_be_placed(const Set& set, const std::map<Tile, int>& availableTiles) {

  std::map<Tile, int> tilesNeededForThisSet;

  for (Tile t : set.tiles) {

    // 3. Normalize the joker so it matches what is actually in availableTiles
    if (t.isJoker) {
      t.value = 0;           
      t.color = Color::None; 
    }

    tilesNeededForThisSet[t]++;

    int availableAmount = 0;
    if (availableTiles.count(t)) {
      availableAmount = availableTiles.at(t);
    }

    if (tilesNeededForThisSet[t] > availableAmount) {
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

inline bool have_winner(const std::vector<std::unique_ptr<Player>>& players) {
  for (auto& player_p : players) {
    if (player_p->placed_all_tiles()) return true;
  }
  return false;
}

inline int val_sum_of_placed_tiles(const Board& oldBoard, const Board& newBoard) {
  int valSum = 0;
  for (const Tile& t : newBoard.tiles_on_board()) {
    valSum += t.value;
  }
  for (const Tile& t : oldBoard.tiles_on_board()) {
    valSum -= t.value;
  }
  return valSum;
}

inline void normalize_jokers(std::vector<Tile>& tiles) {
  for (Tile& t : tiles) {
    if (t.isJoker) {
      t.color = Color::None;
      t.value = 0;
    }
  }
}

#endif







