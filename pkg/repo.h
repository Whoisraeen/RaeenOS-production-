#ifndef REPO_H
#define REPO_H

#include <stdint.h>

// Package metadata structure (from pkg.h)
#include "pkg.h"

// Initialize package repository
void repo_init(void);

// Search for a package in the repository
package_metadata_t* repo_search(const char* package_name);

// Download a package from the repository
int repo_download(const char* package_name, const char* destination_path);

#endif // REPO_H
