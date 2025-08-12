#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "adapter.hpp"
#include "engine.hpp"
#include "board.hpp"

class SimpleHTTPServer {
private:
    int server_fd;
    int port;
    
    std::string read_request(int client_fd) {
        char buffer[4096] = {0};
        int bytes_read = read(client_fd, buffer, 4095);
        if (bytes_read > 0) {
            return std::string(buffer, bytes_read);
        }
        return "";
    }
    
    void send_response(int client_fd, const std::string& response, const std::string& content_type = "application/json") {
        std::string http_response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: " + content_type + "\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Length: " + std::to_string(response.length()) + "\r\n"
            "\r\n" + response;
        
        send(client_fd, http_response.c_str(), http_response.length(), 0);
    }
    
    void send_error(int client_fd, int status_code, const std::string& message) {
        std::string error_response = "{\"error\": \"" + message + "\"}";
        std::string http_response = 
            "HTTP/1.1 " + std::to_string(status_code) + " " + 
            (status_code == 400 ? "Bad Request" : "Internal Server Error") + "\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Length: " + std::to_string(error_response.length()) + "\r\n"
            "\r\n" + error_response;
        
        send(client_fd, http_response.c_str(), http_response.length(), 0);
    }
    
    std::map<std::string, std::string> parse_headers(const std::string& request) {
        std::map<std::string, std::string> headers;
        std::istringstream stream(request);
        std::string line;
        
        // Skip first line (request line)
        std::getline(stream, line);
        
        while (std::getline(stream, line) && line != "\r") {
            if (line.empty() || line == "\r") break;
            
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string key = line.substr(0, colon_pos);
                std::string value = line.substr(colon_pos + 1);
                
                // Remove \r from value
                if (!value.empty() && value.back() == '\r') {
                    value.pop_back();
                }
                
                headers[key] = value;
            }
        }
        
        return headers;
    }
    
    std::string get_body(const std::string& request) {
        size_t body_start = request.find("\r\n\r\n");
        if (body_start != std::string::npos) {
            return request.substr(body_start + 4);
        }
        return "";
    }
    
    void handle_bestmove(int client_fd, const std::string& body) {
        try {
            // Parse JSON using nlohmann/json
            auto body_json = json::parse(body);
            
            if (!body_json.contains("fen") || !body_json["fen"].is_string()) {
                send_error(client_fd, 400, "Missing or invalid 'fen' field");
                return;
            }
            
            // Extract FEN
            fen_start += 7; // Skip "fen":"
            size_t fen_end = body.find("\"", fen_start);
            if (fen_end == std::string::npos) {
                send_error(client_fd, 400, "Invalid FEN format");
                return;
            }
            std::string fen = body.substr(fen_start, fen_end - fen_start);
            
            // Extract depth (default to 4)
            int depth = 4;
            if (depth_start != std::string::npos) {
                depth_start += 8; // Skip "depth":
                size_t depth_end = body.find_first_of(",\"}", depth_start);
                if (depth_end != std::string::npos) {
                    std::string depth_str = body.substr(depth_start, depth_end - depth_start);
                    try {
                        depth = std::stoi(depth_str);
                        if (depth < 1 || depth > 12) depth = 4;
                    } catch (...) {
                        depth = 4;
                    }
                }
            }
            
            // Create board and engine
            Board board(fen);
            Engine engine(&board);
            
            // Get best move
            Move best_move = engine.get_best_move(depth);
            
            // Convert to JSON response
            auto move_info = ChessAdapter::move_to_algebraic(best_move, board);
            auto score_info = ChessAdapter::score_to_json(0); // TODO: Get actual score
            
            json response;
            response["bestMove"]["long"] = move_info.long_notation;
            response["bestMove"]["from"] = move_info.from;
            response["bestMove"]["to"] = move_info.to;
            response["bestMove"]["promo"] = move_info.promo.value_or(nullptr);
            response["score"] = score_info;
            
            send_response(client_fd, response.dump());
            
        } catch (const std::exception& e) {
            std::cerr << "Error in /bestmove: " << e.what() << std::endl;
            send_error(client_fd, 500, "Internal server error");
        }
    }
    
    void handle_move(int client_fd, const std::string& body) {
        try {
            // Simple JSON parsing for the request
            size_t fen_start = body.find("\"fen\":\"");
            size_t move_start = body.find("\"move\":\"");
            
            if (fen_start == std::string::npos) {
                send_error(client_fd, 400, "Missing 'fen' field");
                return;
            }
            if (move_start == std::string::npos) {
                send_error(client_fd, 400, "Missing 'move' field");
                return;
            }
            
            // Extract FEN
            fen_start += 7; // Skip "fen":"
            size_t fen_end = body.find("\"", fen_start);
            if (fen_end == std::string::npos) {
                send_error(client_fd, 400, "Invalid FEN format");
                return;
            }
            std::string fen = body.substr(fen_start, fen_end - fen_start);
            
            // Extract move
            move_start += 7; // Skip "move":"
            size_t move_end = body.find("\"", move_start);
            if (move_end == std::string::npos) {
                send_error(client_fd, 400, "Invalid move format");
                return;
            }
            std::string move_str = body.substr(move_start, move_end - move_start);
            
            // Create board
            Board board(fen);
            
            // Parse move
            auto move = ChessAdapter::algebraic_to_move(move_str, board);
            if (!move) {
                send_error(client_fd, 400, "Invalid move format");
                return;
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
                std::string response = 
                    "{\"legal\":false,\"fen\":null,\"status\":\"ongoing\",\"lastMove\":\"" + move_str + "\"}";
                send_response(client_fd, response);
                return;
            }
            
            // Make the move
            board.make_move(&(*move));
            
            // Return response
            auto response_json = ChessAdapter::board_status_to_json(board, move_str);
            send_response(client_fd, response_json.dump());
            
        } catch (const std::exception& e) {
            std::cerr << "Error in /move: " << e.what() << std::endl;
            send_error(client_fd, 500, "Internal server error");
        }
    }
    
    void handle_request(int client_fd) {
        std::string request = read_request(client_fd);
        if (request.empty()) {
            close(client_fd);
            return;
        }
        
        std::istringstream stream(request);
        std::string method, path, version;
        stream >> method >> path >> version;
        
        // Handle CORS preflight
        if (method == "OPTIONS") {
            std::string response = "";
            send_response(client_fd, response, "text/plain");
            close(client_fd);
            return;
        }
        
        if (method == "POST") {
            if (path == "/bestmove") {
                std::string body = get_body(request);
                handle_bestmove(client_fd, body);
            } else if (path == "/move") {
                std::string body = get_body(request);
                handle_move(client_fd, body);
            } else {
                send_error(client_fd, 404, "Not found");
            }
        } else if (method == "GET") {
            if (path == "/health") {
                send_response(client_fd, "OK", "text/plain");
            } else {
                send_error(client_fd, 404, "Not found");
            }
        } else {
            send_error(client_fd, 405, "Method not allowed");
        }
        
        close(client_fd);
    }
    
public:
    SimpleHTTPServer(int p) : port(p), server_fd(-1) {}
    
    bool start() {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }
        
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "Failed to set socket options" << std::endl;
            return false;
        }
        
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "Failed to bind socket" << std::endl;
            return false;
        }
        
        if (listen(server_fd, 10) < 0) {
            std::cerr << "Failed to listen" << std::endl;
            return false;
        }
        
        std::cout << "Server started on port " << port << std::endl;
        return true;
    }
    
    void run() {
        while (true) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            
            if (client_fd < 0) {
                std::cerr << "Failed to accept connection" << std::endl;
                continue;
            }
            
            handle_request(client_fd);
        }
    }
    
    ~SimpleHTTPServer() {
        if (server_fd != -1) {
            close(server_fd);
        }
    }
};

int main() {
    SimpleHTTPServer server(8080);
    
    if (!server.start()) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    server.run();
    
    return 0;
}
