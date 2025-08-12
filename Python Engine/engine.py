from utils import *
from board import Board, GameState, Turn
from position import Position

class Engine:
    def __init__(self):
        self.positions = 0
        pass

    def evaluate(self, board: Board) -> int:
        board._ensure_updated()
        if board.game_state == GameState.WHITE or board.game_state == GameState.BLACK:
            return float('-inf')
        elif board.game_state == GameState.DRAW:
            return 0

        ret = 0

        # add and subtract material
        for p, val in zip(board.friend_list, PIECE_VALUES):
            ret += p.bit_count() * val
        for p, val in zip(board.enemy_list, PIECE_VALUES):
            ret -= p.bit_count() * val
        
        # add and subtract based off of position
        # for p, values in zip(board.friend_list, POSITION_VALUES):
        #     for n in ctzll_iterator(p):
        #         ret += values[n] if board.turn == Turn.WHITE else values[63 - n]
        # for p, values in zip(board.enemy_list, POSITION_VALUES):
        #     for n in ctzll_iterator(p):
        #         ret -= values[n] if board.turn == Turn.WHITE else values[63 - n]
        return ret

    def search(self, board: Board, depth: int) -> int:
        if depth == 0:
            return 1
        ret = 0
        for s, e, piece, promo in board.get_all_moves_list():
            taken, state_changes = board.make_move(s, e, piece, promo)
            ret += self.search(board, depth - 1)
            board.undo_move(s, e, piece, taken, state_changes)
        return ret
    
    def score_move(self, board: Board, piece: Piece, e: Position, promo: Optional[Piece]) -> int:
        guess = 0
        if board.board[e.num] != Piece.EMPTY:
            guess += 10 * PIECE_VALUES[board.board[e.num] % COLOR_DELIMITER] - PIECE_VALUES[piece]
        if promo is not None:
            guess += PIECE_VALUES[promo]
        if e.overlap(board.controlled):
            guess -= PIECE_VALUES[piece]
        return guess

    def minimax(self, board: Board, depth: int, alpha: int, beta: int) -> int:
        if depth == 0:
            self.positions += 1
            return self.evaluate(board)
        
        board._ensure_updated()
        if board.game_state == GameState.WHITE or board.game_state == GameState.BLACK:
            self.positions += 1
            return float('-inf')
        elif board.game_state == GameState.DRAW:
            self.positions += 1
            return 0
        
        all_moves = board.get_all_moves_list()
        all_moves.sort(key=lambda x: self.score_move(board, x[2], x[1], x[3]),reverse=True)
        for s, e, piece, promo in all_moves:
            taken, state_changes = board.make_move(s, e, piece, promo)
            curr_eval = -self.minimax(board, depth - 1, -beta, -alpha)
            board.undo_move(s, e, piece, taken, state_changes)

            if curr_eval >= beta:
                return beta
            alpha = max(alpha, curr_eval)
        return alpha

if __name__ == "__main__":
    li = Engine()
    board = Board()

    # for i in range(1, 6):
    #     a = time.time_ns()
    #     li.positions = 0
    #     engine_eval = li.minimax(board, i, float('-inf'), float('inf'))
    #     b = time.time_ns()
    #     print(f"depth={i}: evaluated {li.positions} positions in {(b - a) // 10**6} ms")
    
    for i in range(1, 5):
        a= time.time_ns()
        positions = li.search(board, i)
        b = time.time_ns()
        print(f"depth={i}: calculated {positions} in {(b - a) // 10**6} ms")