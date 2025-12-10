#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdarg.h>
#include <errno.h>
#include "include/MicroMaxEngine.h"

/* -------------------- Constants -------------------- */

#define BUFFER_SIZE 16384
#define RESPONSE_BUFFER_SIZE 32768
#define LF '\n'
#define CR '\r'
#define READY_MARKER "__READY__"

/* -------------------- IO Buffer Structure -------------------- */

typedef struct {
    int in_fd;
    int out_fd;
    int in_eof;
    int in_size;
    char in_buffer[BUFFER_SIZE];
} io_buffer_t;

/* -------------------- Engine State -------------------- */

static struct {
    int initialized;
    pthread_t engine_thread;
    io_buffer_t to_engine;
    io_buffer_t from_engine;
    char line_buffer[BUFFER_SIZE];
    char response_buffer[RESPONSE_BUFFER_SIZE];
    char* ini_path;
} engine_state = {0};

/* -------------------- External Declarations -------------------- */

// The engine's file descriptor for reading commands
extern int __engineFD__;

// The main entry point of the Fairymax engine
extern int main_fairymax(int argc, char *argv[]);

/* -------------------- IO Helper Functions -------------------- */

static void io_init(io_buffer_t* io) {
    io->in_eof = 0;
    io->in_size = 0;
}

static int my_read(int fd, char* buffer, int size) {
    int n;
    do {
        n = (int)read(fd, buffer, size);
    } while (n == -1 && errno == EINTR);
    
    if (n == -1) {
        return 0;
    }
    return n;
}

static void my_write(int fd, const char* buffer, int size) {
    int n;
    while (size > 0) {
        do {
            n = (int)write(fd, buffer, size);
        } while (n == -1 && errno == EINTR);
        
        if (n == -1) {
            if (errno == EPIPE) {
                return; // Pretend everything was written
            }
            return;
        }
        buffer += n;
        size -= n;
    }
}

static void io_get_update(io_buffer_t* io) {
    int pos = io->in_size;
    int size = BUFFER_SIZE - pos;
    
    if (size <= 0) {
        return;
    }
    
    int n = my_read(io->in_fd, &io->in_buffer[pos], size);
    
    if (n > 0) {
        io->in_size += n;
    } else {
        io->in_eof = 1;
    }
}

static int io_line_ready(const io_buffer_t* io) {
    if (io->in_eof) return 1;
    if (memchr(io->in_buffer, LF, io->in_size) != NULL) return 1;
    return 0;
}

static int io_get_line(io_buffer_t* io, char* string, int size) {
    int src = 0;
    int dst = 0;
    
    while (1) {
        if (src >= io->in_size) {
            if (io->in_eof) {
                return 0;
            }
            return 0;
        }
        
        if (dst >= size - 1) {
            break;
        }
        
        char c = io->in_buffer[src++];
        
        if (c == LF) {
            string[dst] = '\0';
            break;
        } else if (c != CR) {
            string[dst++] = c;
        }
    }
    
    string[dst] = '\0';
    
    // Shift buffer
    io->in_size -= src;
    if (io->in_size > 0) {
        memmove(&io->in_buffer[0], &io->in_buffer[src], io->in_size);
    }
    
    return 1;
}

static void io_send(io_buffer_t* io, const char* format, ...) {
    va_list arg_list;
    char string[4096];
    
    va_start(arg_list, format);
    vsnprintf(string, sizeof(string), format, arg_list);
    va_end(arg_list);
    
    // Write the formatted string
    int len = (int)strlen(string);
    my_write(io->out_fd, string, len);
    
    // Append newline
    my_write(io->out_fd, "\n", 1);
}

// Read a single line from engine (blocks until available)
static void engine_get_line(char* string, int size) {
    while (!io_line_ready(&engine_state.from_engine)) {
        io_get_update(&engine_state.from_engine);
    }
    
    if (!io_get_line(&engine_state.from_engine, string, size)) {
        string[0] = '\0';
    }
}

// Read all lines until __READY__ marker, accumulate in response_buffer
// Returns pointer to response_buffer (empty string if no output before marker)
static const char* read_until_ready(void) {
    engine_state.response_buffer[0] = '\0';
    int response_len = 0;
    
    while (1) {
        engine_get_line(engine_state.line_buffer, BUFFER_SIZE);
        
        // Check if this is the ready marker
        if (strcmp(engine_state.line_buffer, READY_MARKER) == 0) {
            break;
        }
        
        // Accumulate this line in the response buffer
        int line_len = (int)strlen(engine_state.line_buffer);
        if (response_len + line_len + 2 < RESPONSE_BUFFER_SIZE) {
            if (response_len > 0) {
                // Add newline separator between lines
                engine_state.response_buffer[response_len++] = '\n';
            }
            memcpy(&engine_state.response_buffer[response_len], engine_state.line_buffer, line_len);
            response_len += line_len;
            engine_state.response_buffer[response_len] = '\0';
        }
    }
    
    return engine_state.response_buffer;
}

/* -------------------- Engine Thread -------------------- */

static void* engine_thread_func(void* data) {
    (void)data;
    
    // Set up argv for the engine
    // argv[0] = program name (not used meaningfully)
    // argv[1] = ini file path
    char* argv[3];
    argv[0] = "fairymax";
    argv[1] = engine_state.ini_path;
    argv[2] = NULL;
    
    int argc = engine_state.ini_path ? 2 : 1;
    
    main_fairymax(argc, argv);
    
    return NULL;
}

/* -------------------- getMicroMaxIni (called by Fairymax.c) -------------------- */

void getMicroMaxIni(char* ini) {
    if (engine_state.ini_path) {
        strcpy(ini, engine_state.ini_path);
    } else {
        // Default fallback - should not happen if engine started correctly
        strcpy(ini, "fmax.ini");
    }
}

/* -------------------- engine_printf (called by Fairymax.c instead of printf) -------------------- */

void engine_printf(const char* format, ...) {
    if (!engine_state.initialized) {
        return;
    }
    
    va_list arg_list;
    char buffer[4096];
    
    va_start(arg_list, format);
    vsnprintf(buffer, sizeof(buffer), format, arg_list);
    va_end(arg_list);
    
    // Write directly to the from_engine pipe (no newline added - printf doesn't add one automatically)
    int len = (int)strlen(buffer);
    my_write(engine_state.from_engine.out_fd, buffer, len);
}

/* -------------------- Public API -------------------- */

const char* micromax_engine_start(const char* iniFilePath) {
    if (engine_state.initialized) {
        engine_state.response_buffer[0] = '\0';
        return engine_state.response_buffer;
    }
    
    // Store ini path
    if (iniFilePath) {
        engine_state.ini_path = strdup(iniFilePath);
    }
    
    // Create pipes
    int from_fd[2], to_fd[2];
    
    if (pipe(from_fd) == -1 || pipe(to_fd) == -1) {
        engine_state.response_buffer[0] = '\0';
        return engine_state.response_buffer;
    }
    
    engine_state.from_engine.in_fd = from_fd[0];
    engine_state.from_engine.out_fd = from_fd[1];
    
    engine_state.to_engine.in_fd = to_fd[0];
    engine_state.to_engine.out_fd = to_fd[1];
    
    io_init(&engine_state.from_engine);
    io_init(&engine_state.to_engine);
    
    // Set the engine's input file descriptor
    __engineFD__ = engine_state.to_engine.in_fd;
    
    // Note: We do NOT redirect stdout here. Instead, Fairymax uses engine_printf()
    // which writes directly to our pipe. This keeps stdout available for Swift's print().
    
    // Create engine thread
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&engine_state.engine_thread, &attr, engine_thread_func, NULL);
    pthread_attr_destroy(&attr);
    
    engine_state.initialized = 1;
    
    // Wait for the engine to output its init messages and the first __READY__
    // This captures the "tellics say Fairy-Max..." banners
    const char* init_output = read_until_ready();
    
    return init_output;
}

void micromax_engine_stop(void) {
    if (!engine_state.initialized) {
        return;
    }
    
    // Send quit command (don't wait for response since engine will exit)
    io_send(&engine_state.to_engine, "quit");
    
    // Give engine time to process quit
    usleep(50000); // 50ms
    
    // Close pipes
    close(engine_state.to_engine.in_fd);
    close(engine_state.to_engine.out_fd);
    close(engine_state.from_engine.in_fd);
    close(engine_state.from_engine.out_fd);
    
    // Free ini path
    if (engine_state.ini_path) {
        free(engine_state.ini_path);
        engine_state.ini_path = NULL;
    }
    
    engine_state.initialized = 0;
}

const char* micromax_engine_send_command(const char* command) {
    if (!engine_state.initialized || !command) {
        engine_state.response_buffer[0] = '\0';
        return engine_state.response_buffer;
    }
    
    // Send the command
    io_send(&engine_state.to_engine, "%s", command);
    
    // Read all output until we see the __READY__ marker
    return read_until_ready();
}

// Legacy functions kept for compatibility
void micromax_engine_write(const char* command) {
    if (!engine_state.initialized || !command) {
        return;
    }
    io_send(&engine_state.to_engine, "%s", command);
}

const char* micromax_engine_read(void) {
    if (!engine_state.initialized) {
        engine_state.line_buffer[0] = '\0';
        return engine_state.line_buffer;
    }
    engine_get_line(engine_state.line_buffer, BUFFER_SIZE);
    return engine_state.line_buffer;
}

int micromax_engine_has_response(void) {
    if (!engine_state.initialized) {
        return 0;
    }
    return io_line_ready(&engine_state.from_engine);
}
