from utils import *
from board import Board, PROMOTIONS
from position import Position

PIECE_VALUES = [1, 3, 3, 5, 9]

class Engine:
    def __init__(self):
        pass

    def evaluate(self, board: Board):
        board._ensure_updated()
        ret = 0
        for p, val in zip(board.friend_list, PIECE_VALUES):
            ret += p.bit_count() * val
        for p, val in zip(board.enemy_list, PIECE_VALUES):
            ret -= p.bit_count() * val

    def search(self, board: Board, depth: int) -> int:
        if depth == 0:
            return 1
        ret = 0
        for pos, moves in board.calculate_all_moves().items():
            while moves.mask:
                s, e = pos, Position(ctzll(moves.mask))
                if board.is_promoting(s, e):
                    for promo in PROMOTIONS:
                        taken, state_changes = board.make_move(s, e, moves.piece, promo)
                        ret += self.search(board, depth - 1)
                        board.undo_move(s, e, moves.piece, taken, state_changes)
                else:
                    taken, state_changes = board.make_move(s, e, moves.piece)
                    ret += self.search(board, depth - 1)
                    board.undo_move(s, e, moves.piece, taken, state_changes)
                moves.mask &= moves.mask - 1
        return ret

if __name__ == "__main__":
    li = Engine()
    board = Board()
    
    for i in range(1, 5):
        a = time.time_ns()
        positions = li.search(board, i)
        b = time.time_ns()
        print(f"depth={i}: calculated {positions} in {(b - a) // 10**6} ms")