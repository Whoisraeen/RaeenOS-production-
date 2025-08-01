# RaeenOS Makefile

# Toolchain
ASM = nasm
CC = gcc
LD = ld
MKDIR = mkdir -p

# Directories
SRCDIRS := . kernel acpi audio gpu input network nvme pci usb
SRCDIRS := $(SRCDIRS) kernel/fs kernel/hal kernel/ipc kernel/net kernel/process kernel/security kernel/ui
BUILDDIR = build
TARGET = $(BUILDDIR)/kernel/raeenos.bin

# Flags
ASMFLAGS = -f elf64
CFLAGS = -std=gnu99 -m64 -ffreestanding -O2 -Wall -Wextra -I. -Ikernel -Idrivers
LDFLAGS = -T kernel/linker.ld -nostdlib

# Find all source files
ASM_SRCS := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.asm))
C_SRCS := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))

# Object files
OBJS := $(patsubst %.asm,$(BUILDDIR)/%.o,$(ASM_SRCS))
OBJS += $(patsubst %.c,$(BUILDDIR)/%.o,$(C_SRCS))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(MKDIR) $(@D)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(BUILDDIR)/%.o: %.c
	@$(MKDIR) $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.asm
	@$(MKDIR) $(@D)
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf $(BUILDDIR)
