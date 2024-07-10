#ifndef STATE_HPP
#define STATE_HPP

#include "VirtualBoardState.hpp"

namespace Framework
{
	class VirtualState
	{
		public:
			VirtualState(const VirtualBoardState& initial) : initial(initial), current(initial) {}

			// Initial state of the board
			VirtualBoardState initial;

			// Current state of the board
			VirtualBoardState current;
	};
}

#endif
