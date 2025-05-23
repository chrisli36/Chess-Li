#pragma once
#include <cstdint>
#include <iostream>
#include <bitset>

#define CTZLL_ITERATOR(sq, bb) for (uint64_t _bb = (bb); _bb && ((sq) = __builtin_ctzll(_bb), true); _bb &= _bb - 1)


struct Bitboard {
/**
 * @brief A class representing a bitboard for chess.
 * 
 * By convention, the MSB represents the a1 square and the LSB represents the h8 square.
 */
private:
    uint64_t bitboard;

public:
    /** 
     * @brief Constructs a new Bitboard object.
     */
    constexpr Bitboard(uint64_t bits = 0) : bitboard(bits) {}

    /**
     * @brief Implicit conversion operator to uint64_t.
     * 
     * @return uint64_t 
     */
    constexpr operator uint64_t() const { return bitboard; }

    // Bitwise operators
    constexpr Bitboard operator&(const Bitboard& other) const { return Bitboard(bitboard & other.bitboard); }
    constexpr Bitboard operator|(const Bitboard& other) const { return Bitboard(bitboard | other.bitboard); }
    constexpr Bitboard operator^(const Bitboard& other) const { return Bitboard(bitboard ^ other.bitboard); }
    constexpr Bitboard operator~() const { return Bitboard(~bitboard); }

    Bitboard& operator&=(const Bitboard& other) { bitboard &= other.bitboard; return *this; }
    Bitboard& operator|=(const Bitboard& other) { bitboard |= other.bitboard; return *this; }
    Bitboard& operator^=(const Bitboard& other) { bitboard ^= other.bitboard; return *this; }

    /**
     * @brief Resets the bitboard to 0.
     * 
     */
    constexpr void reset() { bitboard = 0ULL;
    }

    /**
     * @brief Determines if bitboard covers a square.
     * 
     * @param sq The square in the query
     * @return A boolean representing whether or not the bitboard contains the square.
     */
    constexpr Bitboard covers(uint8_t sq) const {
        return (bitboard & (1ULL << sq)) > 0;
    }

    /**
     * @brief Adds a piece to the bitboard
     * 
     * @param sq The square to add the piece to.
     */
    constexpr void add_square(uint8_t sq) {
        bitboard |= (1ULL << sq);
    }

    /**
     * @brief Removes a piece from the bitboard.
     * 
     * @param sq The square to remove the piece from.
     */
    constexpr void remove_square(uint8_t sq) {
        bitboard &= ~(1ULL << sq);
    }

    /**
     * @brief prints the bitboard as a 8x8 grid.
     * 
     */
    void print() {
        std::bitset<64> bits(bitboard);
        int rank, file;
        for (rank = 7; rank >= 0; --rank) {
            for (file = 0; file < 8; ++file) {
                std::cout << bits[rank * 8 + file];
            }
            std::cout << '\n';
        }
    }
};