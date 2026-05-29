//
// Created by mfbut on 4/24/2026.
//

#ifndef ECS36BBATTLESHIPHUMANSONLY_PLAYER_H
#define ECS36BBATTLESHIPHUMANSONLY_PLAYER_H

#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Board.h"
#include "FiringResult.h"
#include "GameConfig.h"
#include "Orientation.h"
#include "ShipPlacement.h"

namespace BattleShip {
    /**
     * Represents a player in the game of battleship
     */
    class Player {
    public:
        Player(const std::string& name, const Board& board,
               const std::map<char, int>& ship_healths, Player* opponent);

        virtual ~Player() = default;

        virtual void on_hit_at(int /*row*/, int /*col*/) {}

        //getters

        [[nodiscard]] const std::string& name() const;

        [[nodiscard]] const Board& board() const;

        [[nodiscard]] Board& board();

        [[nodiscard]] Player& opponent();

        [[nodiscard]] const Player& opponent() const;

        //setters

        void set_opponent(Player& opponent);

        //player actions

        virtual void place_ships(std::istream& in, std::ostream& out) = 0;

        virtual std::pair<int, int> get_firing_location(std::istream& in, std::ostream& out) = 0;

        FiringResult receive_fire_at(int row, int col);

        //checks

        [[nodiscard]] bool are_all_ships_destroyed() const;

    protected:

        //members

        std::string name_;
        Board board_;
        std::map<char, int> ship_healths_;
        Player* opponent_;
    };

    class HumanPlayer : public Player {
    public:
        HumanPlayer(const GameConfig& game_config, std::istream& in, std::ostream& out,
                    const std::vector<std::unique_ptr<Player> >& other_players);

        void place_ships(std::istream& in, std::ostream& out) override;

        std::pair<int, int> get_firing_location(std::istream& in, std::ostream& out) override;

    private:

        //placing ships

        void place_ship(char ship_name, int ship_length, std::istream& in = std::cin, std::ostream& out = std::cout);

        [[nodiscard]] std::optional<Orientation> get_orientation(char ship_name, std::istream& in,
                                                                 std::ostream& out) const;

        [[nodiscard]] std::optional<ShipPlacement> get_placement(const char ship_name, int ship_length,
                                                                 Orientation orientation, std::istream& in,
                                                                 std::ostream& out) const;

        //helper for getting input in the form row col

        [[nodiscard]] std::optional<std::pair<int, int> > get_row_and_column(
            const std::string& prompt, std::istream& in, std::ostream& out) const;
    };
} // BattleShip

#endif //ECS36BBATTLESHIPHUMANSONLY_PLAYER_H
