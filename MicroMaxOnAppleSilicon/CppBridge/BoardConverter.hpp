#ifndef BOARD_CONVERTER_HPP
#define BOARD_CONVERTER_HPP

#include "Types.hpp"
#include "BoardCoord.hpp"

#define CONVERT_FILE_TO_CHAR(f)  ('a' + f)
#define CONVERT_CHAR_TO_COORD(c) (c < 'a') ? (c - '1') : (c - 'a')

class BoardConverter
{
    public:
        static inline BoardCoord convert(unsigned char c)
        {
            return (c < 'a') ? (c - '1') : (c - 'a');
        }

        template<typename T> static inline T convert(unsigned char c)
        {
            return static_cast<T>((c < 'a') ? (c - '1') : (c - 'a'));
        }

		static char pieceToChar(const VirtualPiece &piece)
		{
			switch (piece)
			{
				case NilPiece: { return 0; }

				case AnyQueen:
				case WhiteQueen:
				case BlackQueen: { return 'q'; }
            
				case AnyRook:
				case WhiteRook:
				case BlackRook: { return 'r'; }
            
				case AnyBishop:
				case WhiteBishop:
				case BlackBishop: { return 'b'; }
            
				case AnyKnight:
				case WhiteKnight:
				case BlackKnight: { return 'n'; }
            
				default: { return 0; }
			}
		}

		static VirtualPiece charToPiece(char c)
		{
			switch (c)
			{            
				case 'q': { return AnyQueen;  }
				case 'r': { return AnyRook;   }
				case 'b': { return AnyBishop; }
				case 'n': { return AnyKnight; }
			}
    
			return NilPiece;
		}
};

#endif
