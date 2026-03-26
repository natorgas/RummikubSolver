#include "GameTypes.hpp"
#include "Player.hpp"
#include "TilesBag.hpp"

int main() {

  Board board;
  
  TilesBag bag;

  // HumanPlayer p("Human");
  AIPlayer d("Dave");

  // p.inital_draw(bag);
  d.inital_draw(bag);

  d.play_turn(board, bag);

  return 0;
}
