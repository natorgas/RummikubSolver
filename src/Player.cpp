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
                                  hasMadeFirstMove(false), 
                                  nOwnedTiles(0) {}

std::string Player::get_name() const { return name; }

void Player::add_to_hand(const Tile& t) {
  hand.push_back(t);
  increase_tiles(1);
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

void Player::increase_tiles(int n) {
  // nOwnedTiles is redundant now that hand is in base class
}

void Player::decrease_tiles(int n) {
  // nOwnedTiles is redundant now that hand is in base class
}

void Player::make_first_move() { hasMadeFirstMove = true; }

bool Player::made_first_move() const { return hasMadeFirstMove; }

int Player::n_owned_tiles() const { return hand.size(); }

bool Player::placed_all_tiles() const { return n_owned_tiles() == 0; }

/************************************************/

/********************* Player -> Human *******************/

HumanPlayer::HumanPlayer(std::string nme) : Player(nme) {}

void HumanPlayer::play_turn(Board& board, TilesBag& bag) {

  Board initialBoard = board;

  while (true) {

    // Initiate 'sandbox'
    Board boardCopy = board;

    // Show the user the current board
    boardCopy.print();

    // If anything goes wrong, prompt again
    if (!make_move(boardCopy, bag)) continue;

    // If move is valid, realize the change
    board = std::move(boardCopy);

    bool DONE = false;

    while (true) {
      std::cout << "Are you done with your turn? [y/n]. Enter 'y' if you drew a tile.\n";
      char done;
      std::cin >> done;

      if (done == 'y') {

        // Make sure all tiles that were on the board before are also on the board now
        // No permanent removal of tiles allowed
        std::vector<Tile> tilesOnOldBoard = initialBoard.tiles_on_board();
        std::vector<Tile> tilesOnNewBoard = board.tiles_on_board();

        // Normalize jokers s.t. we don't get an error if a joker's role has changed due to a move
        normalize_jokers(tilesOnOldBoard);
        normalize_jokers(tilesOnNewBoard);

        std::sort(tilesOnOldBoard.begin(), tilesOnOldBoard.end());
        std::sort(tilesOnNewBoard.begin(), tilesOnNewBoard.end());

        // If inclusion not satisfied, reset board and start over
        if (!std::includes(tilesOnNewBoard.begin(), tilesOnNewBoard.end(),
                           tilesOnOldBoard.begin(), tilesOnOldBoard.end())) {
          std::cout << "You need to place down all tiles that you picked up by 'removing'" << std::endl;
          std::cout << "Board was reset, start yor turn from the beginning.\n";
          board = initialBoard;
          break;
        }

        DONE = true;
        break;
      }

      else if (done == 'n') break;

      else std::cout << done << " is not a valid answer. Try again.\n";
    }

    if (DONE) {
      int tilesPlaced = board.size() - initialBoard.size();
      if (tilesPlaced == 0) {
        break; // Player skipped or only drew a tile
      }

      // If first turn was not made yet, make minFirstMoveSum is placed
      if (!made_first_move()) {
        const int sumOfPlacedTilesValues = val_sum_of_placed_tiles(initialBoard, board);
        if (sumOfPlacedTilesValues < MIN_FIRST_MOVE_SUM) {
          std::cout << "You must place down tiles worth "
                    << MIN_FIRST_MOVE_SUM
                    << " on your first move. Board has been reset, try again.\n";
          board = initialBoard;
          continue;
        }
      }

      // Else decrease tiles and end the turn
      decrease_tiles(tilesPlaced);
      make_first_move();
      break;
    }
  }
}

bool HumanPlayer::make_move(Board& boardCopy, TilesBag& bag) {

  // boardCopy as a reminder that we are not changing anything about the real board

  std::cout << std::endl;
  std::cout << get_name() << ", do you want to \n"
                          << "-Create new group [cg] \n" 
                          << "-Create new run [cr] \n" 
                          << "-Remove group [rg] \n"
                          << "-Remove run [rr] \n"
                          << "-Draw a tile [d] \n"
                          << "-Abort [a]"
                          << std::endl;

  std::string input;
  std::cin >> input;

  if (input == "cg") return create_group(boardCopy) ? 1 : 0;

  else if (input == "cr") return create_run(boardCopy) ? 1 : 0;

  else if (input == "rg") {
    if (!made_first_move()) {
      std::cout << "You cannot modify existing sets on your first move.\n";
      return 0;
    }
    return remove_group(boardCopy) ? 1 : 0;
  }

  else if (input == "rr") {
    if (!made_first_move()) {
      std::cout << "You cannot modify existing sets on your first move.\n";
      return 0;
    }
    return remove_run(boardCopy) ? 1 : 0;
  }

  else if (input == "a")  return true;
  
  else if (input == "d")  return draw_tile(bag);

  else {
    std::cout << "'" <<input << "'" << " is not a valid move. Try again.\n";
    return false;
  }

  return true;
}

bool HumanPlayer::draw_tile(TilesBag& bag) {
  if (!bag.is_empty()){
    bag.draw(); // Consume a tile from the bag
    
    int val = rand() % 13 + 1;
    Color col = static_cast<Color>(rand() % 4);
    Tile t(val, col);
    if (rand() % 106 < 2) {
        t = Tile(0, Color::None, true);
    }
    
    add_to_hand(t);
    return true;
  }
  else {
    std::cout << "Bag is empty. Try again.\n";
    return false;
  }
}

bool HumanPlayer::create_group(Board& boardCopy) {

  int groupValue, tilesInGroup;

  std::cout << "What is the the value of all tiles in this group? ";
  std::cin >> groupValue;

  std::cout << "How many tiles are in this group? ";
  std::cin >> tilesInGroup;

  // Check for invalid input
  if (groupValue < MIN_TILE_VALUE ||
      groupValue > MAX_TILE_VALUE ||
      tilesInGroup < MIN_SET_SIZE ||
      tilesInGroup > MAX_GROUP_SIZE) {
    std::cout << "Input invalid. Try again.\n";
    return false;
  }

  // Create Set that will be added
  Set newGroup(SetType::Group, {});
  std::vector<Tile> newGroupTiles;

  // If 4 tiles in group => every color once
  if (tilesInGroup == MAX_GROUP_SIZE) {
    for (Color color : ALL_COLORS) {
      newGroupTiles.emplace_back(Tile(groupValue, color));
    }
  }

  // Only 3 tiles, need to know which color not to add
  else if (tilesInGroup == MIN_SET_SIZE) {
    std::cout << "Which color is NOT contained in the new group? ";
    std::string input;
    std::cin >> input;

    Color colorNotContained = str_to_color(input);

    for (Color color : ALL_COLORS) {
      if (color == colorNotContained) continue;
      newGroupTiles.emplace_back(Tile(groupValue, color));
    }
  }

  // joker_check accounts for potential joker usage and first move exceptions (e.g. 30 point rule)
  // Returns true in case of a successful check
  if (!joker_check(newGroupTiles, made_first_move())) return false;

  newGroup.tiles = newGroupTiles;

  boardCopy.add_set(newGroup);

  return true;
}

bool HumanPlayer::create_run(Board& boardCopy) {
  std::cout << "What color will the run be? ";
  std::string c;
  std::cin >> c;
  const Color runColor = str_to_color(c);

  std::cout << "Enter first and last value in this run.\n";

  int first, last;
  std::cin >> first >> last;

  const int tilesInRun = last - first + 1;

  if (first < 0 ||
      last < 0 ||
      first > MAX_TILE_VALUE ||
      last > MAX_TILE_VALUE ||
      first >= last ||
      tilesInRun < MIN_SET_SIZE) {
    std::cout << "Not a valid range. Try again.\n";
    return false;
  }

  Set newRun(SetType::Run, {});
  std::vector<Tile> newRunTiles;

  for (int val = first; val <= last; ++val) {
    newRunTiles.emplace_back(Tile(val, runColor));
  }

  if (!joker_check(newRunTiles, made_first_move())) return false;

  newRun.tiles = newRunTiles;

  boardCopy.add_set(newRun);

  return true;
}

bool HumanPlayer::remove_group(Board& boardCopy) {
  std::cout << "Enter the index of the group you want to remove: ";
  int index;
  std::cin >> index;

  // Index check
  if (boardCopy.groups.begin() + index >= boardCopy.groups.end() ||
      boardCopy.groups.begin() + index < boardCopy.groups.begin()) {
    std::cout << "Group index " << index << " does not exist. Try again.\n";
    return false;
  }

  boardCopy.groups.erase(boardCopy.groups.begin() + index);
  return true;
}

bool HumanPlayer::remove_run(Board& boardCopy) {
  std::cout << "Enter the index of the run you want to remove: ";
  int index;
  std::cin >> index;

  // Index check
  if (boardCopy.runs.begin() + index >= boardCopy.runs.end() ||
      boardCopy.runs.begin() + index < boardCopy.runs.begin()) {
    std::cout << "Run index " << index << " does not exist. Try again.\n";
    return false;
  }

  boardCopy.runs.erase(boardCopy.runs.begin() + index);
  return true;
}

/*********************************************************/

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

  // Don't use board.size, for that case that this is our first move -> need this to be 0
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

  auto startTime = std::chrono::high_resolution_clock::now();

  int unplacedBoardTiles = initialBoardSize;
  int nTilesOnBoardCount = 0;
  int currentBoardValSum = 0;

  // Maps tile to a vector of indices i such that allTiles[i] contains that tile
  TileSetsMap setsContainingTileFast;
  for (int i = 0; i < allSets.size(); ++i) {
    for (const Tile& t : allSets[i].tiles) {
      if (std::find(setsContainingTileFast[t].begin(), setsContainingTileFast[t].end(), i) == setsContainingTileFast[t].end()) {
        setsContainingTileFast[t].push_back(i);
      }
    }
  }

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
    decrease_tiles(maxHandTilesUsed);

    std::vector<Tile> newlyPlacedTiles = get_newly_placed_tiles(board, newBoard); 

    board = std::move(newBoard);
    if (!made_first_move()) {
      make_first_move();
    }

    std::cout << "Placed tiles:\n";
    std::string placedStr = "Placed:\n";

    // Remove used tiles from our hand
    for (const Tile& toErase : newlyPlacedTiles) {
      toErase.print();
      if (toErase.isJoker) {
        placedStr += "Joker\n";
      } else {
        placedStr += std::to_string(toErase.value) + " " + color_to_str(toErase.color) + "\n";
      }
      auto it = std::find(hand.begin(), hand.end(), toErase);
      assert(it != hand.end());
      hand.erase(it);
    } 
    
    if (progressCallback) progressCallback("Turn done.\n" + placedStr);

    std::cout << std::endl;
  }
  
  std::cout << "Current Board: " << std::endl;
  // board.print(); // Removed as requested

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

    if (isValidMove && nHandTilesUsed > maxHandTilesUsed) {
      maxHandTilesUsed = nHandTilesUsed;
      bestSetIndexToUseFreq = setIndexToUseFreq;
      std::cout << "Can place " << maxHandTilesUsed << std::endl;
      if (progressCallback) progressCallback("Can place " + std::to_string(maxHandTilesUsed) + " tiles.");
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
    // For every tile on board check how many sets that we can build with the 
    // remaining tiles contain that tile, choose tile with the least amount of options
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

      if (set_can_be_placed(trialSet, availableTiles)) {
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
