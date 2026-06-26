#include "Player.hpp"
#include "Constants.hpp"
#include "GameTypes.hpp"
#include "TilesBag.hpp"
#include "Utils.hpp"
#include "TileMap.hpp"
#include <algorithm>
#include <cassert>
#include <string>
#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>

/********************* Player *******************/

Player::Player(std::string nme) : name(nme), 
                                  hasMadeFirstMove(false) {}

std::string Player::get_name() const { return name; }

void Player::add_to_hand(const Tile& t) {
  hand.push_back(t);
}

const std::vector<Tile>& Player::get_hand() const {
  return hand;
}


void Player::inital_draw(TilesBag& bag) {
  std::cout << get_name() << ", draw your initial tiles.\n";
  for (int i = 0; i < INITIAL_N_OWNED_TILES; ++i) {
    draw_tile(bag);
  }
}

void Player::make_first_move() { hasMadeFirstMove = true; }

bool Player::made_first_move() const { return hasMadeFirstMove; }

int Player::n_owned_tiles() const { return hand.size(); }

bool Player::placed_all_tiles() const { return n_owned_tiles() == 0; }

/************************************************/

/********************* Player -> Human *******************/

HumanPlayer::HumanPlayer(std::string nme) : Player(nme), nOwnedTiles(0) {}

int HumanPlayer::n_owned_tiles() const { return nOwnedTiles; }

void HumanPlayer::decrease_tiles(int amount) { nOwnedTiles -= amount; }

void HumanPlayer::play_turn(Board& board, TilesBag& bag) {}
bool HumanPlayer::draw_tile(TilesBag& bag) {
  if (!bag.is_empty()){
    bag.draw(); // Consume a tile from the bag
    nOwnedTiles++;
    return true;
  }
  else {
    std::cout << "Bag is empty. Try again.\n";
    return false;
  }
}

/********************* Player -> AI *******************/

AIPlayer::AIPlayer(std::string nme) : stepCounter(0), Player(nme) {}

bool AIPlayer::draw_tile(TilesBag& bag) {
  std::cout << "MUST_DRAW_TILE" << std::endl;
  if (progressCallback) progressCallback("MUST_DRAW_TILE");
  return true;
}

void AIPlayer::play_turn(Board& board, TilesBag& bag) {
  std::vector<Tile> tilesOnBoard = {};
  // Only put board tiles into pool if we have made our first move
  if (made_first_move()) {
    tilesOnBoard = board.tiles_on_board();
  }
  std::vector<Tile> pool = tilesOnBoard;
  pool.insert(pool.end(), hand.begin(), hand.end());
  std::vector<Set> allSets = generate_all_sets(pool);
  normalize_jokers(tilesOnBoard);
  // Frequency of all tiles that can be used to build sets
  TileMap allTiles;
  // Must all be placed -> need every entry to have freq = 0 (all placed)
  TileMap boardTiles;
  for (const Tile& t : tilesOnBoard) {
    boardTiles[t]++;
    allTiles[t]++;
  }
  // Add missing tiles to allTiles (the ones on hand)
  for (const Tile& t : hand) {
    allTiles[t]++;
  }
  // Don't use board.size, for the case that this is our first move -> need this to be 0
  const int initialBoardSize = tilesOnBoard.size();
  int initialBoardValSum = 0;
  for (const Tile& t : tilesOnBoard) {
    initialBoardValSum += t.value;
  }
  std::vector<int> setIndexToUseFreq(allSets.size(), 0);
  std::vector<int> bestSetIndexToUseFreq(allSets.size(), 0);
  int maxHandTilesUsed = 0;
  const int allSetsIndex = 0;
  // Sort in descending order (heuristic)
  std::sort(allSets.begin(), allSets.end(), 
            [](const Set& a, const Set& b){ return a.size() > b.size(); });
  
  int unplacedBoardTiles = initialBoardSize;
  int nTilesOnBoardCount = 0;
  int currentBoardValSum = 0;

  // Maps tile to a vector of indices i such that allSets[i] contains that tile
  TileSetsMap setsContainingTileFast;
  for (int i = 0; i < allSets.size(); ++i) {
    for (const Tile& t : allSets[i].tiles) {
      if (std::find(setsContainingTileFast[t].begin(), setsContainingTileFast[t].end(), i) == setsContainingTileFast[t].end()) {
        setsContainingTileFast[t].push_back(i);
      }
    }
  }

  auto startTime = std::chrono::high_resolution_clock::now();

  find_best_move(allSets, initialBoardSize, boardTiles, allTiles, setIndexToUseFreq,
                 bestSetIndexToUseFreq, maxHandTilesUsed, allSetsIndex, startTime, initialBoardValSum,
                 unplacedBoardTiles, nTilesOnBoardCount, currentBoardValSum, setsContainingTileFast);

  auto endTime = std::chrono::high_resolution_clock::now();
  // Calculate time used by find_best_move
  std::chrono::duration<double> duration = endTime - startTime;
  std::cout << std::fixed << std::setprecision(2);
  std::cout << "Execution time: " << duration.count() << " s\n";
  bool mustDraw = false;
  Board newBoard;
  // If we can't place any tiles we need to draw a tile
  if (maxHandTilesUsed == 0) {
    mustDraw = true;
  }
  // Else we were able to place at least one tile
  else {
    // If we have not made first move yet, copy old board, as we only worked with tiles from our hand on this turn
    if (!made_first_move()) {
      newBoard = board;
    }
    // Finish the new board
    for (int i = 0; i < bestSetIndexToUseFreq.size(); ++i) {
      for (int freq = 0; freq < bestSetIndexToUseFreq[i]; ++freq) {
        newBoard.add_set(allSets[i]);
      }
    }
    // If we have not made first move yet, check if sum of placed tiles >= minFirstMoveSum
    if (!made_first_move()) {
      int moveSum = val_sum_of_placed_tiles(board, newBoard);
      if (moveSum < MIN_FIRST_MOVE_SUM) {
        mustDraw = true;
      }
    }
  }
  if (mustDraw) {
    std::cout << "You have no moves left, draw a tile.\n";
    if (!bag.is_empty()) {
      draw_tile(bag);
    }
    else {
      std::cout << "You have no moves left and the bag is empty.\n";
    }
  }

  else {
    std::cout << "Was able to place " << maxHandTilesUsed << " tiles." << std::endl;
    std::vector<Tile> newlyPlacedTiles = get_newly_placed_tiles(board, newBoard); 
    board = std::move(newBoard);
    if (!made_first_move()) {
      make_first_move();
    }
    std::cout << "Placed tiles:\n";
    std::string placedStr = "Placed:\n";
    // Remove used tiles from our hand
    for (const Tile& toErase : newlyPlacedTiles) {
      if (toErase.isJoker) {
        placedStr += "Joker\n";
      } else {
        placedStr += std::to_string(toErase.value) + " " + color_to_str(toErase.color) + "\n";
      }
      auto it = std::find(hand.begin(), hand.end(), toErase);
      assert(it != hand.end());
      hand.erase(it);
    } 
    if (progressCallback) progressCallback("Turn done.\nTook " + std::to_string(duration.count()).substr(0,4) + "s.\n" + placedStr);
    std::cout << std::endl;
  }
  std::cout << "Current Board: " << std::endl;
}

bool AIPlayer::find_best_move(
    const std::vector<Set>& allSets, 
    const int               initialBoardSize,
    TileMap&                boardTiles,
    TileMap&                availableTiles,
    std::vector<int>&       setIndexToUseFreq,
    std::vector<int>&       bestSetIndexToUseFreq,
    int&                    maxHandTilesUsed,
    const int               allSetsIndex,
    const std::chrono::high_resolution_clock::time_point& startTime,
    const int               initialBoardValSum,
    int                     unplacedBoardTiles,
    int                     nTilesOnBoardCount,
    int                     currentBoardValSum,
    const TileSetsMap&      setsContainingTile) {

  if (++stepCounter % 1000 == 0) {
    auto now = std::chrono::high_resolution_clock::now();
    if (now - startTime > std::chrono::duration<double>(TIME_LIMIT)) {
      std::cout << "Exceeded time limit of " << TIME_LIMIT << " seconds.\n";
      return true;
    }
  }

  if (unplacedBoardTiles == 0) {
    const int nHandTilesUsed = nTilesOnBoardCount - initialBoardSize;
    assert(nHandTilesUsed >= 0);

    bool isValidMove = true;
    if (!made_first_move() && (currentBoardValSum - initialBoardValSum < MIN_FIRST_MOVE_SUM)) {
      isValidMove = false;
    }

    // Update stats if we found a move that places more tiles
    if (isValidMove && nHandTilesUsed > maxHandTilesUsed) {
      maxHandTilesUsed = nHandTilesUsed;
      bestSetIndexToUseFreq = setIndexToUseFreq;
      std::cout << "Can place " << maxHandTilesUsed << std::endl;
      if (progressCallback) progressCallback("Can place " + std::to_string(maxHandTilesUsed) + " tiles.");
      // Signal a win if we placed all our tiles
      if (maxHandTilesUsed == n_owned_tiles()) {
        std::cout << get_name() << " won!\n";
        return true;
      }
    }
  }

  if (unplacedBoardTiles > 0) {
    Tile firstUnplaced(0, Color::None);
    int minSets = allSets.size();

    // Choose which tile we will try to place down next:
    // For every tile on board check how many sets that contain that tile we
    // can build with the remaining tiles. Choose tile with the least amount of options
    // for the sake of efficiency
    for (int val = MIN_TILE_VALUE; val <= MAX_TILE_VALUE; ++val) {
      for (Color c : ALL_COLORS) {
        Tile t(val, c);
        if (boardTiles[t] > 0) {
          int validSetsCount = 0;
          for (int i : setsContainingTile[t]) {
            if (set_can_be_placed(allSets[i], availableTiles)) {
              validSetsCount++;
            }
          }
          if (validSetsCount < minSets) {
            minSets = validSetsCount;
            firstUnplaced = t;
          }
        }
      }
    }
    
    // Jokers handled separately
    Tile joker(0, Color::None, true);
    if (boardTiles[joker] > 0) {
      int validSetsCount = 0;
      for (int i : setsContainingTile[joker]) {
        if (set_can_be_placed(allSets[i], availableTiles)) {
          validSetsCount++;
        }
      }
      if (validSetsCount < minSets) {
        minSets = validSetsCount;
        firstUnplaced = joker;
      }
    }

    // If there is no set we can place which contains the tile which we must place, prune
    if (minSets == 0) return false;
    for (int i : setsContainingTile[firstUnplaced]) {
      const Set& trialSet = allSets[i];
      if (set_can_be_placed(trialSet, availableTiles)) { // TODO: We could save which sets can be placed in the first pass
        setIndexToUseFreq[i]++;
        std::vector<Tile> decrementedBoardTiles;
        decrementedBoardTiles.reserve(trialSet.size());
        
        int boardTilesDecremented = 0;
        int trialSetValSum = 0;

        for (const Tile& t : trialSet.tiles) {
          trialSetValSum += t.value;
          if (t.isJoker) {
            Tile jokerTile(0, Color::None, true);
            availableTiles[jokerTile]--;
            if (boardTiles[jokerTile] > 0) {
              boardTiles[jokerTile]--;
              decrementedBoardTiles.push_back(jokerTile);
              boardTilesDecremented++;
            }
          }
          else {
            availableTiles[t]--;
            if (boardTiles[t] > 0) {
              boardTiles[t]--;
              decrementedBoardTiles.push_back(t);
              boardTilesDecremented++;
            }
          }
        }

        if (find_best_move(allSets, initialBoardSize, boardTiles, availableTiles,
              setIndexToUseFreq, bestSetIndexToUseFreq, maxHandTilesUsed, allSetsIndex, startTime, initialBoardValSum,
              unplacedBoardTiles - boardTilesDecremented, nTilesOnBoardCount + trialSet.size(), currentBoardValSum + trialSetValSum, 
              setsContainingTile)) {
          return true;
        }

        // find_best_move returned false, we backtrack by removing the set we previously used
        setIndexToUseFreq[i]--;
        for (const Tile& t : trialSet.tiles) {
          if (t.isJoker) {
            availableTiles[Tile(0, Color::None, true)]++;
          }
          else availableTiles[t]++;
        }
        for (const Tile& t : decrementedBoardTiles) {
          if (t.isJoker) {
            boardTiles[Tile(0, Color::None, true)]++;
          }
          else boardTiles[t]++;
        }
      }
    }
    return false;
  } 

  // At this point all original tiles have been placed and we check if we can further increase our score
  else {
    if (allSetsIndex >= allSets.size()) return false;
    for (int i = allSetsIndex; i < allSets.size(); ++i) {
      const Set& trialSet = allSets[i];

      if (set_can_be_placed(trialSet, availableTiles)) {
        setIndexToUseFreq[i]++;
        
        int trialSetValSum = 0;
        for (const Tile& t : trialSet.tiles) {
          trialSetValSum += t.value;
          if (t.isJoker) availableTiles[Tile(0, Color::None, true)]--;
          else availableTiles[t]--;
        }

        // Try to milk it even more
        if (find_best_move(allSets, initialBoardSize, boardTiles, availableTiles,
                           setIndexToUseFreq, bestSetIndexToUseFreq, maxHandTilesUsed, i, startTime, initialBoardValSum,
                           unplacedBoardTiles, nTilesOnBoardCount + trialSet.size(), currentBoardValSum + trialSetValSum, 
                           setsContainingTile)) {
          return true;
        }

        setIndexToUseFreq[i]--;
        for (const Tile& t : trialSet.tiles) {
          if (t.isJoker) availableTiles[Tile(0, Color::None, true)]++;
          else availableTiles[t]++;
        }
      }
    }
    return false;
  }
}

/******************************************************/


void Player::set_hand(const std::vector<Tile>& h) {
  hand = h;
}

void Player::set_progress_callback(std::function<void(std::string)> cb) {
  progressCallback = cb;
}
