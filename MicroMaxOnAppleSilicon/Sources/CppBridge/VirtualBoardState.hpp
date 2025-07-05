#ifndef BOARD_STATE_HPP
#define BOARD_STATE_HPP

#include "Types.hpp"
#include "VirtualSquare.hpp"
#include <string>

namespace Framework
{
    class VirtualBoardState
    {
        public:
            VirtualBoardState(const std::string &state, const Player &turn) : state(state), turn(turn) {}

            VirtualBoardState(const VirtualBoardState &boardState)
            {
                this->turn  = boardState.turn;
                this->state = boardState.state;
            }

            bool operator==(const VirtualBoardState &o)
            {
                return state == o.state && turn == o.turn;
            }

            // Current state of the board
            std::string state;

            // The player eligible for the current turn
            Player turn;
    };
}

#endif
