#ifndef RESPONDER_HPP
#define RESPONDER_HPP

#include "Core.hpp"
#include "CoreReceiver.hpp"

namespace Framework
{
	class VirtualResponder
	{
		friend class VirtualCoordinatorTest;

		public:
			inline void attach(CoreReceiver *receiver) { detach(); _receiver = receiver; }

			CoreReceiver *detach()
			{
				CoreReceiver * temp = _receiver;
				_receiver = NULL;
				return temp; 
			}

			inline CoreReceiver *getReceiver() const { return _receiver; }

		protected:
			VirtualResponder() : _receiver(NULL) {}
			CoreReceiver *_receiver;
	};
}

#endif
