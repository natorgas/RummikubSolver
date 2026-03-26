#include "Player.hpp"
#include "Constants.hpp"
#include "GameTypes.hpp"
#include "TilesBag.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <cassert>
#include <new>
#include <string>
#include <iostream>
#include <vector>
#include <map>


/********************* Player *******************/

Player::Player(std::string nme) : name(nme), 
                                  hasMadeFirstMove(false), 
                                  nOwnedTiles(0) {}

std::string Player::get_name() const { return name; }

void Player::inital_draw(TilesBag& bag) {
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

    if (!made_first_move()) {
      make_first_move();
    }

    bool DONE = false;

    while (true) {
      std::cout << "Are you done with your turn? [y/n].\n";
      char done;
      std::cin >> done;

      if (done == 'y') {

        // Make sure all tiles that were on the board before are also on the board now
        // No permanent removal of tiles allowed
        std::vector<Tile> tilesOnOldBoard = initialBoard.tiles_on_board();
        std::vector<Tile> tilesOnNewBoard = board.tiles_on_board();

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
      decrease_tiles(board.size() - initialBoard.size());
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

  if (input == "cg") return create_group(boardCopy);

  else if (input == "cr") return create_run(boardCopy);

  else if (input == "rg") return remove_group(boardCopy);

  else if (input == "rr") return remove_run(boardCopy);

  else if (input == "a")  return true;
  
  else if (input == "d")  return draw_tile(bag);

  else {
    std::cout << "'" <<input << "'" << " is not a valid move. Try again.\n";
    return false;
  }

  return true;
}

bool HumanPlayer::draw_tile(TilesBag& bag) {
  bag.draw();
  increase_tiles(1);
  return true;
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

AIPlayer::AIPlayer(std::string nme) : Player(nme) {}

bool AIPlayer::draw_tile(TilesBag& bag) {
  std::cout << "Enter the tile you drew in the format 'value color'.\n";
  int val;
  std::string c;
  std::cin >> val >> c;
  Color col = str_to_color(c);

  bag.draw();
  Tile drawnTile(val, col);
  hand.push_back(drawnTile);
  bag.remove_tile(drawnTile);

  increase_tiles(1);

  return true;
}

void AIPlayer::play_turn(Board& board, TilesBag& bag) {
  std::vector<Tile> tilesOnBoard = board.tiles_on_board();
  std::vector<Tile> pool = tilesOnBoard;
  pool.insert(pool.end(), hand.begin(), hand.end());

  std::vector<Set> allSets = generate_all_sets(pool);

  // Frequency of all tiles that can be used to build sets
  std::map<Tile, int> allTiles;

  // Must all be placed -> need every entry to have freq = 0 (all placed)
  std::map<Tile, int> boardTiles;

  for (const Tile& t : tilesOnBoard) {
    boardTiles[t]++;
    allTiles[t]++;
  }

  // Add missing tiles to allTiles (the ones on hand)
  for (const Tile& t : hand) {
    allTiles[t]++;
  }

  const int initialBoardSize = board.size();
  std::vector<int> setIndexToUseFreq(allSets.size(), 0);
  std::vector<int> bestSetIndexToUseFreq(allSets.size(), 0);
  int maxHandTilesUsed = 0;
  const int allSetsIndex = 0;

  find_best_move(allSets, initialBoardSize, boardTiles, allTiles, setIndexToUseFreq,
                 bestSetIndexToUseFreq, maxHandTilesUsed, allSetsIndex);

  Board newBoard;

  for (int i = 0; i < bestSetIndexToUseFreq.size(); ++i) {
    for (int freq = 0; freq < bestSetIndexToUseFreq[i]; ++freq) {
      newBoard.add_set(allSets[i]);
    }
  }

  if (maxHandTilesUsed > 0) {
    std::cout << "Was able to place " << maxHandTilesUsed - initialBoardSize << " tiles." << std::endl;
    board = std::move(newBoard);
  }
  
  else {
    std::cout << "You have no moves left, draw a tile." << std::endl;
    draw_tile(bag);
  }

  std::cout << "Current Board: " << std::endl;
  board.print();
}

void AIPlayer::find_best_move(
    const std::vector<Set>& allSets, 
    const int               initialBoardSize,
    std::map<Tile, int>&    boardTiles,
    std::map<Tile, int>&    availableTiles,
    std::vector<int>&       setIndexToUseFreq,
    std::vector<int>&       bestSetIndexToUseFreq,
    int&                    maxHandTilesUsed,
    const int               allSetsIndex
) const {

  // Iterate over allSets, try to use set i. For that check if each tile has availableTiles[tile] >= 1
    // If yes -> 'use' that set and decrease available tiles for each tile and boardTiles for each tile that still has positive value in the map
      // If every entry in boardTiles has value 0 we have a valid board -> max = max(currentUsedTiles, previousMax), save bestBoard if max updated
    // If no -> try next set
    // If we ever manage to place <handSize> tiles -> break

  if (allSetsIndex >= allSets.size()) {
    // std::cout << "Reached the end of vector.\n"; // DEBUG
    return;
  }

  if (all_original_tiles_placed(boardTiles)) {
    // std::cout << "Placed all" << std::endl;
    // If yes count how many tiles we placed and update best move accordingly
    int tilesOnBoard = 0;
    for (int i = 0; i < setIndexToUseFreq.size(); i++) {
      tilesOnBoard += allSets[i].size() * setIndexToUseFreq[i];
    }
    const int nHandTilesUsed = tilesOnBoard - initialBoardSize;
    // std::cout << "Tiles on board = " << tilesOnBoard << std::endl;
    // std::cout << "Initial board size = " << initialBoardSize << std::endl;
    // std::cout << "Tiles used = " << nHandTilesUsed << std::endl;
    assert(nHandTilesUsed >= 0);
    if (nHandTilesUsed > maxHandTilesUsed) {
      maxHandTilesUsed = nHandTilesUsed;
      bestSetIndexToUseFreq = setIndexToUseFreq;

      // If we used all tiles on hand, we won
      if (maxHandTilesUsed == n_owned_tiles()) {
        std::cout << get_name() << " won!\n";
        return;
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
      availableTiles[t]--;
      if (boardTiles[t] > 0) {
        boardTiles[t]--;
        decrementedBoardTiles.push_back(t);
      }
    }

    // Go to next set
    find_best_move(allSets, initialBoardSize, boardTiles, availableTiles,
                   setIndexToUseFreq, bestSetIndexToUseFreq, maxHandTilesUsed, allSetsIndex);

    // If we return from the upper function => placing this set did not work out, remove it
    setIndexToUseFreq[allSetsIndex]--;
    for (const Tile& t : trialSet.tiles) {
      availableTiles[t]++;
    }
    for (const Tile& t : decrementedBoardTiles) {
      boardTiles[t]++;
    }
  }
  
  // We go to next set independently of whether or not we were able to place current set
  find_best_move(allSets, initialBoardSize, boardTiles, availableTiles,
                 setIndexToUseFreq, bestSetIndexToUseFreq, maxHandTilesUsed, allSetsIndex+1);

}

/******************************************************/


