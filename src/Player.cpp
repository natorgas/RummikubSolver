#include "Player.hpp"
#include "Constants.hpp"
#include "GameTypes.hpp"
#include "TilesBag.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <vector>


/********************* Player *******************/


Player::Player(std::string nme) : name(nme), 
                                  hasMadeFirstMove(false), 
                                  n_owned_tiles(INITIAL_N_OWNED_TILES) {}

std::string Player::get_name() const { return name; }


/************************************************/

/********************* Player -> Human *******************/

void HumanPlayer::play_turn(Board& board, TilesBag& bag) {
  while (true) {

    // Initiate 'sandbox'
    Board boardCopy = board;

    // Show the usser the current board
    board.print();

    // If anything goes wrong, prompt again
    if (!make_move(boardCopy, bag)) continue;

    // If move is valid, realize the change
    board = std::move(boardCopy);

    bool DONE = false;

    while (true) {
    std::cout << "Are you done with your turn? [y/n].\n";
    char done;
    std::cin >> done;

      if (done == 'y') {
        DONE = true;
        break;
      }

      else if (done == 'n') break;

      else std::cout << done << " is not a valid answer. Try again.\n";
    }

    if (DONE) break;
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

  // joker_check accounts for potential joker usage
  while(!joker_check(newGroupTiles));

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

  while (!joker_check(newRunTiles));
  newRun.tiles = newRunTiles;

  boardCopy.add_set(newRun);

  return true;
}

bool HumanPlayer::remove_group(Board& boardCopy) {
  std::cout << "Enter the index of the group you want to remove: ";
  int index;
  std::cin >> index;

  // Todo

}

bool HumanPlayer::remove_run(Board& boardCopy) {
  return true;
}

/*********************************************************/

/********************* Player -> AI *******************/

bool AIPlayer::draw_tile(TilesBag& bag) {
  return true;
}

void AIPlayer::play_turn(Board& board, TilesBag& bag) {

}

/******************************************************/


