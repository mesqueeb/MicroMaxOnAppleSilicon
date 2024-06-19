#ifndef CHESS_NOTATION_HPP
#define CHESS_NOTATION_HPP

#include <vector>
#include <algorithm>
#include "./include/ChessCore.hpp"
#include "VirtualMovement.hpp"
#include "VirtualBoardState.hpp"
//#include <boost/algorithm/string/replace.hpp>

namespace Framework
{
	class ChessNotation
	{
		public:
			static bool parseCastle(Core *core, const std::string &str, VirtualMovement &move);
			static bool parsePiece(Core *core, const VirtualPiece &piece, const std::string &str, VirtualMovement &move);
			static bool parsePawn(Core *core, const std::string &str, VirtualMovement &move);
			static bool parseMove(Core *core, const std::string &str, VirtualMovement &move);
			static bool parseSimpleNotation(Core *core, const std::string &str, VirtualMovement &move);

            // Convert a token such as 1.Ng4 its representation in figurine, convert all letters if all is true
            static bool toUnicode(const std::string &token, std::string &converted, bool all = false);

            static std::string getPrefix(const Core &core);

            // Convert a move to the SAN notation. The move is assumed to be a legal.
            static std::string toSAN(const Core &core, const VirtualMovement &move);

			/*
			 * Generate a complex chess notation (eg: Rad1). In Chess, two or more pieces of the same type can attack the same square.
			 * For example, two rooks on the back rank both attacking the d1 square.
			 *
			 *  Arguments
			 *
			 *     move  = The squares the piece moved from and to (eg: e2 to e4)
			 *     piece = The piece the moved is about (eg: WhitePawn)
			 *     type  = The type of the move (eg: NORMAL)
			 *     attackers = The locations of all other attackers
			 *
			 *  Returns
			 *     A string of the chess notation (eg: "e4")
			 */
			static std::string toNotation(const VirtualMovement &move,
                                          const VirtualPiece& srcPiece,
                                          const VirtualMoveType &type,
                                          const std::vector<VirtualSquare>& attackers = std::vector<VirtualSquare>());

			/*
			 * Parse a chess notation
			 *
			 *  Arguments
			 *
			 *     notation = The notation of the move
			 *     move     = The squares the piece moved from and to (eg: e2 to e4)
			 *     board    = Current state of the board
			 *
			 *  Returns
			 *     True if succeed otherwise false
			 */
			static bool parse(const std::string &notation, VirtualMovement &move, const VirtualBoardState &board);

		private:
			ChessCore core;
	};
}

#endif
