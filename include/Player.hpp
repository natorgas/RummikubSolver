#ifndef PLAYER_H
#define PLAYER_H
#include <string>
#include <vector>
#include <chrono>
#include <functional>
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
    virtual bool draw_tile(TilesBag& bag) = 0;
    virtual int n_owned_tiles() const;

    const std::vector<Tile>& get_hand() const;
    void set_progress_callback(std::function<void(std::string)> cb);
    void add_to_hand(const Tile& t);
    void set_hand(const std::vector<Tile>& h);
    void inital_draw(TilesBag& bag);
    bool placed_all_tiles() const;
    bool made_first_move() const;
    void make_first_move();

  protected:
    std::vector<Tile> hand;
    std::function<void(std::string)> progressCallback = nullptr;

  private:
    std::string name;
    bool hasMadeFirstMove;
};

/***********************************************/


/********************* Player -> Human *******************/

class HumanPlayer : public Player {
  public:
    HumanPlayer() = delete;
    HumanPlayer(std::string nme);

    // Sequence of moves making up a turn
    void play_turn(Board& board, TilesBag& bag) override;
    bool draw_tile(TilesBag &bag) override;
    int n_owned_tiles() const override;
    void decrease_tiles(int amount);

  private:
    int nOwnedTiles;
};

/*********************************************************/


/********************* Player -> AI *******************/

class AIPlayer: public Player {
  public:
    AIPlayer(std::string nme);

    void play_turn(Board& board, TilesBag& bag) override;
    bool draw_tile(TilesBag& bag) override;

  private:
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

    // Used for controlling time used by find_best_move
    // unsigned for defined overflow
    unsigned int stepCounter;
};

/******************************************************/

#endif
