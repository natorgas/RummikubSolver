with open('src/Player.cpp', 'r') as f:
    content = f.read()

start_idx = content.find("bool AIPlayer::find_best_move(")
end_idx = content.find("void AIPlayer::set_hand(")

new_func = """bool AIPlayer::find_best_move(
    const std::vector<Set>& allSets, 
    const int               initialBoardSize,
    TileMap&                boardTiles,
    TileMap&                availableTiles,
    std::vector<int>&       setIndexToUseFreq,
    std::vector<int>&       bestSetIndexToUseFreq,
    int&                    maxHandTilesUsed,
    const int               allSetsIndex,
    const std::chrono::high_resolution_clock::time_point& startTime,
    const int               initialBoardValSum,
    int                     unplacedBoardTiles,
    int                     nTilesOnBoardCount,
    int                     currentBoardValSum,
    const std::vector<std::vector<int>>& setsContainingTileFast) {

  if (++stepCounter % 1000 == 0) {
    auto now = std::chrono::high_resolution_clock::now();
    if (now - startTime > std::chrono::duration<double>(TIME_LIMIT)) {
      std::cout << "Exceeded time limit of " << TIME_LIMIT << " seconds.\\n";
      return true;
    }
  }

  if (unplacedBoardTiles == 0) {
    const int nHandTilesUsed = nTilesOnBoardCount - initialBoardSize;
    assert(nHandTilesUsed >= 0);

    bool isValidMove = true;
    if (!made_first_move() && (currentBoardValSum - initialBoardValSum < MIN_FIRST_MOVE_SUM)) {
      isValidMove = false;
    }

    if (isValidMove && nHandTilesUsed > maxHandTilesUsed) {
      maxHandTilesUsed = nHandTilesUsed;
      bestSetIndexToUseFreq = setIndexToUseFreq;
      std::cout << "Can place " << maxHandTilesUsed << std::endl;
      if (maxHandTilesUsed == n_owned_tiles()) {
        std::cout << get_name() << " won!\\n";
        return true;
      }
    }
  }

  if (unplacedBoardTiles > 0) {
    Tile firstUnplaced(0, Color::None);
    int minSets = 1000000;

    auto get_tile_id = [](const Tile& t) {
      if (t.isJoker) return 52;
      return (t.value - 1) * 4 + static_cast<int>(t.color);
    };

    for (int val = MIN_TILE_VALUE; val <= MAX_TILE_VALUE; ++val) {
      for (Color c : ALL_COLORS) {
        Tile t(val, c);
        if (boardTiles[t] > 0) {
          int validSetsCount = 0;
          for (int i : setsContainingTileFast[get_tile_id(t)]) {
            if (set_can_be_placed(allSets[i], availableTiles)) {
              validSetsCount++;
            }
          }
          if (validSetsCount < minSets) {
            minSets = validSetsCount;
            firstUnplaced = t;
          }
        }
      }
    }
    
    Tile joker(0, Color::None, true);
    if (boardTiles[joker] > 0) {
      int validSetsCount = 0;
      for (int i : setsContainingTileFast[get_tile_id(joker)]) {
        if (set_can_be_placed(allSets[i], availableTiles)) {
          validSetsCount++;
        }
      }
      if (validSetsCount < minSets) {
        minSets = validSetsCount;
        firstUnplaced = joker;
      }
    }

    if (minSets == 0) return false;

    for (int i : setsContainingTileFast[get_tile_id(firstUnplaced)]) {
      const Set& trialSet = allSets[i];

      if (set_can_be_placed(trialSet, availableTiles)) {
        setIndexToUseFreq[i]++;
        std::vector<Tile> decrementedBoardTiles;
        decrementedBoardTiles.reserve(trialSet.size());
        
        int boardTilesDecremented = 0;
        int trialSetValSum = 0;

        for (const Tile& t : trialSet.tiles) {
          trialSetValSum += t.value;
          if (t.isJoker) {
            Tile jokerTile(0, Color::None, true);
            availableTiles[jokerTile]--;
            if (boardTiles[jokerTile] > 0) {
              boardTiles[jokerTile]--;
              decrementedBoardTiles.push_back(jokerTile);
              boardTilesDecremented++;
            }
          }
          else {
            availableTiles[t]--;
            if (boardTiles[t] > 0) {
              boardTiles[t]--;
              decrementedBoardTiles.push_back(t);
              boardTilesDecremented++;
            }
          }
        }

        if (find_best_move(allSets, initialBoardSize, boardTiles, availableTiles,
                           setIndexToUseFreq, bestSetIndexToUseFreq, maxHandTilesUsed, allSetsIndex, startTime, initialBoardValSum,
                           unplacedBoardTiles - boardTilesDecremented, nTilesOnBoardCount + trialSet.size(), currentBoardValSum + trialSetValSum, setsContainingTileFast)) {
          return true;
        }

        setIndexToUseFreq[i]--;
        for (const Tile& t : trialSet.tiles) {
          if (t.isJoker) {
            availableTiles[Tile(0, Color::None, true)]++;
          }
          else availableTiles[t]++;
        }
        for (const Tile& t : decrementedBoardTiles) {
          if (t.isJoker) {
            boardTiles[Tile(0, Color::None, true)]++;
          }
          else boardTiles[t]++;
        }
      }
    }
    return false;
  } 
  else {
    if (allSetsIndex >= allSets.size()) return false;

    for (size_t i = allSetsIndex; i < allSets.size(); ++i) {
      const Set& trialSet = allSets[i];

      if (set_can_be_placed(trialSet, availableTiles)) {
        setIndexToUseFreq[i]++;
        
        int trialSetValSum = 0;
        for (const Tile& t : trialSet.tiles) {
          trialSetValSum += t.value;
          if (t.isJoker) availableTiles[Tile(0, Color::None, true)]--;
          else availableTiles[t]--;
        }

        if (find_best_move(allSets, initialBoardSize, boardTiles, availableTiles,
                           setIndexToUseFreq, bestSetIndexToUseFreq, maxHandTilesUsed, i, startTime, initialBoardValSum,
                           unplacedBoardTiles, nTilesOnBoardCount + trialSet.size(), currentBoardValSum + trialSetValSum, setsContainingTileFast)) {
          return true;
        }

        setIndexToUseFreq[i]--;
        for (const Tile& t : trialSet.tiles) {
          if (t.isJoker) availableTiles[Tile(0, Color::None, true)]++;
          else availableTiles[t]++;
        }
      }
    }
    return false;
  }
}

"""

new_content = content[:start_idx] + new_func + content[end_idx:]

with open('src/Player.cpp', 'w') as f:
    f.write(new_content)
