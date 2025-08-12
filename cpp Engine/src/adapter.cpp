#include "adapter.hpp"
#include "nlohmann/json.hpp"
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

namespace ChessAdapter {

    MoveInfo move_to_algebraic(const Move& move, const Board& board) {
        MoveInfo info;
        
        // Get from and to squares
        info.from = Move::to_algebraic(move.start());
        info.to = Move::to_algebraic(move.end());
        info.long_notation = info.from + info.to;
        
        // Handle promotion
        if (move.is_promotion()) {
            switch (move.flag()) {
                case MoveFlag::QUEEN_PROMOTION:
                    info.promo = "q";
                    break;
                case MoveFlag::ROOK_PROMOTION:
                    info.promo = "r";
                    break;
                case MoveFlag::BISHOP_PROMOTION:
                    info.promo = "b";
                    break;
                case MoveFlag::KNIGHT_PROMOTION:
                    info.promo = "n";
                    break;
                default:
                    info.promo = std::nullopt;
            }
            info.long_notation += info.promo.value();
        } else {
            info.promo = std::nullopt;
        }
        
        return info;
    }
    
    std::optional<Move> algebraic_to_move(const std::string& notation, const Board& board) {
        if (notation.length() < 4) return std::nullopt;
        
        // Parse from and to squares
        std::string from_str = notation.substr(0, 2);
        std::string to_str = notation.substr(2, 2);
        
        // Convert algebraic to square indices
        int from_file = from_str[0] - 'a';
        int from_rank = from_str[1] - '1';
        int to_file = to_str[0] - 'a';
        int to_rank = to_str[1] - '1';
        
        if (from_file < 0 || from_file > 7 || from_rank < 0 || from_rank > 7 ||
            to_file < 0 || to_file > 7 || to_rank < 0 || to_rank > 7) {
            return std::nullopt;
        }
        
        // Convert to square indices (0-63)
        // Note: In chess, ranks go 1-8, but we need 0-7 for array indexing
        // Also, the board representation might be different than expected
        int from_sq = from_rank * 8 + from_file;
        int to_sq = to_rank * 8 + to_file;
        
        std::cout << "Debug: Converting " << from_str << " to file=" << from_file << " rank=" << from_rank << " sq=" << from_sq << std::endl;
        std::cout << "Debug: Converting " << to_str << " to file=" << to_file << " rank=" << to_rank << " sq=" << to_sq << std::endl;
        
        // Handle promotion
        MoveFlag flag = MoveFlag::NO_FLAG;
        if (notation.length() == 5) {
            char promo = notation[4];
            switch (promo) {
                case 'q': flag = MoveFlag::QUEEN_PROMOTION; break;
                case 'r': flag = MoveFlag::ROOK_PROMOTION; break;
                case 'b': flag = MoveFlag::BISHOP_PROMOTION; break;
                case 'n': flag = MoveFlag::KNIGHT_PROMOTION; break;
                default: return std::nullopt;
            }
        } else {
            // Check if this is a pawn move up two squares
            // Pawns start on rank 2 (white) or rank 7 (black)
            // If moving from rank 1 or 6 to rank 3 or 4, it's a pawn up two
            if (from_rank == 1 && to_rank == 3) {
                flag = MoveFlag::PAWN_UP_TWO;
            } else if (from_rank == 6 && to_rank == 4) {
                flag = MoveFlag::PAWN_UP_TWO;
            }
        }
        
        return Move(from_sq, to_sq, flag);
    }
    
    std::string board_to_fen(const Board& board) {
        std::ostringstream fen;
        
        // Piece placement
        for (int rank = 7; rank >= 0; --rank) {
            int empty_count = 0;
            for (int file = 0; file < 8; ++file) {
                int sq = rank * 8 + file;
                Piece piece = board.get_piece(sq);
                
                if (piece.is_empty()) {
                    empty_count++;
                } else {
                    if (empty_count > 0) {
                        fen << empty_count;
                        empty_count = 0;
                    }
                    fen << piece.to_char();
                }
            }
            if (empty_count > 0) {
                fen << empty_count;
            }
            if (rank > 0) fen << '/';
        }
        
        // Active color
        fen << (board.get_turn() == Turn::WHITE ? " w " : " b ");
        
        // Castling availability
        // Note: This is a simplified version - you may need to implement proper castling rights
        fen << "KQkq ";
        
        // En passant target square
        // Note: This is a simplified version - you may need to implement proper en passant tracking
        fen << "- ";
        
        // Halfmove clock and fullmove number
        fen << "0 1";
        
        return fen.str();
    }
    
    std::string game_state_to_string(GameState state) {
        switch (state) {
            case GameState::WHITE_WIN: return "mate";
            case GameState::BLACK_WIN: return "mate";
            case GameState::DRAW: return "draw";
            case GameState::IN_PROGRESS: return "ongoing";
            default: return "ongoing";
        }
    }
    
    json move_to_json(const Move& move, const Board& board) {
        std::cout << "Debug: Starting move_to_json" << std::endl;
        std::cout << "Debug: Move: start=" << (int)move.start() << " end=" << (int)move.end() << " flag=" << (int)move.flag() << std::endl;
        
        MoveInfo info = move_to_algebraic(move, board);
        std::cout << "Debug: move_to_algebraic completed" << std::endl;
        
        std::cout << "Debug: MoveInfo: long='" << info.long_notation << "' from='" << info.from << "' to='" << info.to << "'" << std::endl;
        
        json j;
        std::cout << "Debug: Creating JSON object..." << std::endl;
        j["long"] = info.long_notation;
        std::cout << "Debug: Added 'long' field" << std::endl;
        j["from"] = info.from;
        std::cout << "Debug: Added 'from' field" << std::endl;
        j["to"] = info.to;
        std::cout << "Debug: Added 'to' field" << std::endl;
        if (info.promo.has_value()) {
            j["promo"] = info.promo.value();
        } else {
            j["promo"] = nullptr;
        }
        std::cout << "Debug: Added 'promo' field" << std::endl;
        
        std::cout << "Debug: JSON created successfully" << std::endl;
        return j;
    }
    
    json score_to_json(int score) {
        json j;
        if (abs(score) > 90000) { // Mate score
            j["cp"] = nullptr;
            j["mate"] = (score > 0) ? (100000 - score + 1) / 2 : -(100000 + score + 1) / 2;
        } else {
            j["cp"] = score;
            j["mate"] = nullptr;
        }
        return j;
    }
    
    json board_status_to_json(Board& board, const std::string& last_move) {
        json j;
        
        // Check if move is legal
        bool legal = true; // Simplified - you may want to implement proper move validation
        
        j["legal"] = legal;
        j["fen"] = legal ? board_to_fen(board) : nullptr;
        j["status"] = game_state_to_string(board.get_game_state());
        j["lastMove"] = last_move;
        
        return j;
    }

} // namespace ChessAdapter 