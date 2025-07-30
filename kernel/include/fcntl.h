#ifndef _FCNTL_H
#define _FCNTL_H

// Open flags
#define O_RDONLY    0x0000      // Open for reading only
#define O_WRONLY    0x0001      // Open for writing only
#define O_RDWR      0x0002      // Open for reading and writing
#define O_CREAT     0x0040      // Create file if it does not exist
#define O_EXCL      0x0080      // Exclusive use flag
#define O_NOCTTY    0x0100      // Do not assign controlling terminal
#define O_TRUNC     0x0200      // Truncate file to zero length
#define O_APPEND    0x0400      // Set append mode
#define O_NONBLOCK  0x0800      // Non-blocking mode
#define O_DSYNC     0x1000      // Write synchronously
#define O_ASYNC     0x2000      // Asynchronous I/O
#define O_DIRECTORY 0x10000     // Must be a directory
#define O_NOFOLLOW  0x20000     // Do not follow symlinks
#define O_CLOEXEC   0x80000     // Set FD_CLOEXEC

#endif // _FCNTL_H
