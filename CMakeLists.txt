cmake_minimum_required(VERSION 3.15)

project(Rummikub)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(Rummikub
  # Source files
  src/main.cpp
  src/GameTypes.cpp
  src/Player.cpp
  src/TilesBag.cpp
  src/MainWindow.cpp
  src/InitialDrawDialog.cpp
  
  # Header files
  include/Constants.hpp
  include/GameTypes.hpp
  include/InitialDrawDialog.hpp
  include/MainWindow.hpp
  include/Player.hpp
  include/TileMap.hpp
  include/TilesBag.hpp
  include/Utils.hpp
)

target_include_directories(Rummikub PRIVATE include)
target_link_libraries(Rummikub PRIVATE Qt6::Widgets)

