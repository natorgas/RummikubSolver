#include "Player.hpp"
#include <string>

Player::Player(std::string nme) : name(nme), hand({}), hasMadeFirstMove(false) {}

std::string Player::get_name() const { return name; }
