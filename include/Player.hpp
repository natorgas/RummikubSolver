#ifndef PLAYER_H
#define PLAYER_H
#include <string>
#include <vector>
#include "GameTypes.hpp"
#include "TilesBag.hpp"

/********************* Player *******************/

class Player {
  public:
    Player() = delete;
    Player(std::string nme);

    std::string get_name() const;

    virtual ~Player() = default;

    virtual void play_turn(Board& board, TilesBag& bag) = 0;

  private:
    virtual bool draw_tile(TilesBag& bag) = 0;
    
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
    void play_turn(Board& board, TilesBag& bag) override;

  private:
    bool draw_tile(TilesBag &bag) override;

    // Individual moves such as creating a group
    bool make_move(Board& board, TilesBag& bag);

    bool create_group(Board& board);

    bool create_run(Board& board);

    bool remove_group(Board& board);

    bool remove_run(Board& board);
};

/*********************************************************/


/********************* Player -> AI *******************/

class AIPlayer: public Player {
  public:
    void play_turn(Board& board, TilesBag& bag) override;

  private:
    bool draw_tile(TilesBag& bag) override;

    std::vector<Tile> hand;
};

/******************************************************/

#endif
