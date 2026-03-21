#include "GameTypes.hpp"
#include "Player.hpp"
#include "TilesBag.hpp"

int main() {

  Board board;

  TilesBag bag;

  HumanPlayer p("Human");

  p.play_turn(board, bag);

  return 0;
}
