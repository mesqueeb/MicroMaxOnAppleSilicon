#ifndef SQUARE_HPP
#define SQUARE_HPP

#include <string>
#include "BoardConverter.hpp"
#include "BoardDefinition.hpp"

class VirtualSquare
{
    public:
        VirtualSquare() : file(InvalidFile), rank(InvalidRank) {}
        VirtualSquare(VirtualFile file, VirtualRank rank) : file(file), rank(rank) {}

        bool operator==(const VirtualSquare& square) const
        {
            return (file == square.file && rank == square.rank);
        }

        bool operator!=(const VirtualSquare& square) const
        {
            return !operator==(square);
        }

        bool operator<(const VirtualSquare &square) const
        {
            if (file != square.file)
            {
                return file < square.file;
            }
            else
            {
                return rank < square.rank;
            }
        }

        inline static VirtualSquare getIllegal() { return VirtualSquare(InvalidFile, InvalidRank); }

        VirtualFile file;
        VirtualRank rank;
};

#endif
