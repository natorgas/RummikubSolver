#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "GameTypes.hpp"
#include <array>

const int MAX_TILE_VALUE = 13;
const int MIN_TILE_VALUE = 1;
const int NUM_COLORS = 4;
const int INDIVIDUAL_TILE_FREQ = 2;
const int INITIAL_N_OWNED_TILES = 14;
const int MIN_SET_SIZE = 3;
const int MAX_GROUP_SIZE = 4;
const std::array<Color, NUM_COLORS> ALL_COLORS = {Color::Black, 
                                                  Color::Red,
                                                  Color::Blue,
                                                  Color::Orange};

#endif
