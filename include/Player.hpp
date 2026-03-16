#ifndef PLAYER_H
#define PLAYER_H
#include <string>
#include "GameTypes.hpp"

/********************* Player *******************/

class Player {
  public:
    Player() = delete;
    Player(std::string nme);

    std::string get_name() const;

  private:
    std::string name;
    std::vector<Tile> hand;
    bool hasMadeFirstMove;
};

/************************************************/

#endif
