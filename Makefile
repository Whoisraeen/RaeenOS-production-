# RaeenOS Master Makefile

# Tools
ifeq ($(OS),Windows_NT)
    DEVKIT_PATH ?= C:/devkit-i686
    CC = $(DEVKIT_PATH)/bin/i686-elf-gcc.exe
    AS = $(DEVKIT_PATH)/bin/i686-elf-as.exe
    LD = $(DEVKIT_PATH)/bin/i686-elf-ld.exe
    NASM = $(DEVKIT_PATH)/bin/nasm.exe
    MKDIR = mkdir
    CP = copy /b
    RM = del /q
    RMDIR = rmdir /s /q
else
    # Check if cross-compiler is available, fallback to system GCC
    CC := $(shell which i686-elf-gcc 2>/dev/null || echo gcc)
    AS := $(shell which i686-elf-as 2>/dev/null || echo as)
    LD := $(shell which i686-elf-ld 2>/dev/null || echo ld)
    NASM = nasm
    MKDIR = mkdir -p
    CP = cat
    RM = rm -f
    RMDIR = rm -rf
endif

# Build directories
BUILD_DIR = build
SRC_DIR = .
KERNEL_DIR = $(SRC_DIR)/kernel
BOOT_DIR = $(SRC_DIR)/boot

# Kernel sources - collect all C files recursively
KERNEL_C_SOURCES = $(shell find $(KERNEL_DIR) -name "*.c" 2>/dev/null) $(shell find $(SRC_DIR)/drivers -name "*.c" 2>/dev/null)
USERLAND_C_SOURCES = $(shell find $(SRC_DIR)/userland -name "*.c" 2>/dev/null) $(shell find $(SRC_DIR)/pkg -name "*.c" 2>/dev/null)
KERNEL_ASM_SOURCES = $(shell find $(KERNEL_DIR) -name "*.asm" 2>/dev/null)

# Generate object file paths
KERNEL_C_OBJECTS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(KERNEL_C_SOURCES))
KERNEL_ASM_OBJECTS = $(patsubst %.asm, $(BUILD_DIR)/%.o, $(KERNEL_ASM_SOURCES))
USERLAND_OBJECTS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(USERLAND_C_SOURCES))

KERNEL_OBJECTS = $(KERNEL_C_OBJECTS) $(KERNEL_ASM_OBJECTS)

# Bootloader sources
BOOT_SOURCES = $(BOOT_DIR)/boot.asm
BOOT_OBJECT = $(BUILD_DIR)/boot.bin

# Output
OS_IMAGE = $(BUILD_DIR)/os-image.bin
KERNEL_BIN = $(BUILD_DIR)/kernel.bin

# Flags
CFLAGS = -m32 -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -Wall -Wextra -I$(KERNEL_DIR)/include -I$(KERNEL_DIR)
ASFLAGS = -f elf32

# Adjust linker flags based on available tools
ifeq ($(LD),ld)
    LDFLAGS = -m elf_i386 -T $(KERNEL_DIR)/linker.ld --no-relax
else
    LDFLAGS = -m elf_i386 -T $(KERNEL_DIR)/linker.ld
endif

# Additional outputs
USERLAND_BIN = $(BUILD_DIR)/userland.bin

.PHONY: all clean build kernel userland test debug help format lint

all: build

build: $(OS_IMAGE)

kernel: $(KERNEL_BIN)

userland: $(USERLAND_BIN)

debug:
	@echo "Kernel C Sources: $(KERNEL_C_SOURCES)"
	@echo "Kernel ASM Sources: $(KERNEL_ASM_SOURCES)"  
	@echo "Userland Sources: $(USERLAND_C_SOURCES)"
	@echo "Kernel Objects: $(KERNEL_OBJECTS)"

help:
	@echo "RaeenOS Build System"
	@echo "Available targets:"
	@echo "  all      - Build complete OS image (default)"
	@echo "  kernel   - Build kernel only"
	@echo "  userland - Build userland only"
	@echo "  clean    - Clean build files"
	@echo "  debug    - Show build variables"
	@echo "  format   - Format C source files using clang-format"
	@echo "  lint     - Run static analysis on C source files using clang-tidy"
	@echo "  help     - Show this help"

format:
	@echo "Formatting C source files..."
	find $(SRC_DIR) -name "*.c" -o -name "*.h" | xargs clang-format -i
	@echo "Formatting complete."

lint:
	@echo "Running static analysis..."
	find $(SRC_DIR) -name "*.c" | xargs clang-tidy $(CFLAGS) --
	@echo "Static analysis complete."

test: $(OS_IMAGE)
	@echo "Running basic build verification..."
	@if [ -f $(OS_IMAGE) ]; then echo "✓ OS image created successfully"; else echo "✗ OS image build failed"; exit 1; fi
	@if [ -f $(KERNEL_BIN) ]; then echo "✓ Kernel binary created successfully"; else echo "✗ Kernel binary build failed"; exit 1; fi

$(OS_IMAGE): $(BOOT_OBJECT) $(KERNEL_BIN)
ifeq ($(OS),Windows_NT)
	$(CP) $(BOOT_OBJECT) + $(KERNEL_BIN) $(OS_IMAGE)
else
	cat $(BOOT_OBJECT) $(KERNEL_BIN) > $(OS_IMAGE)
endif

$(KERNEL_BIN): $(KERNEL_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

$(USERLAND_BIN): $(USERLAND_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

# Pattern rules for C files
$(BUILD_DIR)/%.o: %.c
	@$(MKDIR) $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rules for ASM files  
$(BUILD_DIR)/%.o: %.asm
	@$(MKDIR) $(@D)
	$(NASM) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/boot.bin: $(BOOT_SOURCES)
	@$(MKDIR) $(@D)
	$(NASM) -f bin $< -o $@

clean:
	$(RMDIR) $(BUILD_DIR)
