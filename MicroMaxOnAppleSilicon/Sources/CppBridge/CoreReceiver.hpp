#ifndef CORE_RECEIVER_HPP
#define CORE_RECEIVER_HPP

#include <vector>
#include "Types.hpp"
#include "VirtualMove.hpp"

namespace Framework
{
	class CoreReceiver
	{
		public:
            virtual bool notifyValid(const VirtualMove &) { return true; }
            virtual void receiveMove(const VirtualMove &) {}
	};
}

#endif
