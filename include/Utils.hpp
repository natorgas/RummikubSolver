#ifndef UTILS_H
#define UTILS_H

#include "GameTypes.hpp"
#include "Constants.hpp"
#include "Player.hpp"
#include <array>
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cassert>
#include <map>
#include <memory>
#include <optional>

inline void to_lower(std::string& str) {
  std::transform(str.begin(), str.end(), str.begin(),
      [](unsigned char c) { return std::tolower(c); });
}

inline Color str_to_color(std::string& str) {
  while (true) {
    to_lower(str);
    if (str == "black")       return Color::Black;
    else if (str == "blue")   return Color::Blue;
    else if (str == "red")    return Color::Red;
    else if (str == "orange") return Color::Orange;
    else {
      std::cout << str << " cannot be converted to type Color. Enter color again: ";
      std::cin >> str;
    }
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

    for (int i = 0; i < jokersUsed; ++i) {
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
  if (tilesOfValue.empty() && nJokers < MIN_SET_SIZE) return allGroups;

  // All tiles in this vector share the same number.
  int numberValue = tilesOfValue.empty() ? 0 : tilesOfValue[0].value;

  // Using a map ensures we automatically drop duplicate colors.
  std::map<Color, Tile> colorToTile; 
  for (const auto& tile : tilesOfValue) {
    if (!tile.isJoker) {
      colorToTile.insert_or_assign(tile.color, tile);
    }
  }

  std::vector<std::vector<Color>> perfectGroups = {
    {Color::Red, Color::Blue, Color::Black, Color::Orange}, // Group of 4
    {Color::Red, Color::Blue, Color::Black},                // Group of 3 (Missing ORANGE)
    {Color::Red, Color::Blue, Color::Orange},               // Group of 3 (Missing BLACK)
    {Color::Red, Color::Black, Color::Orange},              // Group of 3 (Missing BLUE)
    {Color::Blue, Color::Black, Color::Orange}              // Group of 3 (Missing RED)
  };

  // Try to build each perfect group
  for (const auto& groupTemplate : perfectGroups) {
    int templateSize = groupTemplate.size();
    
    // Use a bitmask to try all combinations of Real vs Joker for each color in the template
    for (int mask = 0; mask < (1 << templateSize); ++mask) {
      std::vector<Tile> currentGroup;
      int jokersUsed = 0;
      bool possible = true;

      for (int i = 0; i < templateSize; ++i) {
        Color colorNeeded = groupTemplate[i];
        if (mask & (1 << i)) {
          // Use Joker
          jokersUsed++;
          currentGroup.emplace_back(Tile(numberValue, colorNeeded, true));
        } else {
          // Use Real
          if (colorToTile.count(colorNeeded)) {
            currentGroup.push_back(colorToTile.at(colorNeeded));
          } else {
            possible = false;
            break;
          }
        }
      }

      if (possible && jokersUsed <= nJokers && jokersUsed < templateSize) {
        allGroups.push_back(Set(SetType::Group, currentGroup));
      }
    }
  }

  return allGroups;
}

inline std::vector<Set> generate_runs(const std::vector<Tile>& tilesOfColor, const int nJokers) {
  std::vector<Set> allRuns;
  if (tilesOfColor.empty() && nJokers < MIN_SET_SIZE) return allRuns;

  Color runColor = tilesOfColor.empty() ? Color::None : tilesOfColor[0].color;

  std::array<std::optional<Tile>, MAX_TILE_VALUE+1> valueToTile{};
  for (const auto& tile : tilesOfColor) {
    assert(!tile.isJoker);
    valueToTile[tile.value] = tile;
  }

  // Top-down generation with early pruning
  for (int start = MIN_TILE_VALUE; start <= MAX_TILE_VALUE - MIN_SET_SIZE + 1; ++start) {
    
    std::vector<Tile> currentRun;
    
    // Helper to explore all combinations of Jokers for a fixed start/end
    auto explore = [&](auto self, int currentVal, int jokersUsed) -> void {
      if (currentVal > MAX_TILE_VALUE) return;

      // Try using the real tile if available
      if (valueToTile[currentVal]) {
        currentRun.push_back(valueToTile[currentVal].value());
        if (currentRun.size() >= MIN_SET_SIZE) {
          allRuns.push_back(Set(SetType::Run, currentRun));
        }
        self(self, currentVal + 1, jokersUsed);
        currentRun.pop_back();
      }

      // Try using a joker if available
      if (jokersUsed < nJokers) {
        currentRun.push_back(Tile(currentVal, runColor, true));
        if (currentRun.size() >= MIN_SET_SIZE) {
          allRuns.push_back(Set(SetType::Run, currentRun));
        }
        self(self, currentVal + 1, jokersUsed + 1);
        currentRun.pop_back();
      }
    };

    // Initialize with first tile (either real or joker)
    if (valueToTile[start]) {
      currentRun.push_back(valueToTile[start].value());
      explore(explore, start + 1, 0);
      currentRun.pop_back();
    }
    if (nJokers > 0) {
      currentRun.push_back(Tile(start, runColor, true));
      explore(explore, start + 1, 1);
      currentRun.pop_back();
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

inline bool set_can_be_placed(const Set& set, const TileMap& availableTiles) {

  TileMap tilesNeededForThisSet;

  for (Tile t : set.tiles) {

    // 3. Normalize the joker so it matches what is actually in availableTiles
    if (t.isJoker) {
      t.value = 0;           
      t.color = Color::None; 
    }

    tilesNeededForThisSet[t]++;

    int availableAmount = availableTiles[t];

    if (tilesNeededForThisSet[t] > availableAmount) {
      return false;
    }
  }
  return true;
}

inline bool all_original_tiles_placed(TileMap& boardTiles) {
  for (const auto& freq : boardTiles) {
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

inline std::vector<Tile> get_newly_placed_tiles(const Board& oldBoard, const Board& newBoard) {
  std::vector<Tile> newlyPlacedTiles;
  TileMap map;

  std::vector<Tile> oldTiles = oldBoard.tiles_on_board();
  std::vector<Tile> newTiles = newBoard.tiles_on_board();

  normalize_jokers(oldTiles);
  normalize_jokers(newTiles);

  for (const Tile& t : newTiles) {
    map[t]++;
  }
  for (const Tile& t : oldTiles) {
    map[t]--;
  }

  Tile joker(0, Color::None, true);
  for (int j = 0; j < map[joker]; ++j) {
    newlyPlacedTiles.emplace_back(joker);
  }

  for (int val = MIN_TILE_VALUE; val <= MAX_TILE_VALUE; ++val) {
    for (Color c : ALL_COLORS) {
      Tile t(val, c);
      for (int j = 0; j < map[t]; ++j) {
        newlyPlacedTiles.emplace_back(t);
      }
    }
  }

  return newlyPlacedTiles;
}

#endif







