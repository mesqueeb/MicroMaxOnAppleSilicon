#ifndef MICROMAX_ENGINE_H
#define MICROMAX_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Start the MicroMax chess engine.
 * Blocks until the engine is ready for commands.
 * @param iniFilePath Path to the fmax.ini configuration file
 * @return The engine's initialization output (banners, etc.) - static buffer, do not free
 */
const char* micromax_engine_start(const char* iniFilePath);

/**
 * Stop the MicroMax chess engine.
 */
void micromax_engine_stop(void);

/**
 * Send a command to the engine and wait for the response.
 * Blocks until the engine has processed the command and is ready for the next one.
 * @param command The WinBoard protocol command to send
 * @return The engine's response (may be empty string if command produces no output) - static buffer, do not free
 */
const char* micromax_engine_send_command(const char* command);

/**
 * Send a command to the engine without waiting for response.
 * @param command The WinBoard protocol command to send
 * @deprecated Use micromax_engine_send_command instead for proper synchronization
 */
void micromax_engine_write(const char* command);

/**
 * Read a single line from the engine.
 * Blocks until a complete line is available.
 * @return The response string (static buffer, do not free)
 * @deprecated Use micromax_engine_send_command instead for proper synchronization
 */
const char* micromax_engine_read(void);

/**
 * Check if there's a response available from the engine.
 * @return 1 if a line is ready, 0 otherwise
 */
int micromax_engine_has_response(void);

/**
 * Printf replacement for the engine - writes to the communication pipe instead of stdout.
 * This allows Swift's print() to work normally while engine output goes to the pipe.
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void engine_printf(const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif /* MICROMAX_ENGINE_H */
