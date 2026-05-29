//
// Created by mfbut on 4/24/2026.
//

#include "Game.h"

#include <algorithm>
#include <sstream>

namespace BattleShip {
    /**
     * Create a game based on the game configuration
     * @param game_config : The configuration for this game
     * @param in : the stream to get input from
     * @param out : the stream to display output to
     */
    Game::Game(const GameConfig& game_config, std::istream& in, std::ostream& out) : players_(), cur_player_index_(0),
        in_(in), out_(out) {
        int mode = 0;
        while (mode < 1 || mode > 3) {
            out << "Select game mode:\n1. Human vs Human\n2. Human vs AI\n3. AI vs AI\nYour choice: ";
            std::string line; std::getline(in, line);
            std::istringstream ss(line); ss >> mode;
        }

        int ai_count = 0;
        if (mode == 1) {
            for (int i = 0; i < 2; ++i) {
                players_.push_back(std::make_unique<HumanPlayer>(game_config, in, out, players_));
                players_.at(i)->place_ships(in_, out_);
            }
        } else if (mode == 2) {
            players_.push_back(std::make_unique<HumanPlayer>(game_config, in, out, players_));
            players_.at(0)->place_ships(in_, out_);
            players_.push_back(make_ai(game_config, ++ai_count, in, out));
            players_.at(1)->place_ships(in_, out_);
        } else {
            players_.push_back(make_ai(game_config, ++ai_count, in, out));
            players_.at(0)->place_ships(in_, out_);
            players_.push_back(make_ai(game_config, ++ai_count, in, out));
            players_.at(1)->place_ships(in_, out_);
        }

        for (int i = 0; i < 2; ++i) {
            players_.at(i)->set_opponent(*players_.at((i + 1) % players_.size()));
        }
    }

    /**
     * Play a game of Battleship
     */
    void Game::play() {
        while (true) {
            auto firing_location = cur_player().get_firing_location(in_, out_);
            resolve_current_player_action(firing_location);
            if (is_game_over()) {
                announce_results();
                return;
            }
            advance_to_next_player();
        }
    }


    /**
     * Checks if the game is over. The game is over if any one player has all
     * of their ships destroyed.
     * @return : whether the game is over or not
     */
    bool Game::is_game_over() const {
        constexpr auto is_loser = [](const auto& player) { return player->are_all_ships_destroyed(); };
        return std::ranges::any_of(players_, is_loser);
    }


    /**
     * Resolve the current player shooting their opponent at firing_location
     * @param firing_location : the row,col specifying the location the
     */
    void Game::resolve_current_player_action(const std::pair<int, int>& firing_location) {
        const auto& [row,col] = firing_location;
        const auto firing_result = cur_player().opponent().receive_fire_at(row, col);

        out_ << std::format("{}'s Firing Board\n{}\n\n", cur_player().name(),
                            cur_player().opponent().board().view_as_hidden());
        out_ << std::format("{}'s Placement Board\n{}\n", cur_player().name(), cur_player().board().view_as_visible());


        if (firing_result.is_hit()) {
            out_ << std::format("{} hit {}'s {}!\n",
                                cur_player().name(),
                                cur_player().opponent().name(), firing_result.get_ship_hit().value()) << std::endl;
            if (firing_result.was_ship_destroyed()) {
                out_ << std::format("{} destroyed {}'s {}!\n",
                                    cur_player().name(),
                                    cur_player().opponent().name(), firing_result.get_ship_hit().value()
                );
            }
            cur_player().on_hit_at(row, col);
        } else {
            out_ << "Missed." << std::endl;
        }
    }

    /**
     * Switch to the next player's turn
     */
    void Game::advance_to_next_player() {
        cur_player_index_ = (cur_player_index_ + 1) % players_.size();
    }

    /**
     * Announce the final results of the game
     */
    void Game::announce_results() const {
        out_ << cur_player().name() << " won the game!" << std::endl;
    }

    /**
     * Get the current player
     * @return : The current player
     */
    Player& Game::cur_player() {
        return *players_.at(cur_player_index_);
    }

    /**
     * Get the current player
     * @return : The current player
     */
    const Player& Game::cur_player() const {
        return *players_.at(cur_player_index_);
    }

    std::unique_ptr<Player> Game::make_ai(const GameConfig& game_config, int ai_number,
                                           std::istream& in, std::ostream& out) {
        std::string ai_name = "AI " + std::to_string(ai_number);
        int choice = 0;
        while (choice < 1 || choice > 3) {
            out << "Select AI type:\n1. Cheating AI\n2. Random AI\n3. Search and Destroy AI\nYour choice: ";
            std::string line; std::getline(in, line);
            std::istringstream ss(line); ss >> choice;
        }
        if (choice == 1) return std::make_unique<CheatingAI>(ai_name, game_config);
        if (choice == 2) return std::make_unique<RandomAI>(ai_name, game_config);
        return std::make_unique<SearchAndDestroyAI>(ai_name, game_config);
    }
} // BattleShip
