#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include <stdint.h>

// User and Group IDs
typedef uint32_t uid_t;
typedef uint32_t gid_t;

// File permission bits (simplified POSIX-like)
#define S_IRWXU 00700   // Read, write, execute by owner
#define S_IRUSR 00400   // Read by owner
#define S_IWUSR 00200   // Write by owner
#define S_IXUSR 00100   // Execute by owner

#define S_IRWXG 00070   // Read, write, execute by group
#define S_IRGRP 00040   // Read by group
#define S_IWGRP 00020   // Write by group
#define S_IXGRP 00010   // Execute by group

#define S_IRWXO 00007   // Read, write, execute by others
#define S_IROTH 00004   // Read by others
#define S_IWOTH 00002   // Write by others
#define S_IXOTH 00001   // Execute by others

// File security attributes (placeholder)
typedef struct {
    uid_t owner_id;
    gid_t group_id;
    uint16_t permissions; // Bitmask of S_I* constants
    // Add ACLs or other security attributes later
} file_security_t;

// Check if a user has permission to access a file
bool has_permission(uid_t user, gid_t group, file_security_t* security, uint16_t requested_access);

#endif // PERMISSIONS_H
