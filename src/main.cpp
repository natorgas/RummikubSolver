#include <iostream>
#include "GameTypes.hpp"
#include "TilesBag.hpp"

int main() {

  Tile t(1, Color::Red);

  TilesBag tb;

  t.print();

  tb.remove_tile(t);
  tb.remove_tile(t);

  std::cout << tb.n_tiles_left(t) << std::endl;

  return 0;
}
