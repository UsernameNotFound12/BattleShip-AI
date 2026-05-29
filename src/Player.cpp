//
// Created by mfbut on 4/24/2026.
//

#include "Player.h"

#include <algorithm>
#include <format>
#include <optional>
#include <ranges>
#include <sstream>
#include <unordered_map>

#include "ExtraStringMethods.h"

namespace BattleShip {
    /**
     * Create a player with a board and ships specified by
     * the game configuration but name and ship placements from in
     * The player has no opponent and has not placed their ships
     * @param game_config : the configuration of the game
     * @param in : the stream to read input from
     * @param out : the stream to display output to
     * @param other_players : the other players in the game
     */
    HumanPlayer::HumanPlayer(const GameConfig& game_config, std::istream& in, std::ostream& out,
                   const std::vector<std::unique_ptr<Player> >& other_players) : Player("",
        Board(game_config.board_num_rows(), game_config.board_num_cols()),
        game_config.ship_healths(), nullptr) {
        do {
            out << "Enter your name: ";
            std::getline(in, name_);
            name_ = strip(name_, " \t\n");
        } while (std::ranges::contains(other_players, name_,
                                       [](const auto& player) { return player->name(); })
        );
    }

    /**
     * Create a player with the specified information
     * Ships are still not placed
     * @param name : the name of the player
     * @param board : the board the player should start with a copy of
     * @param ship_healths : the ships the player should have
     * @param opponent : this player's opponent
     */
    Player::Player(const std::string& name,
                   const Board& board,
                   const std::map<char, int>& ship_healths,
                   Player* opponent) : name_(name), board_(board), ship_healths_(ship_healths),
                                       opponent_(opponent) {
    }

    /**
     * Get this player's name
     * @return : this player's name
     */
    const std::string& Player::name() const {
        return name_;
    }

    /**
     * Get this player's board
     * @return : this player's board
     */
    const Board& Player::board() const {
        return board_;
    }

    Board& Player::board() {
        return board_;
    }

    /**
     * Get this player's opponent
     * @return : this player's opponent
     */
    Player& Player::opponent() {
        return *opponent_;
    }

    /**
     * Get this player's opponent
     * @return : this player's opponent
     */
    const Player& Player::opponent() const {
        return *opponent_;
    }

    /**
     * Set this player's opponent to be the given player
     * @param opponent : this player's opponent
     */
    void Player::set_opponent(Player& opponent) {
        opponent_ = &opponent;
    }

    /**
     * Have this player place all of their ships
     * @param in : the stream to get input from
     * @param out : the stream to display output to
     */
    void HumanPlayer::place_ships(std::istream& in, std::ostream& out) {
        for (const auto [ship_name, ship_health]: ship_healths_) {
            out << board_.view_as_visible() << std::endl;
            place_ship(ship_name, ship_health, in, out);
            out << board_.view_as_visible() << std::endl;
        }
    }

    /**
     * Get a valid firing location from this player to shoot
     * @param in : the stream to get input from
     * @param out : the stream to display output to
     * @return : a valid location to shoot out in the form row, col
     */
    std::pair<int, int> HumanPlayer::get_firing_location(std::istream& in, std::ostream& out) {
        const std::string prompt = std::format(
            "{}, where would you like to fire?\n"
            "Enter your attack coordinate in the form row col:",
            name());

        out << std::format("{}'s Firing Board\n{}\n\n", name(), opponent().board().view_as_hidden());
        out << std::format("{}'s Placement Board\n{}\n", name(), board_.view_as_visible());
        while (true) {
            auto possible_row_and_col = get_row_and_column(prompt, in, out);
            if (not possible_row_and_col.has_value()) {
                continue; //ask again
            }
            auto [row,col] = possible_row_and_col.value();
            if (not board_.is_on(row, col)) {
                continue; //ask again
            }
            if (opponent().board().has_been_fired_at(row, col)) {
                continue; //ask again
            }
            return std::make_pair(row, col);
        }
    }

    /**
     * Receive fire at row, column
     * @param row : the row that was shot at
     * @param col : the column that was shot at
     * @return : the results of shooting at the location
     */
    FiringResult Player::receive_fire_at(int row, int col) {
        FiringResult firing_result;
        board_.mark_as_fired_at(row, col);
        if (board_.has_ship_at(row, col)) {
            auto ship = board_.get_ship_at(row, col).value();
            firing_result.set_ship_hit(ship);
            int remaining_health = (ship_healths_.at(ship) -= 1);
            if (remaining_health == 0) {
                firing_result.set_ship_was_destroyed();
            }
        }
        return firing_result;
    }

    /**
     * Check if all of this player's ships are destroyed
     * @return : whether all of this player's ships are destroyed or not
     */
    bool Player::are_all_ships_destroyed() const {
        return ship_healths_.size() == 0 or
               std::ranges::all_of(ship_healths_ | std::ranges::views::values,
                                   [](const auto& health) { return health == 0; }
               );
    }

    /**
     * Place the given ship on the board
     * @param ship_name : the name of ship to be placed
     * @param ship_length : how long the ship is
     * @param in : the stream to get input from
     * @param out : the stream to display output to
     */
    void HumanPlayer::place_ship(const char ship_name, const int ship_length, std::istream& in, std::ostream& out) {
        while (true) {
            auto orientation = get_orientation(ship_name, in, out);
            if (not orientation.has_value()) {
                continue;
            }
            const auto placement = get_placement(ship_name, ship_length, orientation.value(), in, out);
            if (placement.has_value()) {
                board_.place_ship(placement.value());
                return;
            }
        }
    }

    /**
     * Attempt to get an orientation for this
     * @param ship_name : the name of ship to get the orientation of
     * @param in : the stream to get input from
     * @param out : the stream to display output to
     * @return : an optional containing a valid orientation if one
     * was entered, otherwise an empty optional
     */
    std::optional<Orientation>
    HumanPlayer::get_orientation(const char ship_name, std::istream& in, std::ostream& out) const {
        static const std::unordered_map<std::string, Orientation> valid_input{
            {"horizontal", Orientation::HORIZONTAL},
            {"h", Orientation::HORIZONTAL},
            {"vertical", Orientation::VERTICAL},
            {"v", Orientation::VERTICAL}
        };

        out << std::format("{}, do you want to place {}, horizontally or vecrtically?\n"
                           "Enter h for horizontal or v for vertical\n"
                           "Your choice: ", name(), ship_name);

        std::string user_input;
        std::getline(in, user_input);
        user_input = lower(strip(user_input, " \t\n"));
        if (not valid_input.contains(user_input)) {
            return {}; // empty optional
        }

        return std::make_optional(valid_input.at(user_input));
    }

    /**
     * Get a potential placement for this ship
     * @param ship_name : the name of ship to get a placement for
     * @param ship_length : how long the ship is
     * @param orientation : the orientation of the ship
     * @param in : the stream to get input from
     * @param out : the stream to display output to
     * @return : an optional containing the placement for this ship if it
     * is valid, otherwise an empty optional
     */
    std::optional<ShipPlacement> HumanPlayer::get_placement(const char ship_name, const int ship_length,
                                                       Orientation orientation, std::istream& in,
                                                       std::ostream& out) const {
        std::string prompt = std::format("{}, enter the row and column you want to place {}, "
                                         "which is {} long, at with a space in between row and col: ",
                                         name(), ship_name, ship_length);
        auto possible_row_and_col = get_row_and_column(prompt, in, out);
        if (not possible_row_and_col.has_value()) {
            return {};
        }
        auto [start_row, start_col] = possible_row_and_col.value();

        ShipPlacement placement(ship_name, ship_length, start_row, start_col, orientation);
        if (not board_.can_ship_be_placed(placement)) {
            return {};
        }

        return std::make_optional(placement);
    }

    /**
     * Attempt to get input entered in the form row col
     * @param prompt : the prompt to display
     * @param in : the stream to get input from
     * @param out : the stream to display output to
     * @return : an optional containing a row and column if a valid one was
     * entered, otherwise an empty optional
     */
    std::optional<std::pair<int, int> > HumanPlayer::get_row_and_column(const std::string& prompt, std::istream& in,
                                                                   std::ostream& out) const {
        out << prompt;
        std::string user_input;
        std::getline(in, user_input);
        std::stringstream line_parser(user_input);
        int row, col;
        if (not(line_parser >> row >> col)) {
            return {}; // failed to read a row and column
        }
        std::string should_be_empty;
        if (line_parser >> should_be_empty) {
            return {}; //something else on the line after the row and column
        }
        return std::make_optional(std::make_pair(row, col));
    }
} // BattleShip
