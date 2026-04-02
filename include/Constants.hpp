#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include "GameTypes.hpp"
#include <array>

constexpr int MAX_TILE_VALUE = 13;
constexpr int MIN_TILE_VALUE = 1;
constexpr int NUM_COLORS = 4;
constexpr int INDIVIDUAL_TILE_FREQ = 2;
constexpr int INITIAL_N_OWNED_TILES = 14;
constexpr int MIN_SET_SIZE = 3;
constexpr int MAX_GROUP_SIZE = 4;
constexpr int MIN_FIRST_MOVE_SUM = 30;
constexpr double TIME_LIMIT = 10.0;
constexpr int N_DIFF_TILES = MAX_TILE_VALUE * NUM_COLORS + INDIVIDUAL_TILE_FREQ; // numbers * colors + jokers
constexpr std::array<Color, NUM_COLORS> ALL_COLORS = {Color::Black, 
                                                  Color::Red,
                                                  Color::Blue,
                                                  Color::Orange};

#endif
