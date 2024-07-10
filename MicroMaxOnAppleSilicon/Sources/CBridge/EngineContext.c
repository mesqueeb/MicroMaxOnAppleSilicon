#include <unistd.h>
#include <sys/time.h>
#include <stdio.h> // ???
//#include "Application/Difficulty.hpp"

#pragma GCC diagnostic warning "-w"

/* -------------------- Engine Parameters -------------------- */

typedef unsigned ThinkTime;

// The time that the engine should think, zero if not specified
ThinkTime __thinkLimit__;

// This is the number of lines the engine should think
unsigned __thinkMulti__;

// This is the number of lines that should be reported
unsigned __displayMultiPV__ = 1;

// The rating performance for the engine
//Difficulty __difficulty__;

// File descriptor for the engine
extern int __engineFD__;

static int waitForLine(int fd, long seconds, long useconds)
{
#ifndef _MSC_VER
    int retval;
    fd_set readfds;
    struct timeval timeout;
    
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    timeout.tv_sec  = seconds;
    timeout.tv_usec = (int) useconds;
    
    retval = select(fd+1, &readfds, 0, 0, &timeout);
    
    if (retval == -1)
    {
        return -1;
    }
    else if (retval && FD_ISSET(fd, &readfds))
    {
        return 1;
    }
    else
    {
        return 0;
  	}
#else
    return 0;
#endif
} 

// C doesn't have built in boolean types...
typedef int bool;

#define true 1
#define false 0

// Read a line for the engne as directed by it's file descriptor
extern bool readLineForEngine(char buffer[], int size)
{
#ifdef ENGINE_DEBUG
    char *tempBuffer = buffer;
#endif
    
    char c = 0;
    
    while (1)
    {
        // Block until the next character is available
        read(__engineFD__, &c, 1);
        
        if (c != '\n')
        {
            *buffer++ = c;
        }
        else
        {
            break;
        }
    }

#ifdef ENGINE_DEBUG
    sprintf(tempBuffer, "Received in EngineContext: %s.\n", tempBuffer);
    logToFile(tempBuffer_2);
#endif
    
    *buffer = '\0';
    
    return true;
}

extern bool isInputAvailable()
{
    return waitForLine(__engineFD__, 0, 0);
}
