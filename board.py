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

def ctzll(x):
    return (x & -x).bit_length() - 1 if x != 0 else 64

class Board:
    def __init__(self):
        self.white_pieces = [
            sum([1 << i for i in range(48, 56)]),   # white pawns
            (1 << 57) + (1 << 62),                  # white knights
            (1 << 58) + (1 << 61),                  # white bishops
            (1 << 56) + (1 << 63),                  # white rooks
            (1 << 59),                              # white queens
            (1 << 60),                              # white kings
        ]
        self.black_pieces = [
            sum([1 << i for i in range(8, 16)]),    # black pawns
            (1 << 1) + (1 << 6),                    # black knights
            (1 << 2) + (1 << 5),                    # black bishops
            (1 << 0) + (1 << 7),                    # black rooks
            (1 << 3),                               # black queens
            (1 << 4),                               # black kings
        ]
        self.pieces_list = [self.white_pieces, self.black_pieces]

        self.pieces = 0
        self.white_mask = 0
        self.black_mask = 0
        self.turn = Turn.WHITE
        self.enemy = self.black_mask
        self.all_moves = dict()
        self.en_passant = None

        self.CALC_CONTROLLED_FNCS = [
            self._calc_pawn_controlled, 
            self._calc_knight_controlled, 
            self._calc_bishop_controlled, 
            self._calc_rook_controlled, 
            self._calc_queen_controlled,
            self._calc_king_controlled,
        ]

        self.CALC_MOVES_FNCS = [
            self._calc_pawn_moves, 
            self._calc_knight_moves,
            self._calc_bishop_moves,
            self._calc_rook_moves,
            self._calc_queen_moves,
            self._calc_king_moves,
        ]

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
        
        if position.overlap(self.white_mask):
            for i, p in enumerate(self.white_pieces):
                if position.overlap(p):
                    return Piece(i)
        elif position.overlap(self.black_mask):
            for i, p in enumerate(self.black_pieces):
                if position.overlap(p):
                    return Piece(i + COLOR_DELIMITER)

    def _print_piece(self, position: Position) -> str:
        self._ensure_updated()
        representations = ['p', 'n', 'b', 'r', 'q', 'k', ' ']
        piece = self.get_piece(position)
        piece_repr = piece % COLOR_DELIMITER if piece != Piece.EMPTY else len(representations) - 1
        return representations[piece_repr]

    def _ensure_updated(self) -> None:
        if self.updated:
            return
        
        self.white_mask = 0
        for p in self.white_pieces:
            self.white_mask |= p
        self.black_mask = 0
        for p in self.black_pieces:
            self.black_mask |= p
        self.pieces = self.black_mask | self.white_mask

        if self.turn == Turn.WHITE:
            self.enemy = self.black_mask
            self.enemy_list = self.black_pieces
            self.friendly_list = self.white_pieces
        elif self.turn == Turn.BLACK:
            self.enemy = self.white_mask
            self.enemy_list = self.white_pieces
            self.friendly_list = self.black_pieces

        self.in_check = False
        self.calculate_all_valid_moves()

        self.updated = True

    def calculate_all_valid_moves(self) -> dict[Position, dict[Position, Move]]:
        if self.updated:
            return self.all_moves
        
        # need to iterate through enemy pieces first to get controlled squares for king
        self.controlled = 0
        for p, fnc in zip(self.enemy_list, self.CALC_CONTROLLED_FNCS):
            pieces = p
            while pieces > 0:
                fnc(Position(ctzll(pieces)))
                pieces &= pieces - 1
            
        # now, iterate through own pieces to get valid moves
        self.all_moves = dict()
        for p, fnc in zip(self.friendly_list, self.CALC_MOVES_FNCS):
            pieces = p
            while pieces > 0:
                start = Position(ctzll(pieces))
                self.all_moves[start] = fnc(start)
                pieces &= pieces - 1
        return self.all_moves

    def get_valid_moves(self, position: Position) -> dict[Position, Move]:
        self._ensure_updated()
        if position in self.all_moves:
            return self.all_moves[position]
        return None

    def _calc_pawn_controlled(self, position: Position) -> None:
        if position.overlap(self.white_mask):
            pawn_take_left = Position.up_left
            pawn_take_right = Position.up_right
        elif position.overlap(self.black_mask):
            pawn_take_left = Position.down_right
            pawn_take_right = Position.down_left

        for pawn_take in [pawn_take_left, pawn_take_right]:
            take = pawn_take(position)
            if take:
                self.controlled |= take.pos

    def _calc_pawn_moves(self, position: Position) -> dict[Position, Move]:
        if position.overlap(self.white_mask):
            pawn_forward = Position.up
            pawn_take_left = Position.up_left
            pawn_take_right = Position.up_right
        elif position.overlap(self.black_mask):
            pawn_forward = Position.down
            pawn_take_left = Position.down_right
            pawn_take_right = Position.down_left
        moves = dict()
        # calculate forward pawn moves
        forward = pawn_forward(position)
        if forward and not forward.overlap(self.pieces):
            moves[forward] = Move(position, forward, MoveType.MOVE, Piece.PAWN)
            fforward = pawn_forward(forward)
            if ((self.turn == Turn.WHITE and position.rank() == 2) or (self.turn == Turn.BLACK and position.rank() == 7)) \
                    and fforward and not fforward.overlap(self.pieces):
                moves[fforward] = Move(position, fforward, MoveType.PAWN_TWO_MOVE, Piece.PAWN)
        # calculate taking on left and right
        for pawn_take in [pawn_take_left, pawn_take_right]:
            take = pawn_take(position)
            if take is None:
                continue
            if take.overlap(self.enemy):
                moves[take] = Move(position, take, MoveType.TAKE, Piece.PAWN)
            elif self.en_passant and take == self.en_passant:
                moves[take] = Move(position, take, MoveType.EN_PASSANT, Piece.PAWN)
        return moves

    def _calc_knight_controlled(self, position: Position) -> None:
        for dir in KNIGHT_DIRECTIONS:
            mv = dir(position)
            if mv:
                self.controlled |= mv.pos

    def _calc_knight_moves(self, position: Position) -> dict[Position, Move]:
        moves = dict()
        for dir in KNIGHT_DIRECTIONS:
            mv = dir(position)
            if mv is None:
                continue
            # calculate unoccupied squares
            if not mv.overlap(self.pieces):
                moves[mv] = Move(position, mv, MoveType.MOVE, Piece.KNIGHT)
            # calculate taking enemy pieces
            elif mv.overlap(self.enemy):
                moves[mv] = Move(position, mv, MoveType.TAKE, Piece.KNIGHT)
        return moves

    def _calc_bishop_controlled(self, position: Position) -> None:
        for dir in BISHOP_DIRECTIONS:
            mv = dir(position)
            # need to account for bishop piercing through the king
            while mv and (not mv.overlap(self.pieces) or mv.overlap(self.friendly_list[Piece.KING])):
                self.controlled |= mv.pos
                mv = dir(mv)
            if mv:
                self.controlled |= mv.pos

    def _calc_bishop_moves(self, position: Position) -> dict[Position, Move]:
        moves = dict()
        # calculate moves in each diagonal direction
        for dir in BISHOP_DIRECTIONS:
            mv = dir(position)
            while mv and not mv.overlap(self.pieces):
                moves[mv] = Move(position, mv, MoveType.MOVE, Piece.BISHOP)
                mv = dir(mv)
            # if there's an enemy piece at the end, then we could take
            if mv and mv.overlap(self.enemy):
                moves[mv] = Move(position, mv, MoveType.TAKE, Piece.BISHOP)
        return moves
    
    def _calc_rook_controlled(self, position: Position) -> None:
        for dir in ROOK_DIRECTIONS:
            mv = dir(position)
            # need to account for rook piercing through the king
            while mv and (not mv.overlap(self.pieces) or mv.overlap(self.friendly_list[Piece.KING])):
                self.controlled |= mv.pos
                mv = dir(mv)
            if mv:
                self.controlled |= mv.pos

    def _calc_rook_moves(self, position: Position) -> dict[Position, Move]:
        moves = dict()
        # calculate moves in each straight direction
        for dir in ROOK_DIRECTIONS:
            mv = dir(position)
            while mv and not mv.overlap(self.pieces):
                moves[mv] = Move(position, mv, MoveType.MOVE, Piece.ROOK)
                mv = dir(mv)
            # if there's an enemy piece at the end, then we could take
            if mv and mv.overlap(self.enemy):
                moves[mv] = Move(position, mv, MoveType.TAKE, Piece.ROOK)
        return moves

    def _calc_queen_controlled(self, position: Position) -> None:
        for dir in ROOK_DIRECTIONS + BISHOP_DIRECTIONS:
            mv = dir(position)
            # need to account for queen piercing through the king
            while mv and (not mv.overlap(self.pieces) or mv.overlap(self.friendly_list[Piece.KING])):
                self.controlled |= mv.pos
                mv = dir(mv)
            if mv:
                self.controlled |= mv.pos

    def _calc_queen_moves(self, position: Position) -> dict[Position, Move]:
        moves = dict()
        # calculate moves in each diagonal direction
        for dir in ROOK_DIRECTIONS + BISHOP_DIRECTIONS:
            mv = dir(position)
            while mv and not mv.overlap(self.pieces):
                moves[mv] = Move(position, mv, MoveType.MOVE, Piece.QUEEN)
                mv = dir(mv)
            # if there's an enemy piece at the end, then we could take
            if mv and mv.overlap(self.enemy):
                moves[mv] = Move(position, mv, MoveType.TAKE, Piece.QUEEN)
        return moves

    def _calc_king_controlled(self, position: Position) -> None:
        for dir in BISHOP_DIRECTIONS + ROOK_DIRECTIONS:
            mv = dir(position)
            if mv:
                self.controlled |= mv.pos

    def _calc_king_moves(self, position: Position) -> dict[Position, Move]:
        moves = dict()
        for dir in BISHOP_DIRECTIONS + ROOK_DIRECTIONS:
            mv = dir(position)
            if mv is None or mv.overlap(self.controlled):
                continue
            if not mv.overlap(self.pieces):
                moves[mv] = Move(position, mv, MoveType.MOVE, Piece.KING)
            elif mv.overlap(self.enemy):
                moves[mv] = Move(position, mv, MoveType.TAKE, Piece.KING)
        return moves

    def make_move(self, s: Position, e: Position):
        self._ensure_updated()
        if e in self.all_moves[s]:
            move = self.all_moves[s][e]
            print(f"{move}!")

            # erase end square
            if move.type == MoveType.TAKE:
                for i in range(len(self.white_pieces)):
                    self.white_pieces[i] &= ~e.pos
                for i in range(len(self.black_pieces)):
                    self.black_pieces[i] &= ~e.pos
            elif move.type == MoveType.EN_PASSANT:
                taken_pawn = e.down() if self.turn == Turn.WHITE else e.up()
                for i in range(len(self.white_pieces)):
                    self.white_pieces[i] &= ~taken_pawn.pos
                for i in range(len(self.black_pieces)):
                    self.black_pieces[i] &= ~taken_pawn.pos
            
            # if a pawn was moved two squares, then allow for en_passant
            if move.type == MoveType.PAWN_TWO_MOVE:
                self.en_passant = e.down() if self.turn == Turn.WHITE else e.up()
            else:
                self.en_passant = None

            # erase start square
            self.pieces_list[1 - self.turn.value][move.piece] &= ~s.pos
            # place piece at end square
            self.pieces_list[1 - self.turn.value][move.piece] |= e.pos

            self.turn = ~self.turn
            self.updated = False
        else:
            ex = Move(s, e)
            print(f"{ex} is not a move!")

if __name__=="__main__":
    board = Board()
    print(board)
    board.make_move(Position.get_position("e2"), Position.get_position("e4"))
    print(board)
    # board.make_move(Position.get_position("e7"), Position.get_position("e5"))
    # print(board)
    # board.make_move(Position.get_position("d2"), Position.get_position("d4"))
    # print(board)
    # board.make_move(Position.get_position("e5"), Position.get_position("d4"))
    # print(board)
    # print(board.get_valid_moves(Position.get_position("d1")))
    # board.make_move(Position.get_position("d1"), Position.get_position("d4"))
    # print(board)
    # board.make_move(Position.get_position("e8"), Position.get_position("e7"))
    # print(board)
    # board.make_move(Position.get_position("e1"), Position.get_position("e2"))
    # print(board)
    # board.make_move(Position.get_position("e7"), Position.get_position("d6"))
    # print(board)
