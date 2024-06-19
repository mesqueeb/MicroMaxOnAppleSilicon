#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <assert.h>

#include <sys/types.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "io.h"
#include "engine.h"
//#include "Adapter/Linker/util.h"

// constants

static const int StringSize = 4096;

// engine_get()

void engine_get(engine_t * engine, char string[], int size)
{
    //ASSERT(engine_is_ok(engine));
    assert(string!=NULL);
    assert(size>=256);

    while (!io_line_ready(engine->io))
    {
        io_get_update(engine->io);
    }

    if (!io_get_line(engine->io,string,size))   // EOF
    {
        exit(EXIT_SUCCESS);
    }
}

// engine_send()

void engine_send(engine_t * engine, const char format[], ...)
{
    va_list arg_list;
    char string[StringSize];

    //ASSERT(engine_is_ok(engine));
    assert(format);

    // format

    va_start(arg_list,format);
    vsprintf(string,format,arg_list);
    va_end(arg_list);

    // send

    io_send(engine->io,"%s",string);
}
