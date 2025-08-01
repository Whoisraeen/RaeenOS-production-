#include "permissions.h"
#include "../../kernel/vga.h"

bool has_permission(uid_t user, gid_t group, file_security_t* security, uint16_t requested_access) {
    if (!security) {
        // No security defined, assume full access for now
        return true;
    }

    uint16_t effective_permissions = 0;

    if (user == security->owner_id) {
        effective_permissions |= (security->permissions & S_IRWXU) >> 6;
    } else if (group == security->group_id) {
        effective_permissions |= (security->permissions & S_IRWXG) >> 3;
    } else {
        effective_permissions |= (security->permissions & S_IRWXO) >> 0;
    }

    return (effective_permissions & requested_access) == requested_access;
}
