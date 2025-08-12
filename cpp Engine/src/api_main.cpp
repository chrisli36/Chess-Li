#include "crow.h"
#include "nlohmann/json.hpp"
#include "adapter.hpp"
#include "engine.hpp"
#include "board.hpp"
#include <iostream>
#include <memory>

using json = nlohmann::json;

int main() {
    crow::SimpleApp app;
    
    // Enable CORS for development
    app.handle_global().add_after(crow::CORSHandler());
    
    // POST /bestmove endpoint
    CROW_ROUTE(app, "/bestmove").methods("POST"_method)
    ([](const crow::request& req) {
        try {
            auto body = json::parse(req.body);
            
            // Validate required fields
            if (!body.contains("fen") || !body["fen"].is_string()) {
                return crow::response(400, "{\"error\": \"Missing or invalid 'fen' field\"}");
            }
            
            std::string fen = body["fen"];
            int depth = body.value("depth", 4); // Default depth 4
            
            // Validate depth
            if (depth < 1 || depth > 12) {
                return crow::response(400, "{\"error\": \"Depth must be between 1 and 12\"}");
            }
            
            // Create board and engine
            Board board(fen);
            Engine engine(&board);
            
            // Get best move
            Move best_move = engine.get_best_move(depth);
            
            // Convert to JSON response
            json response;
            response["bestMove"] = ChessAdapter::move_to_json(best_move, board);
            response["score"] = ChessAdapter::score_to_json(0); // TODO: Get actual score from engine
            
            return crow::response(200, response.dump());
            
        } catch (const json::parse_error& e) {
            return crow::response(400, "{\"error\": \"Invalid JSON\"}");
        } catch (const std::exception& e) {
            std::cerr << "Error in /bestmove: " << e.what() << std::endl;
            return crow::response(500, "{\"error\": \"Internal server error\"}");
        }
    });
    
    // POST /move endpoint
    CROW_ROUTE(app, "/move").methods("POST"_method)
    ([](const crow::request& req) {
        try {
            auto body = json::parse(req.body);
            
            // Validate required fields
            if (!body.contains("fen") || !body["fen"].is_string()) {
                return crow::response(400, "{\"error\": \"Missing or invalid 'fen' field\"}");
            }
            if (!body.contains("move") || !body["move"].is_string()) {
                return crow::response(400, "{\"error\": \"Missing or invalid 'move' field\"}");
            }
            
            std::string fen = body["fen"];
            std::string move_str = body["move"];
            
            // Create board
            Board board(fen);
            
            // Parse move
            auto move = ChessAdapter::algebraic_to_move(move_str, board);
            if (!move) {
                return crow::response(400, "{\"error\": \"Invalid move format\"}");
            }
            
            // Check if move is legal
            auto legal_moves = board.get_moves();
            bool is_legal = false;
            for (const auto& legal_move : legal_moves) {
                if (legal_move.start() == move->start() && 
                    legal_move.end() == move->end() && 
                    legal_move.flag() == move->flag()) {
                    is_legal = true;
                    break;
                }
            }
            
            if (!is_legal) {
                json response;
                response["legal"] = false;
                response["fen"] = nullptr;
                response["status"] = "ongoing";
                response["lastMove"] = move_str;
                return crow::response(200, response.dump());
            }
            
            // Make the move
            board.make_move(&(*move));
            
            // Return response
            json response = ChessAdapter::board_status_to_json(board, move_str);
            return crow::response(200, response.dump());
            
        } catch (const json::parse_error& e) {
            return crow::response(400, "{\"error\": \"Invalid JSON\"}");
        } catch (const std::exception& e) {
            std::cerr << "Error in /move: " << e.what() << std::endl;
            return crow::response(500, "{\"error\": \"Internal server error\"}");
        }
    });
    
    // WebSocket endpoint for search
    CROW_WEBSOCKET_ROUTE(app, "/ws/search")
    .onopen([](crow::websocket::connection& conn) {
        std::cout << "WebSocket connection opened" << std::endl;
    })
    .onmessage([](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
        try {
            auto msg = json::parse(data);
            
            if (!msg.contains("fen") || !msg["fen"].is_string()) {
                json error = {{"error", "Missing or invalid 'fen' field"}};
                conn.send_text(error.dump());
                return;
            }
            
            std::string fen = msg["fen"];
            int depth = msg.value("depth", 4);
            
            if (depth < 1 || depth > 12) {
                json error = {{"error", "Depth must be between 1 and 12"}};
                conn.send_text(error.dump());
                return;
            }
            
            // Create board and engine
            Board board(fen);
            Engine engine(&board);
            
            // TODO: Implement streaming search info
            // For now, just send the best move
            Move best_move = engine.get_best_move(depth);
            
            json response;
            response["type"] = "bestmove";
            response["bestMove"] = ChessAdapter::move_to_algebraic(best_move, board).long_notation;
            
            conn.send_text(response.dump());
            
        } catch (const json::parse_error& e) {
            json error = {{"error", "Invalid JSON"}};
            conn.send_text(error.dump());
        } catch (const std::exception& e) {
            std::cerr << "WebSocket error: " << e.what() << std::endl;
            json error = {{"error", "Internal server error"}};
            conn.send_text(error.dump());
        }
    })
    .onclose([](crow::websocket::connection& conn, const std::string& reason) {
        std::cout << "WebSocket connection closed: " << reason << std::endl;
    });
    
    // Health check endpoint
    CROW_ROUTE(app, "/health")
    ([]() {
        return "OK";
    });
    
    // Set up the server
    app.port(8080).multithreaded().run();
    
    return 0;
}
