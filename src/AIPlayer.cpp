//
// Created by Nishanth on 5/26/2026.
//

#include "AIPlayer.h"
#include <algorithm>
#include <format>
#include <ranges>

namespace BattleShip {
    std::mt19937 AIPlayer::rng_;
    void AIPlayer::seed_rng(unsigned int seed) { rng_.seed(seed); }
    AIPlayer::AIPlayer(const std::string& name, const GameConfig& game_config)
        : Player(name, Board(game_config.board_num_rows(), game_config.board_num_cols()),
                 game_config.ship_healths(), nullptr) {}
    void AIPlayer::place_ships(std::istream&, std::ostream& out) {
        for (const auto [ship_name, ship_health] : ship_healths_) {
            out << board_.view_as_visible() << std::endl;
            place_ship_randomly(ship_name, ship_health);
            out << board_.view_as_visible() << std::endl;
        }
    }
    void AIPlayer::place_ship_randomly(char ship_name, int ship_length) {
        int num_rows = static_cast<int>(board_.num_rows());
        int num_cols = static_cast<int>(board_.num_cols());
        while (true) {
            std::uniform_int_distribution<int> orient_dist(0, 1);
            bool horizontal = (orient_dist(rng_) == 0);
            Orientation orientation = horizontal ? Orientation::HORIZONTAL : Orientation::VERTICAL;

            std::uniform_int_distribution<int> row_dist(
                0, horizontal ? num_rows - 1 : num_rows - ship_length);
            std::uniform_int_distribution<int> col_dist(
                0, horizontal ? num_cols - ship_length : num_cols - 1);

            int row = row_dist(rng_);
            int col = col_dist(rng_);

            ShipPlacement placement(ship_name, ship_length, row, col, orientation);
            if (board_.can_ship_be_placed(placement)) {
                board_.place_ship(placement);
                return;
            }
        }
    }
    CheatingAI::CheatingAI(const std::string& name, const GameConfig& game_config)
        : AIPlayer(name, game_config) {}
    std::pair<int, int> CheatingAI::get_firing_location(std::istream&, std::ostream& out) {
        out << std::format("{}'s Firing Board\n{}\n\n", name(), opponent().board().view_as_hidden());
        out << std::format("{}'s Placement Board\n{}\n", name(), board_.view_as_visible());
        const Board& opp = opponent().board();
        for (int r = 0; r < static_cast<int>(opp.num_rows()); r++)
            for (int c = 0; c < static_cast<int>(opp.num_cols()); c++)
                if (opp.has_ship_at(r, c) && !opp.has_been_fired_at(r, c))
                    return {r, c};
        for (int r = 0; r < static_cast<int>(opp.num_rows()); r++)
            for (int c = 0; c < static_cast<int>(opp.num_cols()); c++)
                if (!opp.has_been_fired_at(r, c)) return {r, c};
        return {0, 0};
    }
    RandomAI::RandomAI(const std::string& name, const GameConfig& game_config)
        : AIPlayer(name, game_config)
    {
        for (int r = 0; r < game_config.board_num_rows(); r++)
            for (int c = 0; c < game_config.board_num_cols(); c++)
                random_locations_.push_back({r, c});
    }
    void RandomAI::place_ships(std::istream& in, std::ostream& out) {
        AIPlayer::place_ships(in, out);
        std::ranges::shuffle(random_locations_, rng_);
    }
    std::pair<int, int> RandomAI::get_firing_location(std::istream&, std::ostream& out) {
        out << std::format("{}'s Firing Board\n{}\n\n", name(), opponent().board().view_as_hidden());
        out << std::format("{}'s Placement Board\n{}\n", name(), board_.view_as_visible());
        auto loc = random_locations_.back();
        random_locations_.pop_back();
        return loc;
    }
    void RandomAI::remove_from_random_pool(int row, int col) {
        std::erase_if(random_locations_,
                      [row, col](const auto& p) { return p.first == row && p.second == col; });
    }
    SearchAndDestroyAI::SearchAndDestroyAI(const std::string& name, const GameConfig& game_config)
        : RandomAI(name, game_config) {}

    std::pair<int, int> SearchAndDestroyAI::get_firing_location(std::istream&, std::ostream& out) {
        out << std::format("{}'s Firing Board\n{}\n\n", name(), opponent().board().view_as_hidden());
        out << std::format("{}'s Placement Board\n{}\n", name(), board_.view_as_visible());
        if (destroy_queue_.empty()) {
            auto loc = random_locations_.back();
            random_locations_.pop_back();
            return loc;
        }
        auto loc = destroy_queue_.front();
        destroy_queue_.pop_front();
        return loc;
    }
    void SearchAndDestroyAI::on_hit_at(int row, int col) {
        add_neighbors(row, col);
    }
    bool SearchAndDestroyAI::is_in_destroy_queue(int row, int col) const {
        for (const auto& [r, c] : destroy_queue_)
            if (r == row && c == col) return true;
        return false;
    }
    void SearchAndDestroyAI::add_neighbors(int row, int col) {
        const std::vector<std::pair<int,int>> dirs = {{0,-1},{-1,0},{0,1},{1,0}};
        const Board& opp = opponent().board();
        for (auto [dr, dc] : dirs) {
            int nr = row + dr, nc = col + dc;
            if (!opp.is_on(nr, nc)) continue;
            if (opp.has_been_fired_at(nr, nc)) continue;
            if (is_in_destroy_queue(nr, nc)) continue;
            remove_from_random_pool(nr, nc);
            destroy_queue_.push_back({nr, nc});
        }
    }
}