//
// Created by Nishanth on 5/26/2026.
//

#ifndef ECS36BBATTLESHIPWITHAI_AIPLAYER_H
#define ECS36BBATTLESHIPWITHAI_AIPLAYER_H

#include "Player.h"
#include <deque>
#include <random>
#include <vector>

namespace BattleShip {

    class AIPlayer : public Player {
    public:
        AIPlayer(const std::string& name, const GameConfig& game_config);
        static void seed_rng(unsigned int seed);
        void place_ships(std::istream& in, std::ostream& out) override;

    protected:
        static std::mt19937 rng_;

    private:
        void place_ship_randomly(char ship_name, int ship_length);
    };

    class CheatingAI : public AIPlayer {
    public:
        CheatingAI(const std::string& name, const GameConfig& game_config);
        std::pair<int, int> get_firing_location(std::istream& in, std::ostream& out) override;
    };

    class RandomAI : public AIPlayer {
    public:
        RandomAI(const std::string& name, const GameConfig& game_config);
        void place_ships(std::istream& in, std::ostream& out) override;
        std::pair<int, int> get_firing_location(std::istream& in, std::ostream& out) override;

    protected:
        std::vector<std::pair<int, int>> random_locations_;
        void remove_from_random_pool(int row, int col);
    };

    class SearchAndDestroyAI : public RandomAI {
    public:
        SearchAndDestroyAI(const std::string& name, const GameConfig& game_config);
        std::pair<int, int> get_firing_location(std::istream& in, std::ostream& out) override;
        void on_hit_at(int row, int col) override;

    private:
        std::deque<std::pair<int, int>> destroy_queue_;
        void add_neighbors(int row, int col);
        bool is_in_destroy_queue(int row, int col) const;
    };

}

#endif //ECS36BBATTLESHIPWITHAI_AIPLAYER_H
