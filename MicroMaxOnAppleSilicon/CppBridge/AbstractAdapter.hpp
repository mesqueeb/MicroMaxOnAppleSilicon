#ifndef ABSTRACT_ADAPTER_HPP
#define ABSTRACT_ADAPTER_HPP

#include "Adapter.hpp"
#include "Linker.hpp"
#include "AdapterReceiver.hpp"
#include "RunLoop.hpp"

struct AbstractAdapterImpl;

namespace Framework
{
	class AbstractAdapter : public Adapter, public RunLoop
	{
		public:
			AbstractAdapter();
			virtual ~AbstractAdapter();

			virtual void open() override;

			virtual void close() override;

			virtual std::string read() override;

			virtual void write(const std::string &) override;

			void attach(AdapterReceiver *receiver);

			void detach();

			virtual void run() override;
        
			virtual void exit() override { close(); }

		protected:
			// Abstract factory method for the concrete bridge object
			virtual Linker *createLinker() = 0;

			// Create a listener, usually on a new thread
			virtual void createListener() = 0;

		private:
			void createLinking();

			struct AbstractAdapterImpl *_pimpl;
	};
}

#endif
