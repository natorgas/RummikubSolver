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
                                  hasMadeFirstMove(true), 
                                  nOwnedTiles(0) {}

std::string Player::get_name() const { return name; }

void Player::inital_draw(TilesBag& bag) {
  std::cout << get_name() << ", draw your initial tiles.\n";
  for (int i = 0; i < INITIAL_N_OWNED_TILES; ++i) {
    draw_tile(bag);
  }
}

void Player::increase_tiles(int n) {
  assert(n >= 0);
  nOwnedTiles += n;
}

void Player::decrease_tiles(int n) {
  assert(n >= 0);
  nOwnedTiles -= n;
  assert(nOwnedTiles >= 0 && "Can't have negative amount of tiles");
}

void Player::make_first_move() { hasMadeFirstMove = true; }

bool Player::made_first_move() const { return hasMadeFirstMove; }

int Player::n_owned_tiles() const { return nOwnedTiles; }

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
    bag.draw();
    increase_tiles(1);
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

AIPlayer::AIPlayer(std::string nme) : hand({}), stepCounter(0), Player(nme) {}

bool AIPlayer::draw_tile(TilesBag& bag) {
  std::cout << "Enter the tile (either 'Joker' or 'value color'):\n";

  std::string input;
  std::cin >> input;

  if (input == "Joker" || input == "joker") {
    Tile jokerTile(0, Color::None, true);
    hand.push_back(jokerTile);
  }

  else {
    int val = std::stoi(input);

    std::string c;
    std::cin >> c;

    Color col = str_to_color(c);

    Tile drawnTile(val, col);
    hand.push_back(drawnTile);
  }

  bag.draw();
  increase_tiles(1);

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

  find_best_move(allSets, initialBoardSize, boardTiles, allTiles, setIndexToUseFreq,
                 bestSetIndexToUseFreq, maxHandTilesUsed, allSetsIndex, startTime, initialBoardValSum);

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

    // Remove used tiles from our hand
    for (const Tile& toErase : newlyPlacedTiles) {
      auto it = std::find(hand.begin(), hand.end(), toErase);
      assert(it != hand.end());
      hand.erase(it);
    } 
  }
  
  std::cout << "Current Board: " << std::endl;
  board.print();
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
    const int               initialBoardValSum
) {

  // Check how long we have been looking for the best move
  if (++stepCounter % 1000 == 0) {
    auto now = std::chrono::high_resolution_clock::now();
    if (now - startTime > std::chrono::duration<double>(TIME_LIMIT)) {
      std::cout << "Exceeded time limit of " << TIME_LIMIT << " seconds.\n";
      return true;
    }
  }

  if (allSetsIndex >= allSets.size()) {
    return false;
  }

  // Check if we placed all necessary tiles
  if (all_original_tiles_placed(boardTiles)) {
    int tilesOnBoard = 0;
    int currentBoardValSum = 0;

    // Calculate number of tiles on board
    for (int i = 0; i < setIndexToUseFreq.size(); i++) {
      tilesOnBoard += allSets[i].size() * setIndexToUseFreq[i];
    }

    // If we have not made first move, additionally calculate the sum of values of all tiles
    if (!made_first_move()) {
      for (int i = 0; i < setIndexToUseFreq.size(); i++) {
        for (const Tile& t : allSets[i].tiles) {
          currentBoardValSum += t.value * setIndexToUseFreq[i];
        }
      }
    }

    const int nHandTilesUsed = tilesOnBoard - initialBoardSize;
    assert(nHandTilesUsed >= 0);

    bool isValidMove = true;

    // If we have not made first move and we dont meet the minimum sum requirement, mark this move as invalid
    if (!made_first_move() && (currentBoardValSum - initialBoardValSum < MIN_FIRST_MOVE_SUM)) {
      isValidMove = false;
    }

    // Only consider a valid move that improves the number of tiles we place
    if (isValidMove && nHandTilesUsed > maxHandTilesUsed) {
      maxHandTilesUsed = nHandTilesUsed;
      bestSetIndexToUseFreq = setIndexToUseFreq;
      std::cout << "Can place " << maxHandTilesUsed << std::endl;

      // If we used all tiles on hand, we won
      if (maxHandTilesUsed == n_owned_tiles()) {
        std::cout << get_name() << " won!\n";
        return true;
      }
    }
  }

  const Set& trialSet = allSets[allSetsIndex];

  // If set can be placed
  if (set_can_be_placed(trialSet, availableTiles)) {

    // Place it
    setIndexToUseFreq[allSetsIndex]++;

    std::vector<Tile> decrementedBoardTiles;
    decrementedBoardTiles.reserve(trialSet.size());

    // And decrement tiles correctly
    for (const Tile& t : trialSet.tiles) {
      if (t.isJoker) {
        Tile joker(0, Color::None, true);
        availableTiles[joker]--;
        if (boardTiles[joker] > 0) {
          boardTiles[joker]--;
          decrementedBoardTiles.push_back(joker);
        }
      }
      else {
        availableTiles[t]--;
        if (boardTiles[t] > 0) {
          boardTiles[t]--;
          decrementedBoardTiles.push_back(t);
        }
      }
    }

    // Try this set again
    if (find_best_move(allSets, initialBoardSize, boardTiles, availableTiles,
                       setIndexToUseFreq, bestSetIndexToUseFreq, maxHandTilesUsed, allSetsIndex, startTime, initialBoardValSum)) {
      return true;
    }

    // If we return from the upper function => placing this set did not work out, remove it
    setIndexToUseFreq[allSetsIndex]--;
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
  
  // We go to next set independently of whether or not we were able to place current set
  if (find_best_move(allSets, initialBoardSize, boardTiles, availableTiles,
                     setIndexToUseFreq, bestSetIndexToUseFreq, maxHandTilesUsed, allSetsIndex+1, startTime, initialBoardValSum)) {
    return true;
  }
  return false;
}

/******************************************************/


