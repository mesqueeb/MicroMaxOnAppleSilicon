#ifndef IO_H
#define IO_H

typedef signed int sint32;

const int BufferSize = 16384;

struct io_t
{
    int in_fd;
    int out_fd;

    const char * name;

    bool in_eof;

    sint32 in_size;
    sint32 out_size;

    char in_buffer[BufferSize];
    char out_buffer[BufferSize];
};

extern bool io_is_ok      (const io_t * io);

extern void io_init       (io_t * io);
extern void io_close      (io_t * io);

extern void io_get_update (io_t * io);

extern bool io_line_ready (const io_t * io);
extern bool io_get_line   (io_t * io, char string[], int size);

extern void io_send       (io_t * io, const char format[], ...);

#endif