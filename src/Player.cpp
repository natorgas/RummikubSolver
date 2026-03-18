#include "Player.hpp"
#include "Constants.hpp"
#include <string>
#include <iostream>
#include <sys/types.h>


/********************* Player *******************/


Player::Player(std::string nme) : name(nme), 
                                  hasMadeFirstMove(false), 
                                  n_owned_tiles(INITIAL_N_OWNED_TILES) {}

std::string Player::get_name() const { return name; }


/************************************************/

/********************* Player -> Human *******************/

void HumanPlayer::play_turn(Board& board) {
  while (true) {


    break;
  }
}

bool HumanPlayer::make_move(Board& board) {

  std::cout << get_name() << ", do you want to \n"
            << "-Create new group [cg] \n" 
            << "-Create new run [cr] \n" 
            << "-Remove group [eg] \n"
            << "-Remove run [er]"
            << std::endl;

  std::string input;
  std::cin >> input;

  // switch (input) {
  //   case "cg":
  //
  //   case "cr":
  //
  //   case "eg":
  //
  //   case "er":
  //
  //   default:
  //     std::cout << "'" << input << "' is not a valid move. Try again. \n";
  // }

}

/*********************************************************/

/********************* Player -> AI *******************/

void AIPlayer::play_turn(Board& board) {

}

/******************************************************/


