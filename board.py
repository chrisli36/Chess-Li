from position import *
from move import *
from enum import Flag

ROOK_DIRECTIONS = [Position.up, Position.down, Position.right, Position.left]
BISHOP_DIRECTIONS = [Position.up_left, Position.up_right, Position.down_right, Position.down_left]
KNIGHT_DIRECTIONS = [
    Position.up_up_right, Position.up_right_right, Position.down_right_right, Position.down_down_right,
    Position.down_left_left, Position.down_down_left, Position.up_left_left, Position.up_up_left
]

class Turn(Flag):
    WHITE = True
    BLACK = False

class Board:
    def __init__(self):
        self.pieces_list = [
            sum([1 << i for i in range(48, 56)]),   # white pawns
            (1 << 57) + (1 << 62),                  # white knights
            (1 << 58) + (1 << 61),                  # white bishops
            (1 << 56) + (1 << 63),                  # white rooks
            (1 << 59),                              # white queens
            (1 << 60),                              # white kings

            sum([1 << i for i in range(8, 16)]),    # black pawns
            (1 << 1) + (1 << 6),                    # black knights
            (1 << 2) + (1 << 5),                    # black bishops
            (1 << 0) + (1 << 7),                    # black rooks
            (1 << 3),                               # black queens
            (1 << 4),                               # black kings
        ]

        self.pieces = 0
        self.white_pieces = 0
        self.black_pieces = 0
        self.turn = Turn.WHITE
        self.enemy = self.black_pieces
        self.all_moves = dict()

        self.updated = False
        self._ensure_updated()

    def __str__(self) -> str:
        self._ensure_updated()

        position = Position()
        ret = ""
        for i in range(8):
            ret += "  " + "-" * 33 + "\n"
            ret += str(8 - i) + " |"
            for j in range(8):
                ret += " " + self._print_piece(position) + " |"
                position = position.next()
            ret += "\n"
        ret += "  " + "-" * 33 + "\n"
        ret += "    a   b   c   d   e   f   g   h\n"
        ret += f"""{"White" if self.turn == Turn.WHITE else "Black"} to move"""
        return ret

    def is_piece(self, position: Position) -> bool:
        self._ensure_updated()
        return position.overlap(self.pieces) > 0

    def is_enemy_controlled(self, position: Position) -> bool:
        self._ensure_updated()
        return position.overlap(self.controlled) > 0

    def get_piece(self, position: Position) -> Piece:
        self._ensure_updated()
        if not (position.overlap(self.pieces)):
            return Piece.EMPTY
        
        for i, p in enumerate(self.pieces_list):
            if position.overlap(p):
                return Piece(i)

    def _print_piece(self, position: Position) -> str:
        self._ensure_updated()
        representations = ['p', 'n', 'b', 'r', 'q', 'k', ' ']
        piece = self.get_piece(position)
        piece_repr = piece % COLOR_DELIMITER if piece != Piece.EMPTY else len(representations) - 1
        return representations[piece_repr]

    def _ensure_updated(self) -> None:
        if self.updated:
            return
        
        self.white_pieces = 0
        for p in self.pieces_list[:COLOR_DELIMITER]:
            self.white_pieces |= p
        self.black_pieces = 0
        for p in self.pieces_list[COLOR_DELIMITER:]:
            self.black_pieces |= p
        self.pieces = self.black_pieces | self.white_pieces

        if self.turn == Turn.WHITE:
            self.enemy = self.black_pieces
        elif self.turn == Turn.BLACK:
            self.enemy = self.white_pieces

        self.in_check = False
        self.calculate_all_valid_moves()

        self.updated = True

    def calculate_all_valid_moves(self) -> dict[Position, dict[Position, Move]]:
        if self.updated:
            return self.all_moves

        self.controlled = 0
        for f in FILES:
            for r in RANKS:
                start = Position.get_position(f"{f}{r}")
                moves = self.get_valid_moves(start)
                self.all_moves[start] = moves
        return self.all_moves

    def get_valid_moves(self, position: Position) -> dict[Position, Move]:
        if self.updated:
            return self.all_moves[position]

        moves = dict()
        if not position.overlap(self.pieces):
            return moves
        
        if position.overlap(self.pieces_list[Piece.WHITE_PAWN] | self.pieces_list[Piece.BLACK_PAWN]):
            self._calc_pawn_moves(position, moves)
        elif position.overlap(self.pieces_list[Piece.WHITE_KNIGHT] | self.pieces_list[Piece.BLACK_KNIGHT]):
            self._calc_knight_moves(position, moves)
        elif position.overlap(self.pieces_list[Piece.WHITE_BISHOP] | self.pieces_list[Piece.BLACK_BISHOP]):
            self._calc_bishop_moves(position, moves)
        elif position.overlap(self.pieces_list[Piece.WHITE_ROOK] | self.pieces_list[Piece.BLACK_ROOK]):
            self._calc_rook_moves(position, moves)
        elif position.overlap(self.pieces_list[Piece.WHITE_QUEEN] | self.pieces_list[Piece.BLACK_QUEEN]):
            self._calc_queen_moves(position, moves)
        elif position.overlap(self.pieces_list[Piece.WHITE_KING] | self.pieces_list[Piece.BLACK_KING]):
            self._calc_king_moves(position, moves)
        return moves

    def _calc_pawn_moves(self, position: Position, moves: dict[Position, Move]) -> None:
        if position.overlap(self.white_pieces):
            pawn_forward = Position.up
            pawn_take_right = Position.up_right
            pawn_take_left = Position.up_left
        elif position.overlap(self.black_pieces):
            pawn_forward = Position.down
            pawn_take_right = Position.down_left
            pawn_take_left = Position.down_right
        opposing = position.overlap(self.enemy)
        pawn = Piece.WHITE_PAWN + (6 if position.overlap(self.black_pieces) else 0)
        
        # update controlled squares
        if opposing:
            for pawn_take in [pawn_take_left, pawn_take_right]:
                take = pawn_take(position)
                if take:
                    self.controlled |= take.pos
            return

        # calculate forward pawn moves
        forward = pawn_forward(position)
        if forward and not forward.overlap(self.pieces):
            moves[forward] = Move(position, forward, MoveType.MOVE, pawn)
            fforward = pawn_forward(forward)
            if ((self.turn == Turn.WHITE and position.rank() == 2) or (self.turn == Turn.BLACK and position.rank() == 7)) \
                    and fforward and not fforward.overlap(self.pieces):
                moves[fforward] = Move(position, fforward, MoveType.MOVE, pawn)
        # calculate taking on left and right
        for pawn_take in [pawn_take_left, pawn_take_right]:
            take = pawn_take(position)
            if take is None:
                continue
            # self.controlled |= take.pos
            if take.overlap(self.enemy):
                moves[take] = Move(position, take, MoveType.TAKE, pawn)

    def _calc_knight_moves(self, position: Position, moves: dict[Position, Move]) -> None:
        opposing = position.overlap(self.enemy)
        knight = Piece.WHITE_KNIGHT + (6 if position.overlap(self.black_pieces) else 0)

        # update controlled squares
        if opposing:
            for dir in KNIGHT_DIRECTIONS:
                mv = dir(position)
                if mv:
                    self.controlled |= mv.pos
            return

        for dir in KNIGHT_DIRECTIONS:
            mv = dir(position)
            if mv is None:
                continue
            # calculate unoccupied squares
            if not mv.overlap(self.pieces):
                moves[mv] = Move(position, mv, MoveType.MOVE, knight)
            # calculate taking enemy pieces
            elif mv.overlap(self.enemy):
                moves[mv] = Move(position, mv, MoveType.TAKE, knight)

    def _calc_bishop_moves(self, position: Position, moves: dict[Position, Move]) -> None:
        opposing = position.overlap(self.enemy)
        bishop = Piece.WHITE_BISHOP + (6 if position.overlap(self.black_pieces) else 0)

        # update controlled squares
        if opposing:
            for dir in BISHOP_DIRECTIONS:
                mv = dir(position)
                while mv and not mv.overlap(self.pieces):
                    self.controlled |= mv.pos
                    mv = dir(mv)
                if mv:
                    self.controlled |= mv.pos
            return
        
        # calculate moves in each diagonal direction
        for dir in BISHOP_DIRECTIONS:
            mv = dir(position)
            while mv and not mv.overlap(self.pieces):
                moves[mv] = Move(position, mv, MoveType.MOVE, bishop)
                mv = dir(mv)
            # if there's an enemy piece at the end, then we could take
            if mv and mv.overlap(self.enemy):
                moves[mv] = Move(position, mv, MoveType.TAKE, bishop)
    
    def _calc_rook_moves(self, position: Position, moves: dict[Position, Move]) -> None:
        opposing = position.overlap(self.enemy)
        rook = Piece.WHITE_ROOK + (6 if position.overlap(self.black_pieces) else 0)

        if opposing:
            for dir in ROOK_DIRECTIONS:
                mv = dir(position)
                while mv and not mv.overlap(self.pieces):
                    self.controlled |= mv.pos
                    mv = dir(mv)
                if mv:
                    self.controlled |= mv.pos
            return

        # calculate moves in each straight direction
        for dir in ROOK_DIRECTIONS:
            mv = dir(position)
            while mv and not mv.overlap(self.pieces):
                moves[mv] = Move(position, mv, MoveType.MOVE, rook)
                mv = dir(mv)
            # if there's an enemy piece at the end, then we could take
            if mv and mv.overlap(self.enemy):
                moves[mv] = Move(position, mv, MoveType.TAKE, rook)
    
    def _calc_queen_moves(self, position: Position, moves: dict[Position, Move]) -> None:
        opposing = position.overlap(self.enemy)
        queen = Piece.WHITE_QUEEN + (6 if position.overlap(self.black_pieces) else 0)

        if opposing:
            for dir in ROOK_DIRECTIONS + BISHOP_DIRECTIONS:
                mv = dir(position)
                while mv and not mv.overlap(self.pieces):
                    self.controlled |= mv.pos
                    mv = dir(mv)
                if mv:
                    self.controlled |= mv.pos
            return

        # calculate moves in each diagonal direction
        for dir in ROOK_DIRECTIONS + BISHOP_DIRECTIONS:
            mv = dir(position)
            while mv and not mv.overlap(self.pieces):
                moves[mv] = Move(position, mv, MoveType.MOVE, queen)
                mv = dir(mv)
            # if there's an enemy piece at the end, then we could take
            if mv and mv.overlap(self.enemy):
                moves[mv] = Move(position, mv, MoveType.TAKE, queen)

    def _calc_king_moves(self, position: Position, moves: dict[Position, Move]) -> None:
        opposing = position.overlap(self.enemy)
        king = Piece.WHITE_KING + (6 if position.overlap(self.black_pieces) else 0)

        if opposing:
            for dir in BISHOP_DIRECTIONS + ROOK_DIRECTIONS:
                mv = dir(position)
                if mv:
                    self.controlled |= mv.pos
            return

        for dir in BISHOP_DIRECTIONS + ROOK_DIRECTIONS:
            mv = dir(position)
            if mv is None or mv.overlap(self.controlled):
                continue
            if not mv.overlap(self.pieces):
                moves[mv] = Move(position, mv, MoveType.MOVE, king)
            elif mv.overlap(self.enemy):
                moves[mv] = Move(position, mv, MoveType.TAKE, king)

    def make_move(self, s: Position, e: Position):
        self._ensure_updated()
        if e in self.all_moves[s]:
            move = self.all_moves[s][e]
            move_type = move.type
            print(f"{move}!")

            if move_type == MoveType.TAKE:
                for i in range(len(self.pieces_list)):
                    self.pieces_list[i] &= ~e.pos

            self.pieces_list[move.piece] &= ~s.pos
            self.pieces_list[move.piece] |= e.pos
            self.turn = ~self.turn
            self.updated = False
        else:
            ex = Move(s, e)
            print(f"{ex} is not a move!")


if __name__=="__main__":
    board = Board()
    print(board)
    board.make_move("e2", "e4")
    print(board)
    board.make_move("e7", "e5")
    print(board)
    board.make_move("d2", "d4")
    print(board)
    board.make_move("e5", "d4")
    print(board)
    print(board.get_valid_moves(Position.get_position("d1")))
    board.make_move("d1", "d4")
    print(board)
    board.make_move("e8", "e7")
    print(board)
    board.make_move("e1", "e2")
    print(board)
    board.make_move("e7", "d6")
    print(board)
