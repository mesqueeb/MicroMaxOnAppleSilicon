#include "UnixAdapter.hpp"

using namespace Framework;

// An abstract factory method for creating an instance of Unix adapter, encapsualting its internal details.
AbstractAdapter *createUnixAdapter()
{
    return new UnixAdapter();
}
