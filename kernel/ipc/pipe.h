// RaeenOS Pipe IPC Implementation - Header

#ifndef PIPE_H
#define PIPE_H

#include <stddef.h>
#include <stdint.h>
#include "../include/wait.h"

#define PIPE_DEFAULT_SIZE 4096 // Default size for a pipe buffer, 4KB

// Represents a pipe for inter-process communication
typedef struct {
    uint8_t* buffer;        // The data buffer data buffer
    size_t size;        // The total size of the buffer
    size_t read_pos;    // Index for the next read
    size_t write_pos;   // Index for the next write
    size_t count;       // Number of bytes currently in the pipe
    wait_queue_t* read_queue; // Processes waiting to read
    wait_queue_t* write_queue; // Processes waiting to write
} pipe_t;

/**
 * @brief Creates a new pipe with a given buffer size.
 * 
 * @param size The size of the pipe's buffer.
 * @return A pointer to the new pipe, or NULL on failure.
 */
pipe_t* pipe_create(void);

/**
 * @brief Reads data from a pipe into a buffer.
 * 
 * @param pipe The pipe to read from.
 * @param buffer The destination buffer to store the data.
 * @param size The maximum number of bytes to read.
 * @return The number of bytes actually read.
 */
int pipe_read(pipe_t* pipe, uint8_t* buffer, int size);

/**
 * @brief Writes data from a buffer into a pipe.
 * 
 * @param pipe The pipe to write to.
 * @param buffer The source buffer containing the data.
 * @param size The number of bytes to write.
 * @return The number of bytes actually written.
 */
int pipe_write(pipe_t* pipe, const uint8_t* buffer, int size);

/**
 * @brief Destroys a pipe and frees its resources.
 * 
 * @param pipe The pipe to destroy.
 */
void pipe_destroy(pipe_t* pipe);

#endif // PIPE_H
