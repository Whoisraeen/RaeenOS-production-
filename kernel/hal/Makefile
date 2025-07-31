# HAL (Hardware Abstraction Layer) Makefile for RaeenOS
# Version: 1.0
# Date: 2025-07-31

# Architecture detection
ARCH ?= $(shell uname -m)
ifeq ($(ARCH),x86_64)
    TARGET_ARCH = x86_64
    ARCH_CFLAGS = -m64 -march=x86-64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2
    ARCH_ASFLAGS = -64
else ifeq ($(ARCH),aarch64)
    TARGET_ARCH = arm64
    ARCH_CFLAGS = -march=armv8-a -mgeneral-regs-only
    ARCH_ASFLAGS = 
else
    $(error Unsupported architecture: $(ARCH))
endif

# Build configuration
BUILD_DIR ?= ../../build/hal
OBJ_DIR = $(BUILD_DIR)/obj
DEP_DIR = $(BUILD_DIR)/deps
LIB_DIR = $(BUILD_DIR)/lib
TEST_DIR = $(BUILD_DIR)/tests

# Compiler and tools
CC = gcc
AS = nasm
AR = ar
LD = ld
OBJCOPY = objcopy
OBJDUMP = objdump

# Compiler flags
CFLAGS = -std=c11 -Wall -Wextra -Werror -O2 -g
CFLAGS += -ffreestanding -fno-stack-protector -fno-pic -fno-pie
CFLAGS += -nostdlib -nostdinc -fno-builtin
CFLAGS += -mcmodel=kernel -mno-red-zone
CFLAGS += $(ARCH_CFLAGS)
CFLAGS += -I. -I../include -I..
CFLAGS += -DHAL_VERSION_MAJOR=1 -DHAL_VERSION_MINOR=0
CFLAGS += -DTARGET_ARCH_$(shell echo $(TARGET_ARCH) | tr a-z A-Z)

# Debug and optimization flags
ifdef DEBUG
    CFLAGS += -DDEBUG -DHAL_DEBUG -O0 -g3
else
    CFLAGS += -DNDEBUG -O2 -g1
endif

ifdef ENABLE_ASSERTIONS
    CFLAGS += -DHAL_ENABLE_ASSERTIONS
endif

ifdef ENABLE_TRACING
    CFLAGS += -DHAL_ENABLE_TRACING
endif

# Assembly flags
ASFLAGS = -f elf64 -g -F dwarf
ASFLAGS += $(ARCH_ASFLAGS)

# Linker flags
LDFLAGS = -nostdlib -static

# Include paths
INCLUDES = -I../include -I. -I..

# Source files
CORE_SOURCES = hal_core.c
DEVICE_SOURCES = device/hal_device_manager.c
PERFORMANCE_SOURCES = performance/hal_performance.c
DETECTION_SOURCES = detection/hal_hardware_detection.c
TEST_SOURCES = tests/hal_integration_tests.c

# Architecture-specific sources
ifeq ($(TARGET_ARCH),x86_64)
    ARCH_SOURCES = x86_64/hal_x86_64.c
    ARCH_ASM_SOURCES = x86_64/hal_x86_64_asm.S
else ifeq ($(TARGET_ARCH),arm64)
    ARCH_SOURCES = arm64/hal_arm64.c
    ARCH_ASM_SOURCES = arm64/hal_arm64_asm.S
endif

# All sources
ALL_SOURCES = $(CORE_SOURCES) $(DEVICE_SOURCES) $(PERFORMANCE_SOURCES) $(DETECTION_SOURCES) $(ARCH_SOURCES)
ALL_ASM_SOURCES = $(ARCH_ASM_SOURCES)
ALL_TEST_SOURCES = $(TEST_SOURCES)

# Object files
CORE_OBJECTS = $(CORE_SOURCES:%.c=$(OBJ_DIR)/%.o)
DEVICE_OBJECTS = $(DEVICE_SOURCES:%.c=$(OBJ_DIR)/%.o)
PERFORMANCE_OBJECTS = $(PERFORMANCE_SOURCES:%.c=$(OBJ_DIR)/%.o)
DETECTION_OBJECTS = $(DETECTION_SOURCES:%.c=$(OBJ_DIR)/%.o)
ARCH_OBJECTS = $(ARCH_SOURCES:%.c=$(OBJ_DIR)/%.o)
ARCH_ASM_OBJECTS = $(ARCH_ASM_SOURCES:%.S=$(OBJ_DIR)/%.o)
TEST_OBJECTS = $(ALL_TEST_SOURCES:%.c=$(OBJ_DIR)/%.o)

ALL_OBJECTS = $(CORE_OBJECTS) $(DEVICE_OBJECTS) $(PERFORMANCE_OBJECTS) $(DETECTION_OBJECTS) $(ARCH_OBJECTS) $(ARCH_ASM_OBJECTS)

# Dependency files
DEPS = $(ALL_OBJECTS:%.o=$(DEP_DIR)/%.d) $(TEST_OBJECTS:%.o=$(DEP_DIR)/%.d)

# Targets
HAL_LIBRARY = $(LIB_DIR)/libhal.a
HAL_TEST_BINARY = $(TEST_DIR)/hal_tests

# Default target
.PHONY: all
all: $(HAL_LIBRARY)

# Create directories
$(OBJ_DIR) $(DEP_DIR) $(LIB_DIR) $(TEST_DIR):
	@mkdir -p $@

$(OBJ_DIR)/device $(OBJ_DIR)/performance $(OBJ_DIR)/detection $(OBJ_DIR)/tests:
	@mkdir -p $@

$(OBJ_DIR)/$(TARGET_ARCH):
	@mkdir -p $@

$(DEP_DIR)/device $(DEP_DIR)/performance $(DEP_DIR)/detection $(DEP_DIR)/tests:
	@mkdir -p $@

$(DEP_DIR)/$(TARGET_ARCH):
	@mkdir -p $@

# Build HAL library
$(HAL_LIBRARY): $(ALL_OBJECTS) | $(LIB_DIR)
	@echo "Creating HAL library..."
	$(AR) rcs $@ $(ALL_OBJECTS)
	@echo "HAL library created: $@"

# Core HAL objects
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR) $(DEP_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -MF $(DEP_DIR)/$*.d -c $< -o $@

# Device management objects
$(OBJ_DIR)/device/%.o: device/%.c | $(OBJ_DIR)/device $(DEP_DIR)/device
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -MF $(DEP_DIR)/device/$*.d -c $< -o $@

# Performance objects
$(OBJ_DIR)/performance/%.o: performance/%.c | $(OBJ_DIR)/performance $(DEP_DIR)/performance
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -MF $(DEP_DIR)/performance/$*.d -c $< -o $@

# Detection objects
$(OBJ_DIR)/detection/%.o: detection/%.c | $(OBJ_DIR)/detection $(DEP_DIR)/detection
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -MF $(DEP_DIR)/detection/$*.d -c $< -o $@

# Test objects
$(OBJ_DIR)/tests/%.o: tests/%.c | $(OBJ_DIR)/tests $(DEP_DIR)/tests
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -MF $(DEP_DIR)/tests/$*.d -c $< -o $@

# Architecture-specific C objects
$(OBJ_DIR)/$(TARGET_ARCH)/%.o: $(TARGET_ARCH)/%.c | $(OBJ_DIR)/$(TARGET_ARCH) $(DEP_DIR)/$(TARGET_ARCH)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -MF $(DEP_DIR)/$(TARGET_ARCH)/$*.d -c $< -o $@

# Architecture-specific assembly objects
$(OBJ_DIR)/$(TARGET_ARCH)/%.o: $(TARGET_ARCH)/%.S | $(OBJ_DIR)/$(TARGET_ARCH) $(DEP_DIR)/$(TARGET_ARCH)
	@echo "Assembling $<..."
ifeq ($(TARGET_ARCH),x86_64)
	$(AS) $(ASFLAGS) $< -o $@
else ifeq ($(TARGET_ARCH),arm64)
	$(CC) $(CFLAGS) -c $< -o $@
endif

# Build tests
.PHONY: tests
tests: $(HAL_TEST_BINARY)

$(HAL_TEST_BINARY): $(TEST_OBJECTS) $(HAL_LIBRARY) | $(TEST_DIR)
	@echo "Building HAL tests..."
	$(CC) $(LDFLAGS) -o $@ $(TEST_OBJECTS) -L$(LIB_DIR) -lhal
	@echo "HAL tests built: $@"

# Run tests
.PHONY: test
test: $(HAL_TEST_BINARY)
	@echo "Running HAL tests..."
	$(HAL_TEST_BINARY)

# Install targets
.PHONY: install
install: $(HAL_LIBRARY)
	@echo "Installing HAL library..."
	@mkdir -p $(DESTDIR)/usr/lib/raeenos
	@mkdir -p $(DESTDIR)/usr/include/raeenos/hal
	cp $(HAL_LIBRARY) $(DESTDIR)/usr/lib/raeenos/
	cp ../include/hal_interface.h $(DESTDIR)/usr/include/raeenos/hal/
	cp device/hal_device_manager.h $(DESTDIR)/usr/include/raeenos/hal/
	cp performance/hal_performance.h $(DESTDIR)/usr/include/raeenos/hal/
	cp detection/hal_hardware_detection.h $(DESTDIR)/usr/include/raeenos/hal/
	@echo "HAL installed to $(DESTDIR)"

# Clean targets
.PHONY: clean
clean:
	@echo "Cleaning HAL build files..."
	rm -rf $(BUILD_DIR)

.PHONY: distclean
distclean: clean
	@echo "Cleaning all generated files..."
	find . -name "*.o" -delete
	find . -name "*.d" -delete
	find . -name "*.a" -delete
	find . -name "*~" -delete

# Debug targets
.PHONY: debug
debug:
	$(MAKE) DEBUG=1 ENABLE_ASSERTIONS=1

.PHONY: debug-tests
debug-tests:
	$(MAKE) tests DEBUG=1 ENABLE_ASSERTIONS=1 ENABLE_TRACING=1

# Static analysis
.PHONY: analyze
analyze:
	@echo "Running static analysis..."
	cppcheck --enable=all --std=c11 --platform=unix64 $(ALL_SOURCES) $(ALL_TEST_SOURCES)

.PHONY: lint
lint:
	@echo "Running code linting..."
	splint +posixlib $(INCLUDES) $(ALL_SOURCES)

# Code formatting
.PHONY: format
format:
	@echo "Formatting code..."
	clang-format -i $(ALL_SOURCES) $(ALL_TEST_SOURCES)

# Documentation
.PHONY: docs
docs:
	@echo "Generating documentation..."
	doxygen Doxyfile

# Performance profiling
.PHONY: profile
profile: $(HAL_TEST_BINARY)
	@echo "Running performance profiling..."
	perf record -g $(HAL_TEST_BINARY)
	perf report

# Memory debugging
.PHONY: memcheck
memcheck: $(HAL_TEST_BINARY)
	@echo "Running memory check..."
	valgrind --tool=memcheck --leak-check=full $(HAL_TEST_BINARY)

# Code coverage
.PHONY: coverage
coverage:
	$(MAKE) CFLAGS="$(CFLAGS) --coverage" LDFLAGS="$(LDFLAGS) --coverage"
	$(HAL_TEST_BINARY)
	gcov $(ALL_SOURCES)
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory coverage-html

# Benchmarking
.PHONY: benchmark
benchmark: $(HAL_TEST_BINARY)
	@echo "Running HAL benchmarks..."
	$(HAL_TEST_BINARY) --benchmark

# Size analysis
.PHONY: size
size: $(HAL_LIBRARY)
	@echo "HAL library size analysis:"
	size $(HAL_LIBRARY)
	@echo "Object file sizes:"
	size $(ALL_OBJECTS) | sort -n -k1

# Assembly listings
.PHONY: listings
listings: $(ALL_OBJECTS)
	@echo "Generating assembly listings..."
	@mkdir -p $(BUILD_DIR)/listings
	$(foreach obj,$(ALL_OBJECTS),$(OBJDUMP) -d $(obj) > $(BUILD_DIR)/listings/$(notdir $(obj:.o=.asm));)

# Symbol analysis
.PHONY: symbols
symbols: $(HAL_LIBRARY)
	@echo "Symbol analysis:"
	nm -C --size-sort $(HAL_LIBRARY)

# Dependency analysis
.PHONY: deps
deps:
	@echo "Dependency analysis:"
	$(CC) $(CFLAGS) $(INCLUDES) -MM $(ALL_SOURCES)

# Architecture-specific targets
.PHONY: x86_64
x86_64:
	$(MAKE) TARGET_ARCH=x86_64

.PHONY: arm64
arm64:
	$(MAKE) TARGET_ARCH=arm64

# Help target
.PHONY: help
help:
	@echo "RaeenOS HAL Makefile"
	@echo "==================="
	@echo ""
	@echo "Available targets:"
	@echo "  all          - Build HAL library (default)"
	@echo "  tests        - Build test suite"
	@echo "  test         - Run tests"
	@echo "  install      - Install HAL library and headers"
	@echo "  clean        - Clean build files"
	@echo "  distclean    - Clean all generated files"
	@echo "  debug        - Build with debug symbols"
	@echo "  debug-tests  - Build tests with debug and tracing"
	@echo "  analyze      - Run static analysis (cppcheck)"
	@echo "  lint         - Run code linting (splint)"
	@echo "  format       - Format code (clang-format)"
	@echo "  docs         - Generate documentation (doxygen)"
	@echo "  profile      - Profile performance (perf)"
	@echo "  memcheck     - Check memory usage (valgrind)"
	@echo "  coverage     - Generate code coverage report"
	@echo "  benchmark    - Run performance benchmarks"
	@echo "  size         - Analyze binary sizes"
	@echo "  listings     - Generate assembly listings"
	@echo "  symbols      - Analyze symbols"
	@echo "  deps         - Show dependencies"
	@echo "  x86_64       - Build for x86-64 architecture"
	@echo "  arm64        - Build for ARM64 architecture"
	@echo "  help         - Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  DEBUG=1      - Enable debug build"
	@echo "  ARCH=arch    - Target architecture (x86_64, arm64)"
	@echo "  DESTDIR=dir  - Installation prefix"

# Include dependency files
-include $(DEPS)

# Phony targets
.PHONY: all clean distclean install test tests debug debug-tests analyze lint format docs profile memcheck coverage benchmark size listings symbols deps x86_64 arm64 help