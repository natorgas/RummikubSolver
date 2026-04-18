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

  // Ask user how much time they are willing to wait for answer of AI
  std::cout << "Wie lang magsch warte? (in seconds): ";
  std::cin >> TIME_LIMIT;

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
      player_p->play_turn(board, bag);
      if (player_p->placed_all_tiles()) {
        std::cout << player_p->get_name() << " wins. Congrats!\n"
                  << "'The game is over' -Babu\n";
        gameOver = true;
        break;
      }
    }
  }

  return 0;
}
