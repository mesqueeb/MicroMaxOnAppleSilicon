#include "./include/Resources.hpp"
#include "UnixAdapter.hpp"
#include "WBProtocol.hpp"

#define DELETE_PTR(x)  if (x) { x->close(); delete(x); x = NULL; }
#define SAFE_DETACH(x) if (x) { const bool succeed = detach(x); assert(succeed); x = NULL; }

using namespace Framework;

void Resources::open()
{
    if (adapter)  { adapter->open();  }
    if (protocol) { protocol->open(); }
}

void Resources::close()
{
    DELETE_PTR(adapter);
    DELETE_PTR(protocol);
}

// Detach an instance of a Core in the stack
bool Resources::detach(DecoratorCore *core)
{
    /*
    Core *temp = core;
    
    #define FRONT_OF_THE_STACK this->core

    while (true)
    {
        // Terminate as soon as there's no match
        if (!temp || !dynamic_cast<DecoratorCore *>(temp))
        {
            break;
        }
        
        DecoratorCore *dCore = dynamic_cast<DecoratorCore *>(temp);
        
        if (dCore == core)
        {
            // If we're detaching the front of the stack, we also must modify the front
			if (FRONT_OF_THE_STACK == dCore)
			{
				FRONT_OF_THE_STACK = dCore->_core;
			}

            assert(FRONT_OF_THE_STACK != core);

            dCore->rebuild();

            assert(!dCore->_core);

            // Once the connection has been re-established, we can safety deallocate it
            delete dCore;

            // We've found the match, terminate immediately
            return true;
        }
        
        temp = dCore->getChild();
    }    
*/
    return false;
}

Resources *Resources::create()
{
    // Step 1: adapter layer
    AbstractAdapter *adapter = new UnixAdapter();

    // Step 2: protocol layer
    AbstractProtocol *protocol = new WBProtocol(adapter);

    adapter->attach((WBProtocol *) protocol);
    return new Resources(adapter, protocol);
}
