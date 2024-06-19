#ifndef MOVE_HPP
#define MOVE_HPP

#include <string>
#include "Types.hpp"
#include "VirtualSquare.hpp"
#include "VirtualMovement.hpp"

class VirtualMove
{
    public:
        // Initialise an undefined VirtualMove
        VirtualMove() {}

        VirtualMove(const std::string& notation,
                    const VirtualSquare& src,
                    const VirtualSquare& dst,
                    VirtualPiece  srcPiece,
                    VirtualPiece  dstPiece,
                    Player turn,
                    VirtualMoveType type
                   )
        : notation(notation),
          src(src),
          dst(dst),
          srcPiece(srcPiece),
          dstPiece(dstPiece),
          turn(turn),
          type(type) {}

        // Description of the move, eg: e4 for chess
        std::string notation;

        // Square played from src to dst
        VirtualSquare src, dst;

        // Player who has the turn after the move is made
        Player turn;

        // Type of the move
        VirtualMoveType type;

        // Pieces on src and dst
        VirtualPiece srcPiece, dstPiece;

        bool operator!=(const VirtualMovement &move) const { return !operator==(move); }
        bool operator==(const VirtualMovement &move) const { return (src == move.src) && (dst == move.dst); }
    
        bool operator!=(const VirtualMove &move) const { return !operator==(move); }
        bool operator==(const VirtualMove &move) const
        {
            return (src == move.src) && (dst == move.dst) && (turn == move.turn) && (srcPiece == move.srcPiece) && (dstPiece == move.dstPiece);
        }
};

#endif
