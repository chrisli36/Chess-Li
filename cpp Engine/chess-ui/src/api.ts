const API_BASE = import.meta.env.VITE_API_BASE || 'http://localhost:8080';

export interface BestMoveResponse {
  bestMove: {
    long: string;
    from: string;
    to: string;
    promo: string | null;
  };
  score: {
    cp: number;
    mate: number | null;
  };
}

export interface MoveResponse {
  legal: boolean;
  fen: string | null;
  status: 'ongoing' | 'mate' | 'draw';
  lastMove: string;
}

export const api = {
  async bestMove(fen: string, depth: number = 4): Promise<BestMoveResponse> {
    const response = await fetch(`${API_BASE}/bestmove`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ fen, depth }),
    });

    if (!response.ok) {
      throw new Error(`API error: ${response.status}`);
    }

    return response.json();
  },

  async makeMove(fen: string, move: string): Promise<MoveResponse> {
    const response = await fetch(`${API_BASE}/move`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ fen, move }),
    });

    if (!response.ok) {
      throw new Error(`API error: ${response.status}`);
    }

    return response.json();
  },
};
