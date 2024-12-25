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

    def _print_piece(self, position: Position) -> str:
        self._ensure_updated()
        if not (position.overlap(self.pieces)):
            return " "
        
        if position.overlap(self.white_pieces):
            # is white
            pass
        if position.overlap(self.black_pieces):
            # is black
            pass
        
        representations = ['p', 'n', 'b', 'r', 'q', 'k']
        for p, r in zip(self.pieces_list[:COLOR_DELIMITER], representations):
            if position.overlap(p):
                return r
        for p, r in zip(self.pieces_list[COLOR_DELIMITER:], representations):
            if position.overlap(p):
                return r

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

        self.calculate_all_valid_moves()

        self.updated = True

    def calculate_all_valid_moves(self) -> dict[Position, dict[Position, Move]]:
        if self.updated:
            return self.all_moves

        self.controlled = 0
        for f in FILES:
            for r in RANKS:
                start = Position.get_position(f"{f}{r}")
                moves = self.calculate_valid_moves(start)
                self.all_moves[start] = moves
        return self.all_moves

    def calculate_valid_moves(self, position: Position) -> dict[Position, Move]:
        if self.updated:
            return self.all_moves[position]

        moves = dict()
        if not position.overlap(self.pieces):
            return moves
        
        if self.turn == Turn.WHITE and position.overlap(self.white_pieces):
            my_pawns = self.pieces_list[Piece.WHITE_PAWN]
            pawn_forward = Position.up
            pawn_take_right = Position.up_right
            pawn_take_left = Position.up_left
            offset = 0
            enemy = self.black_pieces
        elif self.turn == Turn.BLACK and position.overlap(self.black_pieces):
            my_pawns = self.pieces_list[Piece.BLACK_PAWN]
            pawn_forward = Position.down
            pawn_take_right = Position.down_left
            pawn_take_left = Position.down_right
            offset = 6
            enemy = self.white_pieces
        else:
            return moves
        
        if position.overlap(my_pawns):
            pawn = Piece.WHITE_PAWN + offset
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
                self.controlled |= take.pos
                if take.overlap(enemy):
                    moves[take] = Move(position, take, MoveType.TAKE, pawn)
        
        elif position.overlap(self.pieces_list[Piece.WHITE_KNIGHT] | self.pieces_list[Piece.BLACK_KNIGHT]):
            knight = Piece.WHITE_KNIGHT + offset
            for dir in KNIGHT_DIRECTIONS:
                mv = dir(position)
                if mv is None:
                    continue
                self.controlled |= mv.pos
                # calculate unoccupied squares
                if not mv.overlap(self.pieces):
                    moves[mv] = Move(position, mv, MoveType.MOVE, knight)
                # calculate taking enemy pieces
                if mv.overlap(enemy):
                    moves[mv] = Move(position, mv, MoveType.TAKE, knight)
        
        elif position.overlap(self.pieces_list[Piece.WHITE_BISHOP] | self.pieces_list[Piece.BLACK_BISHOP]):
            bishop = Piece.WHITE_BISHOP + offset
            # calculate moves in each diagonal direction
            for dir in BISHOP_DIRECTIONS:
                mv = dir(position)
                while mv and not mv.overlap(self.pieces):
                    self.controlled |= mv.pos
                    moves[mv] = Move(position, mv, MoveType.MOVE, bishop)
                    mv = dir(mv)
                # if there's an enemy piece at the end, then we could take
                if mv is None:
                    continue
                self.controlled |= mv.pos
                if mv.overlap(enemy):
                    moves[mv] = Move(position, mv, MoveType.TAKE, bishop)

        elif position.overlap(self.pieces_list[Piece.WHITE_ROOK] | self.pieces_list[Piece.BLACK_ROOK]):
            rook = Piece.WHITE_ROOK + offset
            # calculate moves in each straight direction
            for dir in ROOK_DIRECTIONS:
                mv = dir(position)
                while mv and not mv.overlap(self.pieces):
                    self.controlled |= mv.pos
                    moves[mv] = Move(position, mv, MoveType.MOVE, rook)
                    mv = dir(mv)
                # if there's an enemy piece at the end, then we could take
                if mv is None:
                    continue
                self.controlled |= mv.pos
                if mv.overlap(enemy):
                    moves[mv] = Move(position, mv, MoveType.TAKE, rook)

        elif position.overlap(self.pieces_list[Piece.WHITE_QUEEN] | self.pieces_list[Piece.BLACK_QUEEN]):
            queen = Piece.WHITE_QUEEN + offset
            # calculate moves in each diagonal direction
            for dir in ROOK_DIRECTIONS + BISHOP_DIRECTIONS:
                mv = dir(position)
                while mv and not mv.overlap(self.pieces):
                    self.controlled |= mv.pos
                    moves[mv] = Move(position, mv, MoveType.MOVE, queen)
                    mv = dir(mv)
                # if there's an enemy piece at the end, then we could take
                if mv is None:
                    continue
                self.controlled |= mv.pos
                if mv.overlap(enemy):
                    moves[mv] = Move(position, mv, MoveType.TAKE, queen)

        elif position.overlap(self.pieces_list[Piece.WHITE_KING] | self.pieces_list[Piece.BLACK_KING]):
            king = Piece.WHITE_KING + offset
            for dir in BISHOP_DIRECTIONS + ROOK_DIRECTIONS:
                mv = dir(position)
                if mv is None:
                    continue
                self.controlled |= 
                if not mv.overlap(self.pieces):
                    moves[mv] = Move(position, mv, MoveType.MOVE, king)
                elif mv.overlap(enemy):
                    moves[mv] = Move(position, mv, MoveType.TAKE, king)

        return moves

    def make_move(self, start: str, end: str):
        self._ensure_updated()
        s = Position.get_position(start)
        e = Position.get_position(end)
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
            print("not a move!")

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
print(board.calculate_valid_moves(Position.get_position("d1")))
board.make_move("d1", "d4")
print(board)