import re

with open('src/Player.cpp', 'r') as f:
    content = f.read()

# Replace play_turn setup
old_play_turn = """  TileMap tileMapHelper;
  std::vector<std::vector<int>> setsContainingTileFast(N_DIFF_TILES);
  for (size_t i = 0; i < allSets.size(); ++i) {
      for (const Tile& t : allSets[i].tiles) {
          int id = tileMapHelper.index(t);
          if (std::find(setsContainingTileFast[id].begin(), setsContainingTileFast[id].end(), i) == setsContainingTileFast[id].end()) {
              setsContainingTileFast[id].push_back(i);
          }
      }
  }"""
new_play_turn = """  TileSetsMap setsContainingTileFast;
  for (size_t i = 0; i < allSets.size(); ++i) {
      for (const Tile& t : allSets[i].tiles) {
          if (std::find(setsContainingTileFast[t].begin(), setsContainingTileFast[t].end(), i) == setsContainingTileFast[t].end()) {
              setsContainingTileFast[t].push_back(i);
          }
      }
  }"""
content = content.replace(old_play_turn, new_play_turn)

# Replace find_best_move signature
old_sig = """    int                     currentBoardValSum,
    const std::vector<std::vector<int>>& setsContainingTileFast) {"""
new_sig = """    int                     currentBoardValSum,
    const TileSetsMap&      setsContainingTileFast) {"""
content = content.replace(old_sig, new_sig)

# Replace tileMapHelper declaration
old_helper = """    TileMap tileMapHelper;

    for (int val = MIN_TILE_VALUE; val <= MAX_TILE_VALUE; ++val) {"""
new_helper = """    for (int val = MIN_TILE_VALUE; val <= MAX_TILE_VALUE; ++val) {"""
content = content.replace(old_helper, new_helper)

# Replace usage of index
content = content.replace("setsContainingTileFast[tileMapHelper.index(t)]", "setsContainingTileFast[t]")
content = content.replace("setsContainingTileFast[tileMapHelper.index(joker)]", "setsContainingTileFast[joker]")
content = content.replace("setsContainingTileFast[tileMapHelper.index(firstUnplaced)]", "setsContainingTileFast[firstUnplaced]")

with open('src/Player.cpp', 'w') as f:
    f.write(content)
