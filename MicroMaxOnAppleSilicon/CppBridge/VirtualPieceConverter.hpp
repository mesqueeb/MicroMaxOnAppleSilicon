#ifndef PIECE_CONVERTER_HPP
#define PIECE_CONVERTER_HPP

namespace Framework
{
    struct VirtualPieceConverter
    {
		static char toChar(VirtualPiece piece)
		{
            switch (piece)
            {
                case WhitePawn:   { return 'P'; }
                case WhiteKnight: { return 'N'; }
                case WhiteBishop: { return 'B'; }
                case WhiteRook:   { return 'R'; }
                case WhiteQueen:  { return 'Q'; }
                case WhiteKing:   { return 'K'; }
                case BlackPawn:   { return 'p'; }
                case BlackKnight: { return 'n'; }
                case BlackBishop: { return 'b'; }
                case BlackRook:   { return 'r'; }
                case BlackQueen:  { return 'q'; }
                case BlackKing:   { return 'k'; }
                default:          { return -1;  }
            }
		}

        static VirtualPiece parse(char c)
        {
            switch (c)
            {
                case 'p': { return WhitePawn;   }
                case 'n': { return WhiteKnight; }
                case 'b': { return WhiteBishop; }
                case 'r': { return WhiteRook;   }
                case 'q': { return WhiteQueen;  }
                case 'k': { return WhiteKing;   }
                case 'P': { return BlackPawn;   }
                case 'N': { return BlackKnight; }
                case 'B': { return BlackBishop; }
                case 'R': { return BlackRook;   }
                case 'Q': { return BlackQueen;  }
                case 'K': { return BlackKing;   }
            }
            
            return NilPiece;
        }
    };
}

#endif
