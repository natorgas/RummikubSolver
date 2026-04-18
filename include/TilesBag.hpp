#ifndef TILES_BAG_HPP
#define TILES_BAG_HPP

class TilesBag {
  public:
    TilesBag();

    bool is_empty() const;

    // Returns the actual number of tiles left inside the bag (info available to everyone in the game)
    int n_tiles_left_total() const;

    // Decreases number of tiles inside the bag (called whenever someone draws from bag)
    void draw();

  private:
    int totalTilesLeft;
};

#endif
