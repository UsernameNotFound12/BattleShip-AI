//
// Created by mfbut on 4/24/2026.
//

#ifndef ECS36BBATTLESHIPHUMANSONLY_GAME_H
#define ECS36BBATTLESHIPHUMANSONLY_GAME_H

#include <vector>
#include <iostream>

#include "GameConfig.h"
#include "Player.h"
#include "AIPlayer.h"

#include <memory>

namespace BattleShip {
    /**
     * Represents a game of battleship
     */
    class Game {
    public:
        explicit Game(const GameConfig& game_config, std::istream& in, std::ostream& out);

        //playing the game

        void play();

        [[nodiscard]] bool is_game_over() const;

        void resolve_current_player_action(const std::pair<int, int>& firing_location);

        void advance_to_next_player();

        void announce_results() const;


        //get the current player

        [[nodiscard]] Player& cur_player();

        [[nodiscard]] const Player& cur_player() const;

    private:
        std::vector<std::unique_ptr<Player> > players_;
        size_t cur_player_index_;
        std::istream& in_; // the stream to get input from
        std::ostream& out_; // the stream to display output to

        std::unique_ptr<Player> make_ai(const GameConfig& game_config, int ai_number,
                                         std::istream& in, std::ostream& out);
    };
} // BattleShip

#endif //ECS36BBATTLESHIPHUMANSONLY_GAME_H
