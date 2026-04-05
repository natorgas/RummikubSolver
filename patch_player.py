with open('src/Player.cpp', 'r') as f:
    content = f.read()

# Fix play_turn: Remove get_tile_id lambda and use TileMap's index method
old_play_turn_decl = """  auto get_tile_id = [](const Tile& t) {
      if (t.isJoker) return 52;
      return (t.value - 1) * 4 + static_cast<int>(t.color);
  };
  std::vector<std::vector<int>> setsContainingTileFast(53);
  for (size_t i = 0; i < allSets.size(); ++i) {
      for (const Tile& t : allSets[i].tiles) {
          int id = get_tile_id(t);
          if (std::find(setsContainingTileFast[id].begin(), setsContainingTileFast[id].end(), i) == setsContainingTileFast[id].end()) {
              setsContainingTileFast[id].push_back(i);
          }
      }
  }"""
new_play_turn_decl = """  TileMap tileMapHelper;
  std::vector<std::vector<int>> setsContainingTileFast(53);
  for (size_t i = 0; i < allSets.size(); ++i) {
      for (const Tile& t : allSets[i].tiles) {
          int id = tileMapHelper.index(t);
          if (std::find(setsContainingTileFast[id].begin(), setsContainingTileFast[id].end(), i) == setsContainingTileFast[id].end()) {
              setsContainingTileFast[id].push_back(i);
          }
      }
  }"""
content = content.replace(old_play_turn_decl, new_play_turn_decl)

# Fix find_best_move: Remove get_tile_id lambda and use TileMap's index method
old_find_best_move_logic = """    auto get_tile_id = [](const Tile& t) {
      if (t.isJoker) return 52;
      return (t.value - 1) * 4 + static_cast<int>(t.color);
    };

    for (int val = MIN_TILE_VALUE; val <= MAX_TILE_VALUE; ++val) {"""
new_find_best_move_logic = """    TileMap tileMapHelper;

    for (int val = MIN_TILE_VALUE; val <= MAX_TILE_VALUE; ++val) {"""
content = content.replace(old_find_best_move_logic, new_find_best_move_logic)

# Update get_tile_id(X) calls to tileMapHelper.index(X)
content = content.replace("get_tile_id(t)", "tileMapHelper.index(t)")
content = content.replace("get_tile_id(joker)", "tileMapHelper.index(joker)")
content = content.replace("get_tile_id(firstUnplaced)", "tileMapHelper.index(firstUnplaced)")

with open('src/Player.cpp', 'w') as f:
    f.write(content)
