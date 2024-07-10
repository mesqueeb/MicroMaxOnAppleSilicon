#ifndef BOARD_HPP
#define BOARD_HPP

#include "VirtualResponder.hpp"
#include "CoreResponder.hpp"
#include "./include/ProtocolReceiver.hpp"

namespace Framework
{
	class VirtualBoard : public CoreResponder
	{
		friend class ChessCore;
		friend class Resources;
		friend class DecoratorCore;

		public:
			VirtualBoard(Protocol * protocol) : _protocol(protocol) {}
			virtual ~VirtualBoard();

            // Inherited from Core
            virtual std::shared_ptr<Core> copy() const;

            inline Protocol *getProtocol() const { return _protocol; }
            inline void setProtocol(Protocol * protocol) { _protocol = protocol; }

	#ifndef UNIT_TESTING
		private:
	#endif

			Protocol *_protocol;
	};
}

#endif
