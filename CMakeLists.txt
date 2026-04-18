cmake_minimum_required(VERSION 3.15)

project(Rummikub)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(main
  src/main.cpp
  src/GameTypes.cpp
  src/Player.cpp
  src/TilesBag.cpp
  src/MainWindow.cpp
  include/MainWindow.hpp
  src/InitialDrawDialog.cpp
  include/InitialDrawDialog.hpp
)

target_include_directories(main PRIVATE include)
target_link_libraries(main PRIVATE Qt6::Widgets)

