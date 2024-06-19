#include <sstream>
#include <iostream>
#include "AbstractAdapter.hpp"

#ifndef _MSC_VER
#include <pthread.h>
#else
#include <mutex>
#include <condition_variable>
#endif

#ifdef _MSC_VER
#include <windows.h>
#else
#include <unistd.h>
#endif

extern "C" void __printToConsole__(const char * str);

using namespace Framework;

typedef enum AdapterState
{
    // State in which processing is disabled
    INACTIVE,

    // State in which processing is enabled
    ACTIVE,

    // State in which processing is terminated
    TERMINATED
} AdapterState;

/* -------------------- Pimpl -------------------- */

struct AbstractAdapterImpl
{
    AbstractAdapterImpl() : state(INACTIVE)
    {
#ifndef _MSC_VER
        pthread_mutex_init(&processLock, NULL);
#endif
	}

    ~AbstractAdapterImpl()
    {
#ifndef _MSC_VER
		pthread_mutex_destroy(&processLock);
#endif
    }

    Linker *link;
    AdapterReceiver *receiver;

    // Internal state of the adapter
    AdapterState state;

#ifndef _MSC_VER
    // Mutex for processing messages
    pthread_mutex_t processLock;

    // A condition for synchronization between main and listener thread
    pthread_cond_t terminateCond;
#else
    // Mutex for processing messages
    std::mutex processLock;

    // A condition for synchronization between main and listener thread
	std::condition_variable terminateCond;
#endif
};

AbstractAdapter::AbstractAdapter() : _pimpl(new AbstractAdapterImpl())
{
    _pimpl->link = NULL;
    _pimpl->receiver = NULL;

#ifndef _MSC_VER
    if (pthread_cond_init(&_pimpl->terminateCond, NULL) != 0)
    {
        throw std::runtime_error("Failed to initialise terminate condition");
    }
#endif
}

AbstractAdapter::~AbstractAdapter()
{
    delete _pimpl;
    _pimpl = NULL;
}

// Attach to the current receiver
void AbstractAdapter::attach(AdapterReceiver *receiver)
{
    _pimpl->receiver = receiver;
}

// Detach the current receiver
void AbstractAdapter::detach()
{
    _pimpl->receiver = NULL;
}

void AbstractAdapter::open()
{
	createLinking();
    
#ifdef _MSC_VER
#ifndef _WINRT_DLL
    Sleep(200);
#endif
#else
    sleep(1);
#endif

    // Create a listener, waiting for message from VirtualLink
    createListener();
}

// Inhertied from VirtualAdapter
void AbstractAdapter::close()
{
    if (_pimpl->link)
    {
        _pimpl->state = TERMINATED;

#ifndef _MSC_VER
        pthread_mutex_lock(&_pimpl->processLock);
#else
		std::unique_lock<std::mutex> lock(_pimpl->processLock);
#endif

        // Close the linking
        _pimpl->link->close();

#ifndef _MSC_VER
        // Wait until the listener thread is terminated
        pthread_cond_wait(&_pimpl->terminateCond, &_pimpl->processLock);

        pthread_mutex_unlock(&_pimpl->processLock);
#else
        // Wait until the listener thread is terminated
		_pimpl->terminateCond.wait(lock);
#endif
    }
}

void AbstractAdapter::createLinking()
{
    // Create an instance of VirtualLinker, createLinker() is a template method
    _pimpl->link = createLinker();

    _pimpl->link->create();
    _pimpl->state = ACTIVE;
}

// Inhertied from VirtualAdapter
std::string AbstractAdapter::read()
{
    return _pimpl->link->read();
}

// Inhertied from VirtualAdapter
void AbstractAdapter::write(const std::string& line)
{
    _pimpl->link->write(line);
}

// Inherited from VirtualRunLoop
void AbstractAdapter::run()
{
    std::string lines;

    while (true)
    {
        try
        {
            if (_pimpl->state == TERMINATED)
            {
                throw "This exception is thrown so that the code can reach the catch block";
            }

            lines = this->read();
        }
        catch (...)
        {
            /*
             * Signal the terminate condition just before we're about to terminate
             */

            pthread_mutex_lock(&_pimpl->processLock);
            pthread_cond_signal(&_pimpl->terminateCond);
            pthread_mutex_unlock(&_pimpl->processLock);
            return;
        }

        /* -------------------- Message Processing -------------------- */

        std::string line;
        std::stringstream ss(lines);

        /*
         * Since more than a line could be read, the lines have to be splitted.
         */
        while (std::getline(ss, line, '\n'))
        {
#ifdef LOGGING_DEFINED
            std::cerr << "<-- " << line << std::endl;
#endif
            if (line.empty())
            {
                continue;
            }

            switch (_pimpl->state)
            {
                case TERMINATED:
                {
                    return;
                }
                    
                case INACTIVE:
                {
                    continue;
                }

                // Process messages to receiver
                case ACTIVE:
                {
                    if (_pimpl->receiver)
                    {
                        _pimpl->receiver->process(line);
                    }
                }
            }
        }
    }
}
