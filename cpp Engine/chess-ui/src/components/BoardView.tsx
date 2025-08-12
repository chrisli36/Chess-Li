import { useState, useEffect } from 'react';
import { Chessboard } from 'react-chessboard';
import { Chess } from 'chess.js';
import { api } from '../api';

interface BoardViewProps {
  fen: string;
  onFenChange: (fen: string) => void;
  onMoveListUpdate: (lastMove: string) => void;
  onGameStateChange: (status: 'ongoing' | 'mate' | 'draw') => void;
  userPlaysAs: 'white' | 'black';
  engineDepth: number;
}

export default function BoardView({
  fen,
  onFenChange,
  onMoveListUpdate,
  onGameStateChange,
  userPlaysAs,
  engineDepth
}: BoardViewProps) {
  const [game, setGame] = useState(new Chess(fen));
  const [isPromoting, setIsPromoting] = useState(false);
  const [promotionMove, setPromotionMove] = useState<any>(null);

  useEffect(() => {
    try {
      const newGame = new Chess(fen);
      setGame(newGame);
      
      // Update game state
      if (newGame.isGameOver()) {
        if (newGame.isCheckmate()) {
          onGameStateChange('mate');
        } else if (newGame.isDraw()) {
          onGameStateChange('draw');
        }
      } else {
        onGameStateChange('ongoing');
      }
    } catch (error) {
      console.error('Invalid FEN:', error);
    }
  }, [fen, onGameStateChange]);

  const makeMove = async (move: any) => {
    try {
      // Convert our move object to algebraic notation
      // Only add promotion if it's actually a pawn promotion
      let moveString = `${move.from}${move.to}`;
      if (move.promotion && move.promotion !== 'q') {
        moveString += move.promotion;
      }
      
      console.log('Sending move:', moveString, 'for move object:', move);
      const response = await api.makeMove(fen, moveString);
      console.log('API response:', response);
      
      if (response.legal) {
        onFenChange(response.fen!);
        onMoveListUpdate(response.lastMove);
        
        // Make engine move after a short delay
        setTimeout(() => {
          makeEngineMove(response.fen!);
        }, 500);
      } else {
        console.log('Move was illegal:', response);
      }
      
      return response.legal;
    } catch (error) {
      console.error('Move failed:', error);
      return false;
    }
  };

  const makeEngineMove = async (currentFen: string) => {
    try {
      const response = await api.bestMove(currentFen, engineDepth);
      const engineMove = response.bestMove.long;
      
      // Apply engine move
      const engineResponse = await api.makeMove(currentFen, engineMove);
      if (engineResponse.legal) {
        onFenChange(engineResponse.fen!);
        onMoveListUpdate(engineResponse.lastMove);
      }
    } catch (error) {
      console.error('Engine move failed:', error);
    }
  };

  const onPieceDrop = (sourceSquare: string, targetSquare: string, _piece: any) => {
    const move = {
      from: sourceSquare,
      to: targetSquare,
      promotion: null // No promotion by default
    };

    // Check if this is a pawn promotion
    const boardPiece = game.get(sourceSquare as any);
    if (boardPiece?.type === 'p' && (targetSquare[1] === '8' || targetSquare[1] === '1')) {
      setPromotionMove(move);
      setIsPromoting(true);
      return false; // Don't make the move yet
    }

    // Try to make the move - but we need to return boolean immediately
    // So we'll start the async process and return true
    makeMove(move);
    return true;
  };

  const handlePromotion = (promotionPiece: string) => {
    if (promotionMove) {
      const move = {
        ...promotionMove,
        promotion: promotionPiece
      };
      
      makeMove(move);
      setIsPromoting(false);
      setPromotionMove(null);
    }
  };

  const boardOrientation = userPlaysAs === 'white' ? 'white' : 'black';

  return (
    <div className="relative">
      <Chessboard
        position={fen}
        onPieceDrop={onPieceDrop}
        boardOrientation={boardOrientation}
        boardWidth={400}
        customBoardStyle={{
          borderRadius: '8px',
          boxShadow: '0 4px 6px -1px rgba(0, 0, 0, 0.1)'
        }}
      />
      
      {/* Promotion Modal */}
      {isPromoting && (
        <div className="absolute inset-0 bg-black bg-opacity-50 flex items-center justify-center rounded-lg">
          <div className="bg-gray-800 p-4 rounded-lg">
            <h3 className="text-white text-lg font-semibold mb-3">Choose Promotion</h3>
            <div className="grid grid-cols-2 gap-2">
              {['q', 'r', 'b', 'n'].map((piece) => (
                <button
                  key={piece}
                  onClick={() => handlePromotion(piece)}
                  className="bg-blue-600 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded"
                >
                  {piece.toUpperCase()}
                </button>
              ))}
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
