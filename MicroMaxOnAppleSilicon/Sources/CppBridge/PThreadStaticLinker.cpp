#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include "engine.h"
#include "PThreadStaticLinker.hpp"

using namespace std;

// Shared between engines
int __engineFD__;

/* -------------------- Internal Class -------------------- */

struct PThreadStaticLinkerImpl
{
    void createEngine();

    // Handle for the engine thread
    pthread_t engine;

    // Private structure for sending to engine
    engine_t toEngine;

    // Private strucutre for receiving from engine
    engine_t fromEngine;

    // Shared buffer for IO buffering
    char buffer[BufferSize];
};

/* -------------------- Forward Declaration -------------------- */

// Thread for running engine
static void* engine_thread(void* data);

/* -------------------- Engine Methods -------------------- */

extern "C"
{
    int main_fairymax(int argc, char *argv[]);
    int main_smallchess(int argc, char *argv[]);
}

int (*main_engine)(int argc, char *argv[]);

/* -------------------- Member Methods -------------------- */

PThreadStaticLinker::PThreadStaticLinker(const string& engine) : _pimpl(new PThreadStaticLinkerImpl())
{
    main_engine = main_fairymax;
    assert(main_engine);
}

// Thread for running engine
static void* engine_thread(void * data)
{
    // Shared the engine with it's file descriptor
    __engineFD__ = (int) (size_t) data;
    
    char *argv[1];

    // Really only need 3 characters (eg: "10\n"), an extra char for precaution.
    argv[0] = (char *) malloc(4 * sizeof(char));

    // Convert int to char *
    sprintf(argv[0], "%d", (int) (size_t) data);

    try
    {
        main_engine(1, argv);
    }
    catch (exception)
    {
        throw std::runtime_error("Failed to initalize the engine");
    }
    
    // There shouldn't be any reason that an engine quits it's main loop
    assert(false);
    
    return NULL;
}

void PThreadStaticLinkerImpl::createEngine()
{
    // Duplicate output pipe to standard output
    dup2(fromEngine.io[0].out_fd, STDOUT_FILENO);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&engine, &attr, &engine_thread, reinterpret_cast<void *>(toEngine.io[0].in_fd));
    pthread_attr_destroy(&attr);
}

string PThreadStaticLinker::read()
{
    // Read from engine directly into _buffer
    engine_get(&_pimpl->fromEngine, _pimpl->buffer, BufferSize);

    return string(_pimpl->buffer);
}

// Establish inter-thread connection to engine
void PThreadStaticLinker::create()
{
    int from_fd[2], to_fd[2];

    // Create pipe for receiving from engine
    assert(pipe(from_fd) != -1);

    // Create pipe for sending to engine
    assert(pipe(to_fd) != -1);

    _pimpl->fromEngine.io[0].in_fd  = from_fd[0];
    _pimpl->fromEngine.io[0].out_fd = from_fd[1];

    _pimpl->toEngine.io[0].in_fd  = to_fd[0];
    _pimpl->toEngine.io[0].out_fd = to_fd[1];

    io_init(&_pimpl->fromEngine.io[0]);
    io_init(&_pimpl->toEngine.io[0]);

    _pimpl->createEngine();
    //_pimpl->createListener();

    /*
     * From this point, the caller can communicate with the engine via standard input and output.
     */
}

void PThreadStaticLinker::close()
{
    if (_pimpl)
    {
        assert(pthread_kill(_pimpl->engine, 0) == 0);
        delete _pimpl;
        _pimpl = NULL;
    }
}

void PThreadStaticLinker::write(const string& command)
{
    engine_send(&_pimpl->toEngine, command.c_str());
}
