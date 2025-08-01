#ifndef MODULE_SIGNING_H
#define MODULE_SIGNING_H

#include "include/types.h"

// Initialize module signing verification
void module_signing_init(void);

// Verify the signature of a kernel module
bool module_verify_signature(const void* module_data, size_t module_size);

#endif // MODULE_SIGNING_H
