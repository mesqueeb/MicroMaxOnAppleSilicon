#ifndef FEN_PARSER_HPP
#define FEN_PARSER_HPP

#include <map>
#include <string>
#include "Types.hpp"
#include "VirtualSquare.hpp"
#include "VirtualPieceConverter.hpp"

namespace Framework
{
    struct FENParser
    {
		static void parse(const std::string &fen, std::map<std::pair<unsigned, unsigned>, char> &placement, VirtualSquare &enpassant)
		{
			std::map<VirtualSquare, VirtualPiece> temp;
			FENParser::parse(fen, temp, enpassant);

			for (std::map<VirtualSquare, VirtualPiece>::const_iterator iter = temp.begin(); iter != temp.end(); iter++)
			{
				placement[std::make_pair<unsigned, unsigned>(static_cast<unsigned>(iter->first.file), static_cast<unsigned>(iter->first.rank))]
						= VirtualPieceConverter::toChar(iter->second);
			}
		}

		static void parse(const std::string &fen, std::map<VirtualSquare, VirtualPiece> &placement, VirtualSquare &enpassant)
		{
			unsigned i = 0;
			unsigned j = 0;

			while (fen[i] != ' ')
			{
				const VirtualSquare square(static_cast<VirtualFile>(j  % 8), static_cast<VirtualRank>(7 - (j / 8)));

				switch (fen[i])
				{
					case 'p' : { placement[square] = BlackPawn;   break; }
					case 'r' : { placement[square] = BlackRook;   break; }
					case 'n' : { placement[square] = BlackKnight; break; }
					case 'b' : { placement[square] = BlackBishop; break; }
					case 'q' : { placement[square] = BlackQueen;  break; }
					case 'k' : { placement[square] = BlackKing;   break; }
					case 'P' : { placement[square] = WhitePawn;   break; }
					case 'R' : { placement[square] = WhiteRook;   break; }
					case 'N' : { placement[square] = WhiteKnight; break; }
					case 'B' : { placement[square] = WhiteBishop; break; }
					case 'Q' : { placement[square] = WhiteQueen;  break; }
					case 'K' : { placement[square] = WhiteKing;   break; }
					case '/' : { j--; break; }
					case '1' : { break; }
					case '2' : { j++;  break; }
					case '3' : { j+=2; break; }
					case '4' : { j+=3; break; }
					case '5' : { j+=4; break; }
					case '6' : { j+=5; break; }
					case '7' : { j+=6; break; }
					case '8' : { j+=7; break; }
				}

				i++;
				j++;
			}
            
            while (fen[++i] != ' ') {} // Skip colors
            while (fen[++i] != ' ') {} // Skip castling

            if (fen[++i] != '-')
            {
                enpassant = VirtualSquare(static_cast<VirtualFile>(fen[i] - 'a'), static_cast<VirtualRank>(fen[i + 1] - '1'));
            }
            else
            {
                enpassant = VirtualSquare::getIllegal();
            }
		}
    };
}

#endif
