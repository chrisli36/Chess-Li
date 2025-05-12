#pragma once
#include <cstdint>
#include <string>

#include "piece.hpp"

enum MoveFlag : uint8_t {
    NO_FLAG             = 0,
    EN_PASSANT_CAPTURE  = 1,
    PAWN_UP_TWO         = 2,
    CASTLE_KINGSIDE     = 3,
    CASTLE_QUEENSIDE    = 4,
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
    constexpr bool is_castle_kingside() const { return flag() == MoveFlag::CASTLE_KINGSIDE; }
    constexpr bool is_castle_queenside() const { return flag() == MoveFlag::CASTLE_QUEENSIDE; }

    static constexpr uint8_t FIRST_PROMOTION_FLAG = MoveFlag::QUEEN_PROMOTION;
    constexpr bool is_promotion() const { return flag() >= FIRST_PROMOTION_FLAG; }
    constexpr Piece promotion_piece() const {
        switch (flag()) {
            case MoveFlag::QUEEN_PROMOTION:  return Piece::WHITE_QUEEN;
            case MoveFlag::ROOK_PROMOTION:   return Piece::WHITE_ROOK;
            case MoveFlag::BISHOP_PROMOTION: return Piece::WHITE_BISHOP;
            case MoveFlag::KNIGHT_PROMOTION: return Piece::WHITE_KNIGHT;
            default:                         return Piece::EMPTY;
        }
    }

    static constexpr char file_of(uint8_t sq) { return 'a' + (sq % 8); }
    static constexpr char rank_of(uint8_t sq) { return '1' + (sq / 8); }
    std::string to_algebraic(uint8_t sq) const {
        return std::string(1, file_of(sq)) + std::string(1, rank_of(sq));
    }
    std::string to_string() const { 
        return to_algebraic(start()) + "->" + to_algebraic(end());
    }
};