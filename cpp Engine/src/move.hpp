#pragma once
#include <cstdint>
#include <optional>
#include <string>

#include "piece.hpp"
#include "castling_rights.hpp"
#include "bitboard.hpp"

enum MoveFlag : uint8_t {
    NO_FLAG             = 0,
    EN_PASSANT_CAPTURE  = 1,
    PAWN_UP_TWO         = 2,
    KINGSIDE_CASTLE     = 3,
    QUEENSIDE_CASTLE    = 4,
    QUEEN_PROMOTION     = 5,
    ROOK_PROMOTION      = 6,
    BISHOP_PROMOTION    = 7,
    KNIGHT_PROMOTION    = 8,
};

struct Move {
    uint16_t move;

    // Bit masks
    static constexpr uint8_t START_MASK = 0b111111;  // bits 0–5
    static constexpr uint8_t END_MASK   = 0b111111;  // bits 6–11
    static constexpr uint8_t FLAG_MASK  = 0b1111;    // bits 12–15

    // Constructors
    constexpr Move(uint16_t m = 0) : move(m) {}
    constexpr Move(uint8_t start, uint8_t end, MoveFlag flag = MoveFlag::NO_FLAG)
        : move(static_cast<uint16_t>(start & START_MASK) |
               (static_cast<uint16_t>(end & END_MASK) << 6) |
               (static_cast<uint16_t>(flag & FLAG_MASK) << 12)) {}

    // Accessors
    constexpr uint8_t start() const { return static_cast<uint8_t>(move & static_cast<uint16_t>(START_MASK)); }
    constexpr uint8_t end() const { return static_cast<uint8_t>((move >> 6) & static_cast<uint16_t>(END_MASK)); }
    constexpr MoveFlag flag() const { return static_cast<MoveFlag>((move >> 12) & static_cast<uint16_t>(FLAG_MASK)); }

    // Queries
    constexpr bool is_no_flag() const { return flag() == MoveFlag::NO_FLAG; }
    constexpr bool is_en_passant() const { return flag() == MoveFlag::EN_PASSANT_CAPTURE; }
    constexpr bool is_pawn_up_two() const { return flag() == MoveFlag::PAWN_UP_TWO; }
    constexpr bool is_castle() const { return is_castle_kingside() || is_castle_queenside(); }
    constexpr bool is_castle_kingside() const { return flag() == MoveFlag::KINGSIDE_CASTLE; }
    constexpr bool is_castle_queenside() const { return flag() == MoveFlag::QUEENSIDE_CASTLE; }

    // Promotion
    static constexpr uint8_t FIRST_PROMOTION_FLAG = MoveFlag::QUEEN_PROMOTION;
    constexpr bool is_promotion() const { return flag() >= FIRST_PROMOTION_FLAG; }
    constexpr Piece promotion_piece(Turn turn) const {
        switch (flag()) {
            case MoveFlag::QUEEN_PROMOTION:  return (turn == Turn::WHITE) ? Piece::WHITE_QUEEN : Piece::BLACK_QUEEN;
            case MoveFlag::ROOK_PROMOTION:   return (turn == Turn::WHITE) ? Piece::WHITE_ROOK : Piece::BLACK_ROOK;
            case MoveFlag::BISHOP_PROMOTION: return (turn == Turn::WHITE) ? Piece::WHITE_BISHOP : Piece::BLACK_BISHOP;
            case MoveFlag::KNIGHT_PROMOTION: return (turn == Turn::WHITE) ? Piece::WHITE_KNIGHT : Piece::BLACK_KNIGHT;
            default:                         return (turn == Turn::WHITE) ? Piece::WHITE_QUEEN : Piece::BLACK_QUEEN;
        }
    }

    // Algebraic notation
    static constexpr char file_of(uint8_t sq) { return 'a' + (sq % 8); }
    static constexpr char rank_of(uint8_t sq) { return '1' + (sq / 8); }
    static std::string to_algebraic(uint8_t sq) { return std::string(1, file_of(sq)) + std::string(1, rank_of(sq)); }

    // UCI format: e2e4, e7e8q, etc.
    std::string to_uci() const {
        std::string uci = to_algebraic(start()) + to_algebraic(end());
        if (is_promotion()) {
            switch (flag()) {
                case QUEEN_PROMOTION:  uci += 'q'; break;
                case ROOK_PROMOTION:   uci += 'r'; break;
                case BISHOP_PROMOTION: uci += 'b'; break;
                case KNIGHT_PROMOTION: uci += 'n'; break;
                default: break;
            }
        }
        return uci;
    }
    static std::optional<Move> from_uci(const std::string& uci) {
        if (uci.size() < 4 || uci.size() > 5) return std::nullopt;
        if (uci[0] < 'a' || uci[0] > 'h' || uci[1] < '1' || uci[1] > '8') return std::nullopt;
        if (uci[2] < 'a' || uci[2] > 'h' || uci[3] < '1' || uci[3] > '8') return std::nullopt;
        uint8_t start_sq = static_cast<uint8_t>((uci[1] - '1') * 8 + (uci[0] - 'a'));
        uint8_t end_sq   = static_cast<uint8_t>((uci[3] - '1') * 8 + (uci[2] - 'a'));
        MoveFlag flag = MoveFlag::NO_FLAG;
        if (uci.size() == 5) {
            switch (uci[4]) {
                case 'q': flag = MoveFlag::QUEEN_PROMOTION;  break;
                case 'r': flag = MoveFlag::ROOK_PROMOTION;   break;
                case 'b': flag = MoveFlag::BISHOP_PROMOTION; break;
                case 'n': flag = MoveFlag::KNIGHT_PROMOTION; break;
                default: return std::nullopt;
            }
        }
        return Move(start_sq, end_sq, flag);
    }
};

struct UnMove {
    Move move;
    Piece taken_piece;
    Bitboard en_passant_square;
    CastlingRights castling_rights;

    UnMove(Move m, Piece t, Bitboard e, CastlingRights c) : move(m), taken_piece(t), en_passant_square(e), castling_rights(c) {}
};