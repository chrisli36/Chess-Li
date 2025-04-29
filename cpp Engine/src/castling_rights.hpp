#pragma once
#include <cstdint>

struct CastlingRights {
    uint8_t rights;

    static constexpr uint8_t K = 1 << 0;
    static constexpr uint8_t Q = 1 << 1;
    static constexpr uint8_t k = 1 << 2;
    static constexpr uint8_t q = 1 << 3;

    /**
     * @brief Constructs a new CastlingRights object.
     * 
     */
    constexpr CastlingRights(uint8_t r = 0) : rights(r) {}

    /** 
     * @brief Determines if a side can castle.
     * 
     * @param side The side to check (K, Q, k, q).
     * @return true if the side can castle, false otherwise.
     */
    constexpr bool can_castle(uint8_t side) const { return (rights & side) != 0; }

    /** 
     * @brief Removes the castling right for a side.
     * 
     * @param side The side to remove the castling right from (K, Q, k, q).
     */
    constexpr void remove_right(uint8_t side) { rights &= ~side; }

    /** 
     * @brief Adds the castling right for a side.
     * 
     * @param side The side to add the castling right to (K, Q, k, q).
     */
    constexpr void add_right(uint8_t side) { rights |= side; }

};