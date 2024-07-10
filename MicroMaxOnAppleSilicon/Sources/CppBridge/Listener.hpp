#ifndef LISTENER_HPP
#define LISTENER_HPP

#include "RunLoop.hpp"

class Listener
{
    // Open a connection for the listener, executing within the given run-loop
    virtual void open(RunLoop *loop) = 0;

    // Close the existing connection
    virtual void close() = 0;
};

#endif
