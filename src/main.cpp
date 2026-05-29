#include <fstream>
#include <iostream>
#include <print>

#include "GameConfig.h"
#include "Game.h"
#include "Player.h"
#include "AIPlayer.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::println("Usage ./BattleShipExe PathToConfigFile");
    }

    unsigned int seed = (argc >= 3) ? std::stoul(argv[2])
                                    : static_cast<unsigned int>(std::time(nullptr));

    BattleShip::AIPlayer::seed_rng(seed);

    std::ifstream config_file(argv[1]);
    BattleShip::GameConfig game_config(config_file);
    BattleShip::Game game(game_config, std::cin, std::cout);
    game.play();
    return 0;
}
