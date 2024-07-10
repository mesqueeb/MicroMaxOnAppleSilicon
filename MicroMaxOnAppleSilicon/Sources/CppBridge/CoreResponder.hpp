#ifndef CORE_RESPONDER_HPP
#define CORE_RESPONDER_HPP

#include "Core.hpp"
#include "Protocol.hpp"
#include "VirtualResponder.hpp"

namespace Framework
{
	class CoreResponder : public Core, public VirtualResponder
	{
		public:
			virtual Protocol *getProtocol() const = 0;
	};
}

#endif
