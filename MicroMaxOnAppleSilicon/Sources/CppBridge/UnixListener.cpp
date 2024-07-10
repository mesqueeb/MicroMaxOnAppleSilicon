#include "UnixListener.hpp"

// The thread function for listener execution
static void *listenerThread(void *loop)
{
    ((RunLoop *) loop)->run();
    return NULL;
}

void UnixListener::close()
{
    if (_loop)
    {
        // Terminate the run-loop execution
        _loop->exit();
    }

    if (_listener)
    {
        // Cancel execution of the listener thread
        pthread_cancel(*_listener);
        delete _listener;
    }

    _loop     = NULL;
    _listener = NULL;
}

void UnixListener::open(RunLoop *loop)
{
    if (!_listener)
    {
        // Copy the run-loop reference
        _loop = loop;

        pthread_attr_t attr;
        pthread_attr_init(&attr);

        // Create a detached thread, its threadID and other resources can be reused as soon as the thread terminates
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        // Create a listener thread
        pthread_create(_listener = new pthread_t, &attr, &listenerThread, loop);

        // Destroy the unused attribute
        pthread_attr_destroy(&attr);
    }
}
