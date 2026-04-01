/**
 * UCI (Universal Chess Interface) mode for ChessLi engine.
 * Run with --uci to communicate via stdin/stdout.
 * No SFML or GUI - standalone engine process.
 */

#include <iostream>
#include <sstream>
#include <string>

#include "board.hpp"
#include "engine.hpp"

static Board board;
static Engine engine(&board);

static void cmd_uci() {
    std::cout << "id name ChessLi" << std::endl;
    std::cout << "id author Pr0ph3t" << std::endl;
    std::cout << "uciok" << std::endl;
}

static void cmd_isready() {
    std::cout << "readyok" << std::endl;
}

static void cmd_position(const std::string& line) {
    // position startpos | position fen <fen> [moves m1 m2 ...]
    std::istringstream iss(line);
    std::string token;
    iss >> token;  // "position"
    iss >> token;  // "startpos" or "fen"

    if (token == "startpos") {
        board.set_fen(Board::STARTING_BOARD);
        iss >> token;  // may be "moves" or nothing
    } else if (token == "fen") {
        std::string fen;
        while (iss >> token && token != "moves") {
            if (!fen.empty()) fen += ' ';
            fen += token;
        }
        if (!fen.empty()) {
            board.set_fen(fen);
        }
    } else {
        return;
    }

    if (token != "moves") return;
    std::string uci_move;
    while (iss >> uci_move) {
        auto maybe_move = Move::from_uci(uci_move);
        if (!maybe_move) continue;
        auto moves = board.get_moves();
        const Move& m = *maybe_move;
        const Move* to_apply = nullptr;
        for (const auto& legal : moves) {
            if (legal.start() == m.start() && legal.end() == m.end()) {
                if (legal.flag() == m.flag() || (!legal.is_promotion() && !m.is_promotion())) {
                    to_apply = &legal;
                    break;
                }
            }
        }
        if (!to_apply) break;
        board.make_move(to_apply);
    }
}

static void cmd_go(const std::string& line) {
    // go depth N
    int depth = Engine::DEFAULT_DEPTH;
    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {
        if (token == "depth" && iss >> depth) break;
    }
    if (depth < 1) depth = 1;
    if (depth > Engine::MAX_DEPTH) depth = Engine::MAX_DEPTH;

    auto moves = board.get_moves();
    if (moves.empty()) {
        std::cout << "bestmove (none)" << std::endl;
        return;
    }

    Move best = engine.get_best_move(depth);
    std::cout << "bestmove " << best.to_uci() << std::endl;
}

static void cmd_undo(const std::string& line) {
    // undo N
    int plies = 1;
    std::istringstream iss(line);
    std::string token;
    iss >> token;  // "undo"
    iss >> plies;
    if (plies < 1) plies = 1;

    int undone = 0;
    while (undone < plies && board.get_ply_count() > 0) {
        board.undo_move();
        undone++;
    }
    std::cout << "undook" << std::endl;
}

static void cmd_getfen() {
    std::cout << "fen " << board.get_fen() << std::endl;
}

int main(int argc, char* argv[]) {
    // Check for --uci flag (optional; if no args, assume UCI mode for subprocess use)
    bool uci_mode = (argc <= 1);
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--uci") {
            uci_mode = true;
            break;
        }
    }

    if (!uci_mode) {
        std::cerr << "Run with --uci for UCI protocol mode." << std::endl;
        return 1;
    }

    board.set_fen(Board::STARTING_BOARD);

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "uci") {
            cmd_uci();
        } else if (cmd == "isready") {
            cmd_isready();
        } else if (cmd == "position") {
            cmd_position(line);
        } else if (cmd == "go") {
            cmd_go(line);
        } else if (cmd == "stop") {
            // We don't support pondering; ignore
        } else if (cmd == "quit") {
            break;
        } else if (cmd == "undo") {
            cmd_undo(line);
        } else if (cmd == "getfen") {
            cmd_getfen();
        }
    }

    return 0;
}
