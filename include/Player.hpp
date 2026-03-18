#ifndef PLAYER_H
#define PLAYER_H
#include <string>
#include <vector>
#include "GameTypes.hpp"

/********************* Player *******************/

class Player {
  public:
    Player() = delete;
    Player(std::string nme);

    std::string get_name() const;

    virtual ~Player() = default;

    virtual void play_turn(Board& board) = 0;

  private:
    std::string name;
    bool hasMadeFirstMove;
    int n_owned_tiles;
};

/***********************************************/


/********************* Player -> Human *******************/

class HumanPlayer : public Player {
  public:
    HumanPlayer() = delete;
    HumanPlayer(std::string nme) : Player(nme) {}

    // Sequence of moves making up a turn
    void play_turn(Board& board) override;

  private:
    // Individual moves sucha as creating a group
    bool make_move(Board& board);

    void create_group(Board& board);
};

/*********************************************************/


/********************* Player -> AI *******************/

class AIPlayer: public Player {
  public:
    void play_turn(Board& board) override;

  private:
    std::vector<Tile> hand;
};

/******************************************************/

#endif
