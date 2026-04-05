#ifndef PLAYER_H
#define PLAYER_H
#include <string>
#include <vector>
#include <chrono>
#include "TileMap.hpp"
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

    void inital_draw(TilesBag& bag);

    bool placed_all_tiles() const;

  protected:
    void increase_tiles(int n);

    void decrease_tiles(int n);

    bool made_first_move() const;

    void make_first_move();

    int n_owned_tiles() const;

  private:
    virtual bool draw_tile(TilesBag& bag) = 0;
    
    std::string name;
    bool hasMadeFirstMove;
    int nOwnedTiles;
};

/***********************************************/


/********************* Player -> Human *******************/

class HumanPlayer : public Player {
  public:
    HumanPlayer() = delete;

    HumanPlayer(std::string nme);

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
    AIPlayer(std::string nme);

    void play_turn(Board& board, TilesBag& bag) override;

    void set_hand(const std::vector<Tile>& v); // DEBUG

  private:
    bool draw_tile(TilesBag& bag) override;

    bool find_best_move(
        const std::vector<Set>& allSets, 
        const int               initialBoardSize,
        TileMap&                boardTiles,
        TileMap&                allTiles,
        std::vector<int>&       setIndexToUseFreq,
        std::vector<int>&       bestSetIndexToUseFreq,
        int&                    maxHandTilesUsed,
        const int               allSetsIndex,
        const std::chrono::high_resolution_clock::time_point& startTime,
        const int               initialBoardValSum,
        int                     unplacedBoardTiles,
        int                     nTilesOnBoardCount,
        int                     currentBoardValSum,
        const TileSetsMap&      setsContainingTileFast
    );

    std::vector<Tile> hand;

    // Used for controlling time used by find_best_move
    // unsigned for defined overflow
    unsigned int stepCounter;
};

/******************************************************/

#endif
