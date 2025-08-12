#include "engine.hpp"
#include <random>

Engine::Engine(Board* board) : board(board) {}

Move Engine::get_best_move(int depth) {
    int alpha = -MATE, beta = +MATE;
    int best_score = -MATE;

    auto moves = board->get_moves();
    if (moves.empty()) return Move{};
    
    // Reserve capacity to avoid reallocationsa
    moves.reserve(256);

    std::sort(moves.begin(), moves.end(),
              [this](const Move& a, const Move& b){ return score_move(a) > score_move(b); });

    std::vector<Move> best_moves;
    best_moves.reserve(10); // Reserve space for best moves

    for (auto& move : moves) {
        board->make_move(&move);
        int score = -minimax(depth - 1, -beta, -alpha);
        board->undo_move();
        std::cout << "move: " << move.to_string() << " score: " << score << "\n";

        if (score > best_score) {
            best_score = score;
            best_moves.clear();
            // best_moves.emplace_back(move);
            best_moves.push_back(move);
        } else if (score == best_score) {
            // best_moves.emplace_back(move);
            best_moves.push_back(move);
        }

        if (score > alpha) alpha = score;
    }

    for (auto& move : best_moves) {
        std::cout << "\tmove: " << move.to_string() << "\n";
    }

    if (best_moves.size() == 1) return best_moves[0];

    static thread_local std::mt19937 rng(std::random_device{}());
    static thread_local std::uniform_int_distribution<size_t> dist;
    dist.param(std::uniform_int_distribution<size_t>::param_type(0, best_moves.size() - 1));
    return best_moves[dist(rng)];
}

int Engine::score_move(Move move) {
    int end = move.end();
    Piece move_piece = board->get_piece(move.start());
    Piece end_piece = board->get_piece(end);
    Piece promotion_piece = move.promotion_piece(board->get_turn());
    uint8_t move_piece_type = move_piece.get_piece();
    uint8_t end_piece_type = end_piece.get_piece();
    uint8_t promotion_piece_type = promotion_piece.get_piece();

    int score = 0;
    if (!end_piece.is_empty()) {
        score += 10 * PIECE_VALUES[end_piece_type] - PIECE_VALUES[move_piece_type];
    }
    if (!promotion_piece.is_empty()) {
        score += PIECE_VALUES[promotion_piece_type];
    }
    if (board->is_controlled(end)) {
        score -= PIECE_VALUES[move_piece_type];
    }
    return score;
}

int Engine::search(int depth) {
    if (depth == 0) {
        return 1;
    }
    int ret = 0;
    std::vector<Move> moves = board->get_moves();
    moves.reserve(256); // Reserve capacity for typical chess positions
    for (const Move& move : moves) {
        board->make_move(&move);
        ret += search(depth - 1);
        board->undo_move();
    }
    return ret;
}

int Engine::evaluate() {
    GameState game_state = board->get_game_state();
    if (game_state == GameState::WHITE_WIN || game_state == GameState::BLACK_WIN) {
        return -MATE;
    } else if (game_state == GameState::DRAW) {
        return 0;
    }

    int ret = 0;

    // material values
    for (int i = 0; i < 6; ++i) {
        ret += __builtin_popcountll(board->friend_arr[i]) * PIECE_VALUES[i];
        ret -= __builtin_popcountll(board->enemy_arr[i]) * PIECE_VALUES[i];
    }

    // position values
    auto flip = [](int sq){ return 63 - sq; };
    Turn currentTurn = board->get_turn();
    bool whiteToMove = (currentTurn == Turn::WHITE);

    int sq;
    for (int i = 0; i < 6; ++i) {
        CTZLL_ITERATOR(sq, board->friend_arr[i]) {
            ret += POSITION_VALUES[i][ whiteToMove ? sq : flip(sq) ];
        }
        CTZLL_ITERATOR(sq, board->enemy_arr[i]) {
            ret -= POSITION_VALUES[i][ whiteToMove ? flip(sq) : sq ];
        }
    }

    return ret;
}

int Engine::minimax(int depth, int alpha, int beta) {
    if (depth == 0) {
        return evaluate();
    }

    if (board->get_game_state() != GameState::IN_PROGRESS) {
        return evaluate();
    }

    std::vector<Move> moves = board->get_moves();
    std::sort(moves.begin(), moves.end(), 
        [this](const Move& a, const Move& b) {
        return score_move(a) > score_move(b);
    });

    int score;
    for (const Move& move : moves) {
        board->make_move(&move);
        score = -minimax(depth - 1, -beta, -alpha);
        board->undo_move();

        if (score >= beta) {
            return beta;
        }
        alpha = std::max(alpha, score);
    }
    return alpha;
}