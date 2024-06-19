#ifndef UNIX_LISTENER_HPP
#define UNIX_LISTENER_HPP

#include <pthread.h>
#include "Listener.hpp"

class UnixListener : public Listener
{
    public:
        UnixListener() : _listener(NULL), _loop(NULL) {}
        virtual ~UnixListener() { close(); }

        virtual void open(RunLoop *) override;
        virtual void close() override;

    private:
        // Thread reference for the listener
        pthread_t *_listener;

        // Reference to VirtualRunLoop passed in open()
        RunLoop *_loop;
};

#endif
