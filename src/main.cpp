#include "GameTypes.hpp"
#include "Player.hpp"
#include "TilesBag.hpp"
#include "Constants.hpp"
#include "Utils.hpp"
#include <cstdlib>
#include <iostream>
#include <memory>

int main() {

  Board board;

  Set s1(SetType::Group, { Tile(1, Color::Red),
                           Tile(1, Color::Black), 
                           Tile(1, Color::Orange)});

  Set s2(SetType::Group, { Tile(12, Color::Red),
                           Tile(12, Color::Black), 
                           Tile(12, Color::Blue), 
                           Tile(12, Color::Orange)});

  Set s3(SetType::Group, { Tile(3, Color::Red),
                           Tile(3, Color::Black), 
                           Tile(3, Color::Orange)});

  Set s4(SetType::Group, { Tile(10, Color::Red),
                           Tile(10, Color::Black), 
                           Tile(10, Color::Blue), 
                           Tile(10, Color::Orange)});

  Set s5(SetType::Run,   { Tile(7, Color::Red),
                           Tile(8, Color::Red), 
                           Tile(9, Color::Red), 
                           Tile(10, Color::Red)});
  
  Set s6(SetType::Run,   { Tile(3, Color::Blue),
                           Tile(4, Color::Blue), 
                           Tile(5, Color::Blue)});

  Set s7(SetType::Run,   { Tile(7, Color::Black),
                           Tile(8, Color::Black), 
                           Tile(9, Color::Black), 
                           Tile(10, Color::Black), 
                           Tile(11, Color::Black, true)});
  board.add_set(s1);
  board.add_set(s2);
  board.add_set(s3);
  board.add_set(s4);
  board.add_set(s5);
  board.add_set(s6);
  board.add_set(s7);
  
  TilesBag bag;

  int nPlayers;
  std::cout << "How many players are playing?\n";
  std::cin >> nPlayers;

  // Vector containing unique_ptrs to all players
  std::vector<std::unique_ptr<Player>> players;

  std::cout << "Enter the players' names, starting with the name of the AI assisted player.\n";
  for (int i = 0; i < nPlayers; ++i) {
    std::string name;
    std::cout << i << ": ";
    std::cin >> name;
    if (i == 0) {
      players.emplace_back(std::make_unique<AIPlayer>(name));
    }
    else {
      players.emplace_back(std::make_unique<HumanPlayer>(name));
    }
  }

  // General information to the user
  std::cout << "Values go from " << MIN_TILE_VALUE << " to " << MAX_TILE_VALUE << ".\n";
  std::cout << "Colors are: ";
  for (const Color& c : ALL_COLORS) {
    std::cout << color_to_str(c) << " ";
  }
  std::cout << std::endl;
  std::cout << "There is NO case-sensitivity for user input of type string.\n\n";

  // Let everyone draw their initial tiles
  for (auto& player_p : players) {
    player_p->inital_draw(bag);
  }

  bool gameOver = false;

  // Start the game
  while (!gameOver) {
    for (auto& player_p : players) {
      player_p->play_turn(board,  bag);
      exit(0);
      if (player_p->placed_all_tiles()) {
        std::cout << player_p->get_name() << ", wins. Congrats!\n";
        std::cout << "'The game is over' -Babu\n";
        gameOver = true;
        break;
      }
    }
  }

  return 0;
}
