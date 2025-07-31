# Security Framework Makefile for RaeenOS
# Builds all security subsystem components

# Security framework source files
SECURITY_SOURCES = \
	security_core.c \
	mac.c \
	sandbox.c \
	memory_protection.c \
	crypto.c \
	audit.c \
	ids.c

# Security framework object files
SECURITY_OBJECTS = $(SECURITY_SOURCES:.c=.o)

# Security framework library
SECURITY_LIB = libsecurity.a

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -O2 -fno-stack-protector -nostdlib -nostdinc
CFLAGS += -ffreestanding -fno-builtin -fno-omit-frame-pointer
CFLAGS += -mno-red-zone -mcmodel=kernel -mno-mmx -mno-sse -mno-sse2
CFLAGS += -I../include -I.. -I../../

# Architecture-specific flags
ifeq ($(ARCH),x86_64)
    CFLAGS += -m64
else ifeq ($(ARCH),arm64)
    CFLAGS += -march=armv8-a
endif

# Debug flags
ifdef DEBUG
    CFLAGS += -g -DDEBUG
endif

# Security-specific flags
CFLAGS += -DSECURITY_FRAMEWORK_VERSION=1
CFLAGS += -DSECURITY_DEBUG_LEVEL=2

# Default target
all: $(SECURITY_LIB)

# Build security library
$(SECURITY_LIB): $(SECURITY_OBJECTS)
	@echo "  AR      $@"
	@ar rcs $@ $(SECURITY_OBJECTS)

# Compile source files
%.o: %.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Security core module
security_core.o: security_core.c security_core.h
	@echo "  CC      $< (core security framework)"
	@$(CC) $(CFLAGS) -DSECURITY_CORE_MODULE -c $< -o $@

# MAC framework module
mac.o: mac.c security_core.h
	@echo "  CC      $< (mandatory access control)"
	@$(CC) $(CFLAGS) -DMAC_MODULE -c $< -o $@

# Sandbox framework module
sandbox.o: sandbox.c security_core.h
	@echo "  CC      $< (application sandboxing)"
	@$(CC) $(CFLAGS) -DSANDBOX_MODULE -c $< -o $@

# Memory protection module
memory_protection.o: memory_protection.c security_core.h
	@echo "  CC      $< (memory protection and ASLR)"
	@$(CC) $(CFLAGS) -DMEMORY_PROTECTION_MODULE -c $< -o $@

# Cryptographic services module
crypto.o: crypto.c security_core.h
	@echo "  CC      $< (cryptographic services)"
	@$(CC) $(CFLAGS) -DCRYPTO_MODULE -c $< -o $@

# Audit framework module
audit.o: audit.c security_core.h
	@echo "  CC      $< (security audit and logging)"
	@$(CC) $(CFLAGS) -DAUDIT_MODULE -c $< -o $@

# Intrusion detection module
ids.o: ids.c security_core.h
	@echo "  CC      $< (intrusion detection system)"
	@$(CC) $(CFLAGS) -DIDS_MODULE -c $< -o $@

# Clean build artifacts
clean:
	@echo "  CLEAN   security framework"
	@rm -f $(SECURITY_OBJECTS) $(SECURITY_LIB)

# Install security headers
install-headers:
	@echo "  INSTALL security headers"
	@mkdir -p ../../include/security
	@cp security_core.h ../../include/security/
	@cp ../include/security_interface.h ../../include/security/

# Run security framework tests
test: $(SECURITY_LIB)
	@echo "  TEST    security framework"
	@echo "Security framework test suite not yet implemented"

# Generate security documentation
docs:
	@echo "  DOCS    security framework"
	@doxygen security.doxygen 2>/dev/null || echo "Doxygen not available"

# Security static analysis
analyze:
	@echo "  ANALYZE security framework"
	@cppcheck --enable=all --std=c11 $(SECURITY_SOURCES) 2>/dev/null || echo "cppcheck not available"

# Security code coverage
coverage: CFLAGS += --coverage
coverage: $(SECURITY_LIB)
	@echo "  COVERAGE security framework"
	@gcov $(SECURITY_SOURCES) 2>/dev/null || echo "gcov not available"

# Benchmark security performance
benchmark: $(SECURITY_LIB)
	@echo "  BENCHMARK security performance"
	@echo "Security benchmarks not yet implemented"

# Check security compliance
compliance:
	@echo "  COMPLIANCE security framework"
	@echo "Checking security framework compliance..."
	@grep -r "TODO\|FIXME\|XXX" . || echo "No compliance issues found"

# Create security framework package
package: $(SECURITY_LIB)
	@echo "  PACKAGE security framework"
	@tar -czf raeenos-security-framework.tar.gz $(SECURITY_SOURCES) *.h Makefile README.md

# Help target
help:
	@echo "RaeenOS Security Framework Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all           - Build security framework library"
	@echo "  clean         - Remove build artifacts"
	@echo "  install-headers - Install security headers"
	@echo "  test          - Run security tests"
	@echo "  docs          - Generate documentation"
	@echo "  analyze       - Run static analysis"
	@echo "  coverage      - Generate code coverage report"
	@echo "  benchmark     - Run performance benchmarks"
	@echo "  compliance    - Check security compliance"
	@echo "  package       - Create distribution package"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Variables:"
	@echo "  ARCH          - Target architecture (x86_64, arm64)"
	@echo "  DEBUG         - Enable debug build"
	@echo ""
	@echo "Security Modules:"
	@echo "  security_core.o      - Core security framework"
	@echo "  mac.o               - Mandatory Access Control"
	@echo "  sandbox.o           - Application sandboxing"
	@echo "  memory_protection.o - Memory protection and ASLR"
	@echo "  crypto.o            - Cryptographic services"
	@echo "  audit.o             - Security audit and logging"
	@echo "  ids.o               - Intrusion detection system"

# Dependencies
security_core.o: security_core.h ../include/security_interface.h ../include/process_interface.h
mac.o: security_core.h ../include/security_interface.h
sandbox.o: security_core.h ../include/security_interface.h ../process/process.h
memory_protection.o: security_core.h ../include/hal_interface.h ../memory.h
crypto.o: security_core.h ../include/hal_interface.h
audit.o: security_core.h ../fs/vfs.h
ids.o: security_core.h

# Phony targets
.PHONY: all clean install-headers test docs analyze coverage benchmark compliance package help

# Default architecture
ifndef ARCH
    ARCH = x86_64
endif

# Build information
build-info:
	@echo "Security Framework Build Configuration:"
	@echo "  Architecture: $(ARCH)"
	@echo "  Compiler: $(CC)"
	@echo "  Flags: $(CFLAGS)"
	@echo "  Sources: $(SECURITY_SOURCES)"
	@echo "  Objects: $(SECURITY_OBJECTS)"
	@echo "  Library: $(SECURITY_LIB)"