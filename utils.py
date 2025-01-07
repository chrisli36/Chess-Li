from enum import IntEnum
from typing import Optional
import time

def time_it(func):
    def wrapper(*args, **kwargs):
        start_time = time.time_ns()
        result = func(*args, **kwargs)
        end_time = time.time_ns()
        print(f"Function '{func.__name__}' executed in {end_time - start_time} nanoseconds")
        return result
    return wrapper

COLOR_DELIMITER = 6

class Piece(IntEnum):
    WHITE_PAWN = 0;     PAWN = 0
    WHITE_KNIGHT = 1;   KNIGHT = 1
    WHITE_BISHOP = 2;   BISHOP = 2
    WHITE_ROOK = 3;     ROOK = 3
    WHITE_QUEEN = 4;    QUEEN = 4
    WHITE_KING = 5;     KING = 5

    BLACK_PAWN = 6
    BLACK_KNIGHT = 7
    BLACK_BISHOP = 8
    BLACK_ROOK = 9
    BLACK_QUEEN = 10
    BLACK_KING = 11

    EMPTY = 12

def ctzll(x):
    return (x & -x).bit_length() - 1

def ctzll_iterator(x: int):
    while x:
        n = ctzll(x)
        yield n
        x &= x - 1

RANKS = [8, 7, 6, 5, 4, 3, 2, 1]
FILES = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']

FILE = {
    'a': 0x0101010101010101,
    'b': 0x0202020202020202,
    'c': 0x0404040404040404,
    'd': 0x0808080808080808,
    'e': 0x1010101010101010,
    'f': 0x2020202020202020,
    'g': 0x4040404040404040,
    'h': 0x8080808080808080,
}

RANK = {
    1: 0xFF00000000000000,
    2: 0x00FF000000000000,
    3: 0x0000FF0000000000,
    4: 0x000000FF00000000,
    5: 0x00000000FF000000,
    6: 0x0000000000FF0000,
    7: 0x000000000000FF00,
    8: 0x00000000000000FF,
}

# s = ""
# for i, r in enumerate(RANKS):
#     for j, f in enumerate(FILES):
#         # s += f"'{f}{r}':{hex(1 << (i * 8 + j))}, "
#         # s += f"'{f}{r}':{hex(1 << (i * 8 + 
#     s += "\n"

# print(s)

SQUARE_TO_NUM = {
    'a8':0,  'b8':1,  'c8':2,  'd8':3,  'e8':4,  'f8':5,  'g8':6,  'h8':7,
    'a7':8,  'b7':9,  'c7':10, 'd7':11, 'e7':12, 'f7':13, 'g7':14, 'h7':15,
    'a6':16, 'b6':17, 'c6':18, 'd6':19, 'e6':20, 'f6':21, 'g6':22, 'h6':23, 
    'a5':24, 'b5':25, 'c5':26, 'd5':27, 'e5':28, 'f5':29, 'g5':30, 'h5':31, 
    'a4':32, 'b4':33, 'c4':34, 'd4':35, 'e4':36, 'f4':37, 'g4':38, 'h4':39, 
    'a3':40, 'b3':41, 'c3':42, 'd3':43, 'e3':44, 'f3':45, 'g3':46, 'h3':47, 
    'a2':48, 'b2':49, 'c2':50, 'd2':51, 'e2':52, 'f2':53, 'g2':54, 'h2':55, 
    'a1':56, 'b1':57, 'c1':58, 'd1':59, 'e1':60, 'f1':61, 'g1':62, 'h1':63,
}

SQUARE_TO_POS = {
    'a8':0x000000000000001, 'b8':0x000000000000002, 'c8':0x000000000000004, 'd8':0x000000000000008, 'e8':0x0000000000000010, 'f8':0x0000000000000020, 'g8':0x0000000000000040, 'h8':0x0000000000000080, 
    'a7':0x000000000000100, 'b7':0x000000000000200, 'c7':0x000000000000400, 'd7':0x000000000000800, 'e7':0x0000000000001000, 'f7':0x0000000000002000, 'g7':0x0000000000004000, 'h7':0x0000000000008000, 
    'a6':0x000000000010000, 'b6':0x000000000020000, 'c6':0x000000000040000, 'd6':0x000000000080000, 'e6':0x0000000000100000, 'f6':0x0000000000200000, 'g6':0x0000000000400000, 'h6':0x0000000000800000, 
    'a5':0x000000001000000, 'b5':0x000000002000000, 'c5':0x000000004000000, 'd5':0x000000008000000, 'e5':0x0000000010000000, 'f5':0x0000000020000000, 'g5':0x0000000040000000, 'h5':0x0000000080000000, 
    'a4':0x000000100000000, 'b4':0x000000200000000, 'c4':0x000000400000000, 'd4':0x000000800000000, 'e4':0x0000001000000000, 'f4':0x0000002000000000, 'g4':0x0000004000000000, 'h4':0x0000008000000000, 
    'a3':0x000010000000000, 'b3':0x000020000000000, 'c3':0x000040000000000, 'd3':0x000080000000000, 'e3':0x0000100000000000, 'f3':0x0000200000000000, 'g3':0x0000400000000000, 'h3':0x0000800000000000, 
    'a2':0x001000000000000, 'b2':0x002000000000000, 'c2':0x004000000000000, 'd2':0x008000000000000, 'e2':0x0010000000000000, 'f2':0x0020000000000000, 'g2':0x0040000000000000, 'h2':0x0080000000000000, 
    'a1':0x100000000000000, 'b1':0x200000000000000, 'c1':0x400000000000000, 'd1':0x800000000000000, 'e1':0x1000000000000000, 'f1':0x2000000000000000, 'g1':0x4000000000000000, 'h1':0x8000000000000000, 
}

PIECE_VALUES = [100, 300, 320, 500, 900, 0]

POSITION_VALUES = [
    # pawns
    [
        0,   0,   0,   0,   0,   0,   0,   0,
        50,  50,  50,  50,  50,  50,  50,  50,
        10,  10,  20,  30,  30,  20,  10,  10,
        5,   5,  10,  25,  25,  10,   5,   5,
        0,   0,   0,  20,  20,   0,   0,   0,
        5,  -5, -10,   0,   0, -10,  -5,   5,
        5,  10,  10, -20, -20,  10,  10,   5,
        0,   0,   0,   0,   0,   0,   0,   0
    ],
    # knights
    [
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50,
    ],
    # bishops
    [
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
    ],
    # rooks
    [
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        0,  0,  0,  5,  5,  0,  0,  0
    ],
    # queens
    [
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -5,   0,  5,  5,  5,  5,  0, -5,
        0,    0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    ],
    # king
    [
    -80, -70, -70, -70, -70, -70, -70, -80, 
    -60, -60, -60, -60, -60, -60, -60, -60, 
    -40, -50, -50, -60, -60, -50, -50, -40, 
    -30, -40, -40, -50, -50, -40, -40, -30, 
    -20, -30, -30, -40, -40, -30, -30, -20, 
    -10, -20, -20, -20, -20, -20, -20, -10, 
    20,  20,  -5,  -5,  -5,  -5,  20,  20, 
    20,  30,  10,   0,   0,  10,  30,  20
    ],
]