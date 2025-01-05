from position import *
from moves import *
from enum import IntEnum, Enum
from utils import *
from magic import ROOK_MAGIC, QUEEN_MAGIC, BISHOP_MAGIC, KNIGHT_MAGIC
from typing import Optional

ROOK_DIRECTIONS = [Position.up, Position.down, Position.right, Position.left]
BISHOP_DIRECTIONS = [Position.up_left, Position.up_right, Position.down_right, Position.down_left]
KNIGHT_DIRECTIONS = [
    Position.up_up_right, Position.up_right_right, Position.down_right_right, Position.down_down_right,
    Position.down_left_left, Position.down_down_left, Position.up_left_left, Position.up_up_left
]

PROMOTIONS = [Piece.QUEEN, Piece.ROOK, Piece.BISHOP, Piece.KNIGHT]

class Turn(IntEnum):
    WHITE = 0
    BLACK = 1

class GameState(Enum):
    PLAYING = 0
    WHITE = 1
    DRAW = 2
    BLACK = 3

class StateChange(Enum):
    PROMOTION = 0
    PAWN_TWO = 1
    EN_PASSANT = 2
    K = 3
    Q = 4
    k = 5
    q = 6
    K_CASTLE = 7
    Q_CASTLE = 8
    k_CASTLE = 9
    q_CASTLE = 10

class DEFAULT(Enum):
    white_pieces = [
        sum([1 << i for i in range(48, 56)]),   # white pawns
        (1 << 57) + (1 << 62),                  # white knights
        (1 << 58) + (1 << 61),                  # white bishops
        (1 << 56) + (1 << 63),                  # white rooks
        (1 << 59),                              # white queens
        (1 << 60),                              # white kings
    ]
    black_pieces = [
        sum([1 << i for i in range(8, 16)]),    # black pawns
        (1 << 1) + (1 << 6),                    # black knights
        (1 << 2) + (1 << 5),                    # black bishops
        (1 << 0) + (1 << 7),                    # black rooks
        (1 << 3),                               # black queens
        (1 << 4),                               # black kings
    ]
    K = (1 << 61) + (1 << 62)
    Q = (1 << 59) + (1 << 58)
    k = (1 << 5) + (1 << 6)
    q = (1 << 3) + (1 << 2)
    turn = Turn.WHITE

class Board:
    def __init__(self, white_pieces=DEFAULT.white_pieces.value, black_pieces=DEFAULT.black_pieces.value,
            K=DEFAULT.K.value, Q=DEFAULT.Q.value, k=DEFAULT.k.value, q=DEFAULT.q.value, turn=DEFAULT.turn.value):
        # piece bitboards
        self.white_pieces = white_pieces
        self.black_pieces = black_pieces
        self.pieces_list = [self.white_pieces, self.black_pieces]

        # castling rights bitboards
        self.K = K
        self.Q = Q
        self.k = k
        self.q = q

        # internal variables that are adjusted each turn
        self.castle_king = 0
        self.castle_queen = 0

        self.pieces_mask = 0
        self.white_mask = 0
        self.black_mask = 0

        self.turn = turn
        self.friend = 0
        self.enemy = 0
        self.friend_list = None
        self.enemy_list = None

        self.all_moves = None
        self.has_moves = None
        self.pinned_limits = None
        self.en_passant = None
        self.attackers = None

        self.state_changes = None
        self.game_state = GameState.PLAYING

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
        return position.overlap(self.pieces_mask) > 0

    def is_enemy_controlled(self, position: Position) -> bool:
        self._ensure_updated()
        return position.overlap(self.controlled) > 0

    def get_piece(self, position: Position) -> Piece:
        self._ensure_updated()
        if not (position.overlap(self.pieces_mask)):
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

    def _reset_state(self) -> None:
        self.white_mask = 0
        for p in self.white_pieces:
            self.white_mask |= p
        self.black_mask = 0
        for p in self.black_pieces:
            self.black_mask |= p
        self.pieces_mask = self.black_mask | self.white_mask

        if self.turn == Turn.WHITE:
            self.friend = self.white_mask
            self.enemy = self.black_mask
            self.enemy_list = self.black_pieces
            self.friend_list = self.white_pieces
            self.castle_king = self.K
            self.castle_queen = self.Q
        elif self.turn == Turn.BLACK:
            self.friend = self.black_mask
            self.enemy = self.white_mask
            self.enemy_list = self.white_pieces
            self.friend_list = self.black_pieces
            self.castle_king = self.k
            self.castle_queen = self.q
        self.in_check = False

    def _ensure_updated(self) -> None:
        if self.updated:
            return

        self._reset_state()
        self.calculate_all_moves()

        self.updated = True

    def _get_stop_check(self, king: Position, attacker: Position) -> int:
        if attacker.overlap(self.white_pieces[Piece.KNIGHT] | self.black_pieces[Piece.KNIGHT]):
            return 0
        # elif attacker.overlap(self.white_pieces[Piece.ROOK] | self.black_pieces[Piece.ROOK]):
        # elif attacker.overlap(self.white_pieces[Piece.BISHOP] | self.black_pieces[Piece.BISHOP]):
        diff = king.num - attacker.num
        sign = 1 if diff > 0 else -1
        step = None
        if attacker.num // 8 == king.num // 8:
            step = 1 * sign
        elif (king.num - attacker.num) % 8 == 0:
            step = 8 * sign
        elif (king.num - attacker.num) % 7 == 0:
            step = 7 * sign
        elif (king.num - attacker.num) % 9 == 0:
            step = 9 * sign
        
        nums = range(attacker.num, king.num, step)
        return sum([1 << n for n in nums])

    def _update_pinned(self):
        king_position = Position(ctzll(self.friend_list[Piece.KING]))
        self.pinned_limits = dict()

        # iterate through enemy 'piercing' pieces
        for piece, piece_magic in zip([Piece.BISHOP, Piece.ROOK, Piece.QUEEN], [BISHOP_MAGIC, ROOK_MAGIC, QUEEN_MAGIC]):
            p = self.enemy_list[piece]
            while p > 0:
                p_position = Position(ctzll(p))
                # checks if the piece can see the king, ignoring pieces in the way
                if piece_magic[p_position.num] & self.friend_list[Piece.KING]:
                    sc = self._get_stop_check(king_position, p_position)
                    # checks if there's 1 friend (the pinned piece) and 1 enemy (the piercing piece)
                    if (sc & self.friend).bit_count() == 1 and (sc & self.enemy).bit_count() == 1:
                        self.pinned_limits[Position(ctzll(sc & self.friend))] = sc
                    # checks for the rare case where en passant is 
                    # not allowed if two pawns are pinned horizontally
                    elif (sc & self.friend).bit_count() == 1 and (sc & self.enemy).bit_count() == 2:
                        enemy_pawn = sc & self.enemy & self.enemy_list[Piece.PAWN]
                        friend_pawn = sc & self.friend & self.friend_list[Piece.PAWN]
                        if enemy_pawn.bit_count() == 1 and friend_pawn.bit_count() == 1 \
                                and (enemy_pawn == friend_pawn << 1 or enemy_pawn == friend_pawn >> 1):
                            self.pinned_limits[Position(ctzll(friend_pawn))] = (friend_pawn << 8) + (friend_pawn >> 8)
                            print("WOWWWW: limiting en passant power!")
                p &= p - 1

    def calculate_all_moves(self) -> dict[Position, Moves]:
        if self.updated:
            return self.all_moves
        self._reset_state()

        # need to iterate through enemy pieces first to get controlled squares for king
        self.attackers = []
        self.controlled = 0
        for p, fnc in zip(self.enemy_list, self.CALC_CONTROLLED_FNCS):
            pieces = p
            while pieces > 0:
                fnc(Position(ctzll(pieces)))
                pieces &= pieces - 1

        # need to also iterate through enemy sights to get pinned pieces
        self._update_pinned()

        # now, iterate through own pieces to get valid moves
        self.all_moves = dict()
        for p, fnc in zip(self.friend_list, self.CALC_MOVES_FNCS):
            pieces = p
            while pieces > 0:
                start = Position(ctzll(pieces))
                self.all_moves[start] = fnc(start)
                pieces &= pieces - 1

        # not optimal, but if in check, filter out moves
        if len(self.attackers) == 1:
            king_position = Position(ctzll(self.friend_list[Piece.KING]))
            stop_check = self._get_stop_check(king_position, self.attackers[0])
            for s in self.all_moves.keys():
                if s == king_position:
                    continue
                self.all_moves[s].limit_with_mask(stop_check)
        # if in double check, the king is forced to move
        elif len(self.attackers) >= 2: 
            king_position = Position(ctzll(self.friend_list[Piece.KING]))
            self.all_moves = {king_position: self.all_moves[king_position]}

        # check for game end
        self.has_moves = any(moves.mask for moves in self.all_moves.values())
        if not self.has_moves:
            if self.attackers:
                self.game_state = GameState.WHITE if self.turn else GameState.BLACK
                print(f"""GAME OVER, {"WHITE" if self.turn else "BLACK"} wins!!!""")
            else:
                self.game_state = GameState.DRAW
                print(f"""GAME OVER, DRAW.""")
        
        self.updated = True
        return self.all_moves

    def get_moves(self, position: Position) -> Moves:
        self._ensure_updated()
        if position not in self.all_moves:
            return None
        return self.all_moves[position]

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
                if take.overlap(self.friend_list[Piece.KING]):
                    self.attackers.append(position)
                self.controlled |= take.pos

    def _calc_pawn_moves(self, position: Position) -> Moves:
        moves = Moves(0, Piece.PAWN)
        if position.overlap(self.white_mask):
            pawn_forward = Position.up
            pawn_take_left = Position.up_left
            pawn_take_right = Position.up_right
        elif position.overlap(self.black_mask):
            pawn_forward = Position.down
            pawn_take_left = Position.down_right
            pawn_take_right = Position.down_left
        else:
            print("error in _calc_pawn_moves! DJFKLJDSALKF")
        # calculate forward pawn moves
        forward = pawn_forward(position)
        if forward and not forward.overlap(self.pieces_mask):
            moves.update(forward)
            fforward = pawn_forward(forward)
            if ((self.turn == Turn.WHITE and position.overlap(RANK[2])) \
                    or (self.turn == Turn.BLACK and position.overlap(RANK[7]))) \
                    and fforward and not fforward.overlap(self.pieces_mask):
                moves.update(fforward)
        # calculate taking on left and right
        for pawn_take in [pawn_take_left, pawn_take_right]:
            take = pawn_take(position)
            if take is None:
                continue
            if take.overlap(self.enemy):
                moves.update(take)
            elif self.en_passant and take == self.en_passant:
                moves.update(take)
        if position in self.pinned_limits:
            moves.limit_with_mask(self.pinned_limits[position])
        return moves

    def _calc_knight_controlled(self, position: Position) -> None:
        for dir in KNIGHT_DIRECTIONS:
            mv = dir(position)
            if mv:
                if mv.overlap(self.friend_list[Piece.KING]):
                    self.attackers.append(position)
                self.controlled |= mv.pos

    def _calc_knight_moves(self, position: Position) -> Moves:
        moves = Moves(0, Piece.KNIGHT)
        for dir in KNIGHT_DIRECTIONS:
            mv = dir(position)
            if mv is None:
                continue
            # calculate unoccupied squares
            if not mv.overlap(self.pieces_mask):
                moves.update(mv)
            # calculate taking enemy pieces
            elif mv.overlap(self.enemy):
                moves.update(mv)
        if position in self.pinned_limits:
            moves.limit_with_mask(self.pinned_limits[position])
        return moves

    def _calc_bishop_controlled(self, position: Position) -> None:
        for dir in BISHOP_DIRECTIONS:
            mv = dir(position)
            # need to account for bishop piercing through the king
            while mv and (not mv.overlap(self.pieces_mask) or mv.overlap(self.friend_list[Piece.KING])):
                if mv.overlap(self.friend_list[Piece.KING]):
                    self.attackers.append(position)
                self.controlled |= mv.pos
                mv = dir(mv)
            if mv:
                self.controlled |= mv.pos

    def _calc_bishop_moves(self, position: Position) -> Moves:
        moves = Moves(0, Piece.BISHOP)
        # calculate moves in each diagonal direction
        for dir in BISHOP_DIRECTIONS:
            mv = dir(position)
            while mv and not mv.overlap(self.pieces_mask):
                moves.update(mv)
                mv = dir(mv)
            # if there's an enemy piece at the end, then we could take
            if mv and mv.overlap(self.enemy):
                moves.update(mv)
        if position in self.pinned_limits:
            moves.limit_with_mask(self.pinned_limits[position])
        return moves

    def _calc_rook_controlled(self, position: Position) -> None:
        for dir in ROOK_DIRECTIONS:
            mv = dir(position)
            # need to account for rook piercing through the king
            while mv and (not mv.overlap(self.pieces_mask) or mv.overlap(self.friend_list[Piece.KING])):
                if mv.overlap(self.friend_list[Piece.KING]):
                    self.attackers.append(position)
                self.controlled |= mv.pos
                mv = dir(mv)
            if mv:
                self.controlled |= mv.pos

    def _calc_rook_moves(self, position: Position) -> Moves:
        moves = Moves(0, Piece.ROOK)
        # calculate moves in each straight direction
        for dir in ROOK_DIRECTIONS:
            mv = dir(position)
            while mv and not mv.overlap(self.pieces_mask):
                moves.update(mv)
                mv = dir(mv)
            # if there's an enemy piece at the end, then we could take
            if mv and mv.overlap(self.enemy):
                moves.update(mv)
        if position in self.pinned_limits:
            moves.limit_with_mask(self.pinned_limits[position])
        return moves

    def _calc_queen_controlled(self, position: Position) -> None:
        for dir in ROOK_DIRECTIONS + BISHOP_DIRECTIONS:
            mv = dir(position)
            # need to account for queen piercing through the king
            while mv and (not mv.overlap(self.pieces_mask) or mv.overlap(self.friend_list[Piece.KING])):
                if mv.overlap(self.friend_list[Piece.KING]):
                    self.attackers.append(position)
                self.controlled |= mv.pos
                mv = dir(mv)
            if mv:
                self.controlled |= mv.pos

    def _calc_queen_moves(self, position: Position) -> Moves:
        moves = Moves(0, Piece.QUEEN)
        # calculate moves in each diagonal direction
        for dir in ROOK_DIRECTIONS + BISHOP_DIRECTIONS:
            mv = dir(position)
            while mv and not mv.overlap(self.pieces_mask):
                moves.update(mv)
                mv = dir(mv)
            # if there's an enemy piece at the end, then we could take
            if mv and mv.overlap(self.enemy):
                moves.update(mv)
        if position in self.pinned_limits:
            moves.limit_with_mask(self.pinned_limits[position])
        return moves

    def _calc_king_controlled(self, position: Position) -> None:
        for dir in BISHOP_DIRECTIONS + ROOK_DIRECTIONS:
            mv = dir(position)
            if mv:
                self.controlled |= mv.pos

    def _calc_king_moves(self, position: Position) -> Moves:
        moves = Moves(0, Piece.KING)
        for dir in BISHOP_DIRECTIONS + ROOK_DIRECTIONS:
            mv = dir(position)
            if mv is None or mv.overlap(self.controlled):
                continue
            if not mv.overlap(self.pieces_mask):
                moves.update(mv)
            elif mv.overlap(self.enemy):
                moves.update(mv)
        # check for castling
        if self.castle_king and (self.castle_king & (self.pieces_mask | self.controlled)) == 0:
            castle_kingside = position.right().right()
            moves.update(castle_kingside)
        if self.castle_queen and (self.castle_queen & (self.pieces_mask | self.controlled)) == 0:
            castle_queenside = position.left().left()
            moves.update(castle_queenside)
        return moves

    def is_promoting(self, s: Position, e: Position) -> bool:
        self._ensure_updated()
        is_pawn_move = e.overlap(self.get_moves(s).mask) and s.overlap(self.pieces_list[self.turn][Piece.PAWN])
        is_promoting = e.overlap(RANK[1]) or e.overlap(RANK[8])
        return is_pawn_move and is_promoting

    def rook_disables_castling(self, pos: Position) -> list[StateChange]:
        state_changes = list()
        if pos.overlap(SQUARE_TO_POS['a1'] | SQUARE_TO_POS['h1'] | SQUARE_TO_POS['a8'] | SQUARE_TO_POS['h8']):
            if pos.overlap(SQUARE_TO_POS['a1']) and self.Q:
                self.Q = 0
                state_changes.append(StateChange.Q)
            elif pos.overlap(SQUARE_TO_POS['h1']) and self.K:
                self.K = 0
                state_changes.append(StateChange.K)
            elif pos.overlap(SQUARE_TO_POS['a8']) and self.q:
                self.q = 0
                state_changes.append(StateChange.q)
            elif pos.overlap(SQUARE_TO_POS['h8']) and self.k:
                self.k = 0
                state_changes.append(StateChange.k)
        return state_changes

    def make_move(self, s: Position, e: Position, piece: Piece, promotion: Piece=None) -> tuple[Optional[Piece], list[StateChange]]:
        self._ensure_updated()
        if e.overlap(self.get_moves(s).mask):
            taken = None
            state_changes = list()

            # erase taken square if we're taking a piece
            if e.overlap(self.enemy):
                for i in range(COLOR_DELIMITER):
                    if self.enemy_list[i] & e.pos:
                        self.enemy_list[i] &= ~e.pos
                        taken = Piece(i)
                        break
                # if we're taking a piece in a rook corner, disable castling rights for that side
                state_changes.extend(self.rook_disables_castling(e))

            # if we're moving a pawn, consider en passant
            # Note: if en passant, taken=None and StateChange.EN_PASSANT is added instead
            if s.overlap(self.pieces_list[self.turn][Piece.PAWN]):
                # if pawn is moving two squares, then enable en passant
                if (s.overlap(RANK[2]) and e.overlap(RANK[4])) or (s.overlap(RANK[7]) and e.overlap(RANK[5])):
                    self.en_passant = e.down() if self.turn == Turn.WHITE else e.up()
                    state_changes.append(StateChange.PAWN_TWO)
                # if the pawn is taking en passant, erase the pawn that was en passanted
                elif self.en_passant and e == self.en_passant:
                    taken_pawn = e.down() if self.turn == Turn.WHITE else e.up()
                    self.enemy_list[Piece.PAWN] &= ~taken_pawn.pos
                    state_changes.append(StateChange.EN_PASSANT)
                    self.en_passant = None
                else:
                    self.en_passant = None
            else:
                self.en_passant = None
            
            # if a rook is moved, disable casting rights for that side
            if s.overlap(self.pieces_list[self.turn][Piece.ROOK]):
                state_changes.extend(self.rook_disables_castling(s))

            # if the king is moving two squares, then we're castling
            if s.overlap(self.pieces_list[self.turn][Piece.KING]):
                diff = e.num - s.num
                # king side castling
                if diff == 2:
                    self.friend_list[Piece.ROOK] &= ~e.right().pos
                    self.friend_list[Piece.ROOK] |= e.left().pos
                    if self.turn == Turn.WHITE:
                        state_changes.append(StateChange.K_CASTLE)
                    elif self.turn == Turn.BLACK:
                        state_changes.append(StateChange.k_CASTLE)
                # queen side castling
                elif diff == -2:
                    self.friend_list[Piece.ROOK] &= ~e.left().left().pos
                    self.friend_list[Piece.ROOK] |= e.right().pos
                    if self.turn == Turn.WHITE:
                        state_changes.append(StateChange.Q_CASTLE)
                    elif self.turn == Turn.BLACK:
                        state_changes.append(StateChange.q_CASTLE)

                # since the king was moved, disable castling rights
                if self.turn == Turn.WHITE:
                    if self.K:
                        self.K = 0
                        state_changes.append(StateChange.K)
                    if self.Q:
                        self.Q = 0
                        state_changes.append(StateChange.Q)
                elif self.turn == Turn.BLACK:
                    if self.k:
                        self.k = 0
                        state_changes.append(StateChange.k)
                    if self.q:
                        self.q = 0
                        state_changes.append(StateChange.q)

            # make the move of original piece
            self.friend_list[piece] &= ~s.pos
            self.friend_list[piece] |= e.pos

            # if pawn is being promoted, need to overwrite with promoted piece
            if e.overlap(self.pieces_list[self.turn][Piece.PAWN]) and (e.overlap(RANK[8]) or e.overlap(RANK[1])):
                self.friend_list[piece] &= ~e.pos
                self.friend_list[promotion] |= e.pos
                state_changes.append(StateChange.PROMOTION)

            self.turn = 1 - self.turn
            self.updated = False

            return taken, state_changes
        else:
            print(self)
            print(self.calculate_all_moves())
            print(f"{s} to {e} is not a move!")

    def undo_move(self, s: Position, e: Position, piece: Piece, taken: Optional[Piece], state_changes: list[StateChange]) -> None:
        # undo move of piece
        self.turn = 1 - self.turn
        self.pieces_list[self.turn][piece] &= ~e.pos
        self.pieces_list[self.turn][piece] |= s.pos

        # if a piece was taken, bring it back
        if taken is not None:
            self.pieces_list[1 - self.turn][taken] |= e.pos

        # account for special cases
        for sc in state_changes:
            if sc == StateChange.PAWN_TWO:
                self.en_passant = None
            elif sc == StateChange.EN_PASSANT:
                taken_pawn = e.down() if self.turn == Turn.WHITE else e.up()
                self.pieces_list[1 - self.turn][Piece.PAWN] |= taken_pawn.pos
                self.en_passant = e
            elif sc == StateChange.PROMOTION:
                for promo in PROMOTIONS:
                    self.pieces_list[self.turn][promo] &= ~e.pos
            elif sc == StateChange.K:
                self.K = DEFAULT.K.value
            elif sc == StateChange.Q:
                self.Q = DEFAULT.Q.value
            elif sc == StateChange.k:
                self.k = DEFAULT.k.value
            elif sc == StateChange.q:
                self.q = DEFAULT.q.value
            elif sc == StateChange.K_CASTLE or sc == StateChange.k_CASTLE:
                self.pieces_list[self.turn][Piece.ROOK] |= e.right().pos
                self.pieces_list[self.turn][Piece.ROOK] &= ~e.left().pos
            elif sc == StateChange.Q_CASTLE or sc == StateChange.q_CASTLE:
                self.pieces_list[self.turn][Piece.ROOK] |= e.left().left().pos
                self.pieces_list[self.turn][Piece.ROOK] &= ~e.right().pos

        self.updated = False

if __name__=="__main__":
    board = Board()
    print(board)
    board.make_move(Position.get_position("e2"), Position.get_position("e4"), Piece.PAWN)
    print(board)
    board.undo_move(Position.get_position("e2"), Position.get_position("e4"), Piece.PAWN, None, [])
    print(board)