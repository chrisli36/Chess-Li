import { useState, useEffect } from 'react';
import BoardView from './components/BoardView';
import EvalBar from './components/EvalBar';
import { api } from './api';

function App() {
  const [fen, setFen] = useState('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1');
  const [moveList, setMoveList] = useState<string[]>([]);
  const [gameStatus, setGameStatus] = useState<'ongoing' | 'mate' | 'draw'>('ongoing');
  const [isUserTurn, setIsUserTurn] = useState(true);
  const [userPlaysAs, setUserPlaysAs] = useState<'white' | 'black'>('white');
  const [engineDepth, setEngineDepth] = useState(4);
  const [currentScore, setCurrentScore] = useState<{ cp: number; mate: number | null }>({ cp: 0, mate: null });


  // Update turn based on FEN
  useEffect(() => {
    const turn = fen.includes(' w ') ? 'white' : 'black';
    setIsUserTurn(turn === userPlaysAs);
  }, [fen, userPlaysAs]);

  // Get initial evaluation
  useEffect(() => {
    getEvaluation();
  }, []);

  const getEvaluation = async () => {
    try {
      const response = await api.bestMove(fen, engineDepth);
      setCurrentScore(response.score);
    } catch (error) {
      console.error('Failed to get evaluation:', error);
    }
  };

  const handleFenChange = (newFen: string) => {
    setFen(newFen);
  };

  const handleMoveListUpdate = (lastMove: string) => {
    if (lastMove) {
      setMoveList(prev => [...prev, lastMove]);
    }
  };

  const handleGameStateChange = (status: 'ongoing' | 'mate' | 'draw') => {
    setGameStatus(status);
  };

  const newGame = () => {
    setFen('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1');
    setMoveList([]);
    setGameStatus('ongoing');
    setIsUserTurn(true);
    getEvaluation();
  };

  const flipBoard = () => {
    setUserPlaysAs(prev => prev === 'white' ? 'black' : 'white');
  };

  return (
    <div className="min-h-screen bg-gray-900 text-white p-4">
      <div className="max-w-6xl mx-auto">
        <h1 className="text-3xl font-bold text-center mb-6">Chess Engine</h1>
        
        <div className="grid grid-cols-1 lg:grid-cols-4 gap-6">
          {/* Left sidebar - Controls */}
          <div className="space-y-4">
            <div className="bg-gray-800 p-4 rounded-lg">
              <h2 className="text-xl font-semibold mb-3">Controls</h2>
              <button
                onClick={newGame}
                className="w-full bg-blue-600 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded mb-3"
              >
                New Game
              </button>
              <button
                onClick={flipBoard}
                className="w-full bg-green-600 hover:bg-green-700 text-white font-bold py-2 px-4 rounded mb-3"
              >
                Play as {userPlaysAs === 'white' ? 'Black' : 'White'}
              </button>
              <div className="mb-3">
                <label className="block text-sm font-medium mb-1">Engine Depth:</label>
                <select
                  value={engineDepth}
                  onChange={(e) => setEngineDepth(Number(e.target.value))}
                  className="w-full bg-gray-700 text-white border border-gray-600 rounded px-3 py-2"
                >
                  {[1, 2, 3, 4, 5, 6].map(depth => (
                    <option key={depth} value={depth}>{depth}</option>
                  ))}
                </select>
              </div>
            </div>

            {/* Game Status */}
            <div className="bg-gray-800 p-4 rounded-lg">
              <h3 className="text-lg font-semibold mb-2">Game Status</h3>
              <p className="text-sm">
                Status: <span className="capitalize">{gameStatus}</span>
              </p>
              <p className="text-sm">
                Turn: <span className="capitalize">{isUserTurn ? 'Your Turn' : 'Engine Thinking...'}</span>
              </p>
            </div>

            {/* Move List */}
            <div className="bg-gray-800 p-4 rounded-lg">
              <h3 className="text-lg font-semibold mb-2">Moves</h3>
              <div className="max-h-40 overflow-y-auto">
                {moveList.length === 0 ? (
                  <p className="text-gray-400 text-sm">No moves yet</p>
                ) : (
                  <div className="grid grid-cols-2 gap-1 text-sm">
                    {moveList.map((move, index) => (
                      <div key={index} className="text-gray-300">
                        {Math.floor(index / 2) + 1}.{index % 2 === 0 ? '' : '..'} {move}
                      </div>
                    ))}
                  </div>
                )}
              </div>
            </div>
          </div>

          {/* Center - Chess Board */}
          <div className="lg:col-span-2">
            <BoardView
              fen={fen}
              onFenChange={handleFenChange}
              onMoveListUpdate={handleMoveListUpdate}
              onGameStateChange={handleGameStateChange}
              userPlaysAs={userPlaysAs}
              engineDepth={engineDepth}
            />
          </div>

          {/* Right sidebar - Evaluation */}
          <div className="space-y-4">
            <EvalBar
              score={currentScore}
              depth={engineDepth}
              pv=""
            />
          </div>
        </div>
      </div>
    </div>
  );
}

export default App;
