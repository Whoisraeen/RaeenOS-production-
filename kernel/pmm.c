#include "pmm.h"
#include "vga.h"
#include "string.h"

static uint64_t* pmm_bitmap = NULL;
static size_t pmm_total_frames = 0;
static size_t pmm_bitmap_size_in_dwords = 0;

// ... (rest of the pmm.c file remains the same)
