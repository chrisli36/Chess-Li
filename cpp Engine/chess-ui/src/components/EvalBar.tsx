interface EvalBarProps {
  score: { cp: number; mate: number | null };
  depth: number;
  pv: string;
}

export default function EvalBar({ score, depth, pv }: EvalBarProps) {
  const getEvalDisplay = () => {
    if (score.mate !== null) {
      return score.mate > 0 ? `Mate in ${score.mate}` : `Mate in ${Math.abs(score.mate)}`;
    }
    return `${score.cp > 0 ? '+' : ''}${score.cp}`;
  };

  const getEvalBarPosition = () => {
    if (score.mate !== null) {
      return score.mate > 0 ? '100%' : '0%';
    }
    
    // Convert centipawns to 0-100% scale
    // Assuming ±1000 centipawns = ±100%
    const normalized = Math.max(-100, Math.min(100, score.cp / 10));
    return `${50 + normalized / 2}%`;
  };

  return (
    <div className="bg-gray-800 p-4 rounded-lg">
      <h3 className="text-lg font-semibold mb-3">Evaluation</h3>
      
      {/* Score Display */}
      <div className="text-center mb-4">
        <div className="text-2xl font-bold text-blue-400">
          {getEvalDisplay()}
        </div>
      </div>
      
      {/* Evaluation Bar */}
      <div className="relative h-32 bg-gray-700 rounded-lg mb-4">
        <div className="absolute inset-0 flex items-center justify-center">
          <div className="text-gray-400 text-sm">0.0</div>
        </div>
        
        {/* Evaluation indicator */}
        <div
          className="absolute w-2 h-2 bg-blue-400 rounded-full transform -translate-x-1 -translate-y-1"
          style={{ top: getEvalBarPosition() }}
        />
        
        {/* Scale markers */}
        <div className="absolute top-0 left-1/2 transform -translate-x-1/2 text-xs text-gray-500">
          +∞
        </div>
        <div className="absolute bottom-0 left-1/2 transform -translate-x-1/2 text-xs text-gray-500">
          -∞
        </div>
      </div>
      
      {/* Depth Info */}
      <div className="text-sm text-gray-300">
        <div>Depth: {depth}</div>
        {pv && (
          <div className="mt-2">
            <div className="text-gray-400">Principal Variation:</div>
            <div className="font-mono text-xs break-all">{pv}</div>
          </div>
        )}
      </div>
    </div>
  );
}
