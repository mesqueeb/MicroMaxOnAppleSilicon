#ifndef RESOURCES_HPP
#define RESOURCES_HPP

#include <map>
#include "../AbstractProtocol.hpp"

namespace Framework
{
    class Protocol;
	class CoreReceiver;
	class DecoratorCore;
	class VirtualAdapter;

	typedef std::map<std::string, std::string> Config;

	class Resources
	{
		public:
			~Resources() { close(); }

			void open();
			void close();

			// Detach an instance of a CoreResponder in the stack
			bool detach(DecoratorCore *core);

            // Clear anything that relates to annoation
            void clearAnnotation();

			// Return the current thinking time
			void getParameters(unsigned int &level, unsigned int &think) const;

            // Create a resource
            static Resources *create();

			// Reference to the Adapter layer
			Adapter *adapter;

			// Reference to the Protocol layer
			AbstractProtocol *protocol;

		protected:
			Resources(Adapter *adapter, AbstractProtocol *protocol) :
				 adapter(adapter), protocol(protocol) {}
	};
}

#endif
