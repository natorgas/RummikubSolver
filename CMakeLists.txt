cmake_minimum_required(VERSION 3.15)

project(Rummikub)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(main
  src/main.cpp
  src/GameTypes.cpp
  src/Player.cpp
  src/TilesBag.cpp
  src/TileMap.cpp
)

