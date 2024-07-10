#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <assert.h>
#include <sys/types.h>

#ifndef _MSC_VER
#include <unistd.h>
#else
#include <io.h>
#endif

#include "io.h"

// constants

static const bool UseCR = false; // true on Windows

static const int StringSize = 4096;

static const char LF = '\n';
static const char CR = '\r';

// prototypes

static int  my_read  (int fd, char string[], int size);
static void my_write (int fd, const char string[], int size);

// functions

// io_is_ok()

bool io_is_ok(const io_t * io)
{
    // TOOD: Fix this, do we need this?
    return true;
    

    if (io == NULL) return false;

    if (io->name == NULL) return false;

    if (io->in_eof != true && io->in_eof != false) return false;

    if (io->in_size < 0 || io->in_size > BufferSize) return false;
    if (io->out_size < 0 || io->out_size > BufferSize) return false;

    return true;
}

// io_init()

void io_init(io_t * io)
{
    assert(io!=NULL);

    io->in_eof = false;

    io->in_size = 0;
    io->out_size = 0;

    assert(io_is_ok(io));
}

// io_close()

void io_close(io_t * io)
{
#ifndef _MSC_VER

    assert(io_is_ok(io));

    assert(io->out_fd>=0);

    //my_log("> %s EOF\n",io->name);

    if (close(io->out_fd) == -1)
    {
        //my_fatal("io_close(): close(): %s\n",strerror(errno));
    }

    io->out_fd = -1;
#endif
}

// io_get_update()

void io_get_update(io_t * io)
{

    int pos, size;
    int n;

    assert(io_is_ok(io));

    assert(io->in_fd>=0);
    assert(!io->in_eof);

    // init

    pos = io->in_size;

    size = BufferSize - pos;
    if (size <= 0)
    {
        std::cerr << "io_get_update(): buffer overflow\n" << std::endl;
    }

    // read as many data as possible

    n = my_read(io->in_fd,&io->in_buffer[pos],size);
    
    if (n > 0)   // at least one character was read
    {

        // update buffer size

        assert(n>=1&&n<=size);

        io->in_size += n;
        assert(io->in_size>=0&&io->in_size<=BufferSize);

    }
    else     // EOF
    {

        assert(n==0);

        io->in_eof = true;
    }
}

// io_line_ready()

bool io_line_ready(const io_t * io)
{

    assert(io_is_ok(io));

    if (io->in_eof) return true;

    if (memchr(io->in_buffer,LF,io->in_size) != NULL) return true; // buffer contains LF

    return false;
}

// io_get_line()

bool io_get_line(io_t * io, char string[], int size)
{

    int src, dst;
    int c;

    assert(io_is_ok(io));
    assert(string!=NULL);
    assert(size>=256);

    src = 0;
    dst = 0;

    while (true)
    {

        // test for end of buffer

        if (src >= io->in_size)
        {
            if (io->in_eof)
            {                
                //my_log("< %s EOF\n",io->name);
                return false;
            }
            else
            {
                assert(false);
                //my_fatal("io_get_line(): no EOL in buffer\n");
            }
        }

        // test for end of string

        if (dst >= size)
        {
//            my_fatal("io_get_line(): buffer overflow\n");
            assert(false);
        }
        
        // copy the next character

        c = io->in_buffer[src++];

        if (c == LF)   // LF => line complete
        {
            string[dst] = '\0';
            break;
        }
        else if (c != CR)     // skip CRs
        {
            string[dst++] = c;
        }
    }

    // shift the buffer

    assert(src>0);

    io->in_size -= src;
    assert(io->in_size>=0);

    if (io->in_size > 0) memmove(&io->in_buffer[0],&io->in_buffer[src],io->in_size);

    return true;
}

// io_send()

void io_send(io_t * io, const char format[], ...)
{

    va_list arg_list;
    char string[StringSize];
    int len;

    assert(io_is_ok(io));
    assert(format!=NULL);

    assert(io->out_fd>=0);

    // format

    va_start(arg_list,format);
    vsprintf(string,format,arg_list);
    va_end(arg_list);

    // append string to buffer

    len = strlen(string);
    if (io->out_size + len > BufferSize-2)
    {
        assert(false);
     //    my_fatal("io_send(): buffer overflow\n");
    }

    memcpy(&io->out_buffer[io->out_size],string,len);
    io->out_size += len;

    assert(io->out_size>=0&&io->out_size<=BufferSize-2);

    // log

    io->out_buffer[io->out_size] = '\0';
    //my_log("> %s %s\n",io->name,io->out_buffer);

    // append EOL to buffer

    if (UseCR) io->out_buffer[io->out_size++] = CR;
    io->out_buffer[io->out_size++] = LF;

    assert(io->out_size>=0&&io->out_size<=BufferSize);

    // flush buffer

    my_write(io->out_fd,io->out_buffer,io->out_size);

    io->out_size = 0;
}

// my_read()

static int my_read(int fd, char string[], int size)
{
    int n;

    assert(fd>=0);
    assert(string!=NULL);
    assert(size>0);

    do
    {
#ifdef _MSC_VER
        n = _read(fd, string, size);
#else
        n = read(fd,string,size);
#endif
    }
    while (n == -1 && errno == EINTR);

    if (n == -1)
    {
        assert(false);
//        my_fatal("my_read(): read(): %s\n",strerror(errno));
    }
    
    assert(n>=0);

    return n;
}

// my_write()

static void my_write(int fd, const char string[], int size)
{
#ifndef _MSC_VER

    int n;

    assert(fd>=0);
    assert(string!=NULL);
    assert(size>0);

    do
    {

        n = write(fd,string,size);

        // if (n == -1 && errno != EINTR && errno != EPIPE) my_fatal("my_write(): write(): %s\n",strerror(errno));

        if (n == -1)
        {
            if (false)
            {
            }
            else if (errno == EINTR)
            {
                n = 0; // nothing has been written
            }
            else if (errno == EPIPE)
            {
                n = size; // pretend everything has been written
            }
            else
            {
                assert(false);
               // my_fatal("my_write(): write(): %s\n",strerror(errno));
            }
        }

        assert(n>=0);

        string += n;
        size -= n;

    }
    while (size > 0);

    assert(size==0);
#endif
}
