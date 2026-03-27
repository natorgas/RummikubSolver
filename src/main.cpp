#include "GameTypes.hpp"
#include "Player.hpp"
#include "TilesBag.hpp"
#include "Constants.hpp"
#include "Utils.hpp"
#include <iostream>
#include <memory>

int main() {

  Board board;
  
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

  std::cout << "Values go from " << MIN_TILE_VALUE << " to " << MAX_TILE_VALUE << ".\n";
  std::cout << "Colors are: ";
  for (const Color& c : ALL_COLORS) {
    std::cout << color_to_str(c) << " ";
  }
  std::cout << std::endl;
  std::cout << "There is NO case-sensitivity for user input of type string.\n\n";

  board.add_set(Set(SetType::Run, {Tile(6, Color::Red, true),
        Tile(7, Color::Red),
        Tile(8, Color::Red),
        Tile(9, Color::Red)}));

  HumanPlayer p("Ivo");
  p.inital_draw(bag);
  p.play_turn(board, bag);
  p.play_turn(board, bag);

  // Let everyone draw their initial tiles
  for (auto& player_p : players) {
    player_p->inital_draw(bag);
  }

  // while (!have_winner(players)) {
  //   for (auto& player_p : players) {
  //     player_p->play_turn(board,  bag);
  //   }
  // }

  std::cout << "'The game is over' -Babu\n";

  return 0;
}
