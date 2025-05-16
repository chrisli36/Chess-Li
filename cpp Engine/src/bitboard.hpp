#pragma once
#include <cstdint>
#include <iostream>
#include <bitset>

#define CTZLL_ITERATOR(sq, bb) for (uint64_t _bb = (bb); _bb && ((sq) = __builtin_ctzll(_bb), true); _bb &= _bb - 1)


struct Bitboard {
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
     * @brief Returns the intersection of the bitboard with a specific square.
     * 
     * @param sq The square to intersect with.
     * @return The intersection of the bitboard with the square.
     */
    constexpr Bitboard intersect_with_square(uint8_t sq) const {
        return Bitboard(bitboard & (1ULL << sq));
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
     * @brief prints the bitboard as a 8x8 grid.
     * 
     */
    void print() {
        std::bitset<64> bits(bitboard);

        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                int bit_index = 63 - (row * 8 + col);
                std::cout << bits[bit_index];
            }
            std::cout << '\n';
        }
    }
};