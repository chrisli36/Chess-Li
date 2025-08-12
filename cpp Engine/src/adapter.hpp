#pragma once

#include <string>
#include <optional>
#include "move.hpp"
#include "board.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace ChessAdapter {

    // Move conversion helpers
    struct MoveInfo {
        std::string long_notation;  // "e2e4"
        std::string from;           // "e2"
        std::string to;             // "e4"
        std::optional<std::string> promo;  // "q", "r", "b", "n" or null
    };

    // Convert Move to algebraic notation
    MoveInfo move_to_algebraic(const Move& move, const Board& board);
    
    // Parse algebraic notation to Move
    std::optional<Move> algebraic_to_move(const std::string& notation, const Board& board);
    
    // FEN serialization
    std::string board_to_fen(const Board& board);
    
    // Game state helpers
    std::string game_state_to_string(GameState state);
    
    // JSON serialization helpers
    json move_to_json(const Move& move, const Board& board);
    json score_to_json(int score);
    json board_status_to_json(Board& board, const std::string& last_move);

} // namespace ChessAdapter 