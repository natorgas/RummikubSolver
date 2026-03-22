#include "GameTypes.hpp"
#include "Player.hpp"
#include "TilesBag.hpp"
#include <iostream>

int main() {

  Board board;

  board.add_set(Set(SetType::Group, {Tile(3, Color::Red),
                                     Tile(3, Color::Black),
                                     Tile(3, Color::Orange),
                                     Tile(3, Color::Blue)}));

  TilesBag bag;

  HumanPlayer p("Human");
  AIPlayer d("Dave");

  p.inital_draw(bag);
  // d.inital_draw(bag);

  d.play_turn(board, bag);

  return 0;
}
