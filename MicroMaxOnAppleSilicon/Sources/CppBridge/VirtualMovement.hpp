#ifndef MOVEMENT_HPP
#define MOVEMENT_HPP

#include "Types.hpp"
#include "VirtualSquare.hpp"

class VirtualMovement
{
    public:
        VirtualMovement() {}
		VirtualMovement(const VirtualSquare &src, const VirtualSquare &dst, VirtualPiece promote = AnyQueen) : src(src), dst(dst), promote(promote) {}
        VirtualMovement(VirtualFile srcFile, VirtualRank srcRank, VirtualFile dstFile, VirtualRank dstRank, VirtualPiece promote = AnyQueen) :
                            src(VirtualSquare(srcFile, srcRank)), dst(VirtualSquare(dstFile, dstRank)), promote(promote) {}

        bool operator!=(const VirtualMovement &move) const
        {
            return !operator==(move);
        }
    
        bool operator==(const VirtualMovement &move) const
        {
            return (src == move.src) && (dst == move.dst) && (promote == move.promote);
        }
    
        bool operator<(const VirtualMovement &move) const
        {
            if (src != move.src)
            {
                return src < move.src;
            }
            else if (dst != move.dst)
            {
                return dst < move.dst;
            }
            else
            {
                return promote < move.promote;
            }
        }

        VirtualSquare src;
        VirtualSquare dst;
		VirtualPiece promote;
};

#endif
