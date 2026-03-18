#include <iostream>
#include <vector>
#include "GameTypes.hpp"
#include "Player.hpp"
#include "TilesBag.hpp"

int main() {

  Board board;

  std::vector<Tile> v = {Tile(1, Color::Red), Tile(2, Color::Red), Tile(3, Color::Red)};

  Set s(SetType::Run, v);

  board.add_set(s);

  board.print();

  HumanPlayer p("Human");

  p.play_turn(board);

  return 0;
}
