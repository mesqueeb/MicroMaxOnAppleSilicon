#ifndef CHESS_POSITION_HPP
#define CHESS_POSITION_HPP

#include <string>
#include "Types.hpp"

#define INITIAL_CHESS_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"

class ChessPosition
{
    public:
        /*
         *ruct a complete FEN from an incomplete FEN state and other details
         */
        static std::string construct(const std::string &, Player, bool wkCastle, bool bkCastle, bool wqCastle, bool bqCastle);
    
        /*
         * Return the player to move for a given FEN state
         */
        static Player getTurn(const std::string &);
};

#endif
