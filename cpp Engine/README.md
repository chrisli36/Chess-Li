# Chess Engine HTTP API

A tiny HTTP+WebSocket API built around a C++ chess engine using Crow and nlohmann/json.

## Features

- **POST /bestmove** - Get the best move for a given position
- **POST /move** - Make a move and get the resulting position
- **WebSocket /ws/search** - Stream search information
- **CORS enabled** for development

## Requirements

- C++20 compatible compiler
- CMake 3.16+
- Internet connection (for fetching dependencies)

## Dependencies

- **Crow** - HTTP/WebSocket framework (automatically fetched)
- **nlohmann/json** - JSON library (automatically fetched)
- **SFML** - For the original chess GUI (optional)

## Build

```bash
# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make -j$(nproc)

# Or on macOS
cmake ..
make -j$(sysctl -n hw.ncpu)
```

## Run

```bash
# Start the HTTP API server
./server

# The server will start on port 8080
```

## API Endpoints

### POST /bestmove

Get the best move for a given position.

**Request:**
```json
{
  "fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
  "depth": 4
}
```

**Response:**
```json
{
  "bestMove": {
    "long": "e2e4",
    "from": "e2",
    "to": "e4",
    "promo": null
  },
  "score": {
    "cp": 25,
    "mate": null
  }
}
```

### POST /move

Make a move and get the resulting position.

**Request:**
```json
{
  "fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
  "move": "e2e4"
}
```

**Response:**
```json
{
  "legal": true,
  "fen": "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
  "status": "ongoing",
  "lastMove": "e2e4"
}
```

### WebSocket /ws/search

Stream search information for a position.

**Message:**
```json
{
  "fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
  "depth": 4
}
```

**Response:**
```json
{
  "type": "bestmove",
  "bestMove": "e2e4"
}
```

## Architecture

- **Instantiate per-request**: Each request creates a new `Board` and `Engine` instance
- **Move validation**: Checks if moves are legal before applying them
- **FEN serialization**: Converts board state to/from FEN notation
- **Error handling**: Returns appropriate HTTP status codes and error messages

## Development

The API is built around the existing chess engine classes:
- `Board` - Handles board state and move generation
- `Engine` - Implements search algorithms
- `Move` - Represents chess moves
- `ChessAdapter` - Converts between internal and external formats

## Testing

```bash
# Test the health endpoint
curl http://localhost:8080/health

# Test bestmove endpoint
curl -X POST http://localhost:8080/bestmove \
  -H "Content-Type: application/json" \
  -d '{"fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "depth": 4}'
```
