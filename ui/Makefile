# RaeenUI Framework Makefile
# Modern GPU-Accelerated UI Framework for RaeenOS

# RaeenUI source files
RAEENUI_SOURCES = \
	raeenui_core.c \
	raeenui_render.c \
	raeenui_theme.c \
	raeenui_declarative.c

# RaeenUI object files
RAEENUI_OBJECTS = $(RAEENUI_SOURCES:.c=.o)

# RaeenUI library
RAEENUI_LIB = libraeenui.a

# Example and test programs
EXAMPLES = \
	examples/hello_world \
	examples/theme_demo \
	examples/animation_demo \
	examples/ai_chat_demo

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -O2 -fno-stack-protector
CFLAGS += -ffreestanding -fno-builtin -fno-omit-frame-pointer
CFLAGS += -I../include -I.. -I../gpu -I../security -I../kernel

# GPU acceleration flags
CFLAGS += -DRAEENUI_GPU_ACCELERATION=1
CFLAGS += -DRAEENUI_VULKAN_SUPPORT=1

# Debug flags
ifdef DEBUG
    CFLAGS += -g -DDEBUG -DRAEENUI_DEBUG=1
endif

# Architecture-specific flags
ifeq ($(ARCH),x86_64)
    CFLAGS += -m64
else ifeq ($(ARCH),arm64)
    CFLAGS += -march=armv8-a
endif

# RaeenUI-specific flags
CFLAGS += -DRAEENUI_VERSION_MAJOR=1
CFLAGS += -DRAEENUI_VERSION_MINOR=0
CFLAGS += -DRAEENUI_VERSION_PATCH=0

# Default target
all: $(RAEENUI_LIB) examples

# Build RaeenUI library
$(RAEENUI_LIB): $(RAEENUI_OBJECTS)
	@echo "  AR      $@"
	@ar rcs $@ $(RAEENUI_OBJECTS)
	@echo "RaeenUI library built successfully"

# Compile source files
%.o: %.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Build examples
examples: $(EXAMPLES)

examples/hello_world: examples/hello_world.c $(RAEENUI_LIB)
	@echo "  CC      $@"
	@mkdir -p examples
	@$(CC) $(CFLAGS) $< -L. -lraeenui -o $@

examples/theme_demo: examples/theme_demo.c $(RAEENUI_LIB)
	@echo "  CC      $@"
	@mkdir -p examples
	@$(CC) $(CFLAGS) $< -L. -lraeenui -o $@

examples/animation_demo: examples/animation_demo.c $(RAEENUI_LIB)
	@echo "  CC      $@"
	@mkdir -p examples
	@$(CC) $(CFLAGS) $< -L. -lraeenui -o $@

examples/ai_chat_demo: examples/ai_chat_demo.c $(RAEENUI_LIB)
	@echo "  CC      $@"
	@mkdir -p examples
	@$(CC) $(CFLAGS) $< -L. -lraeenui -o $@

# Install RaeenUI
install: $(RAEENUI_LIB)
	@echo "Installing RaeenUI framework..."
	@mkdir -p $(DESTDIR)/usr/lib/raeenos
	@mkdir -p $(DESTDIR)/usr/include/raeenos/ui
	@cp $(RAEENUI_LIB) $(DESTDIR)/usr/lib/raeenos/
	@cp raeenui.h $(DESTDIR)/usr/include/raeenos/ui/
	@echo "RaeenUI framework installed"

# Uninstall RaeenUI
uninstall:
	@echo "Uninstalling RaeenUI framework..."
	@rm -f $(DESTDIR)/usr/lib/raeenos/$(RAEENUI_LIB)
	@rm -f $(DESTDIR)/usr/include/raeenos/ui/raeenui.h
	@echo "RaeenUI framework uninstalled"

# Clean build artifacts
clean:
	@echo "Cleaning RaeenUI build artifacts..."
	@rm -f $(RAEENUI_OBJECTS)
	@rm -f $(RAEENUI_LIB)
	@rm -f $(EXAMPLES)
	@rm -rf examples/*.o
	@echo "Clean complete"

# Create documentation
docs:
	@echo "Generating RaeenUI documentation..."
	@mkdir -p docs
	@echo "# RaeenUI Framework Documentation" > docs/README.md
	@echo "" >> docs/README.md
	@echo "RaeenUI is a modern, GPU-accelerated UI framework for RaeenOS." >> docs/README.md
	@echo "" >> docs/README.md
	@echo "## Features" >> docs/README.md
	@echo "- Declarative UI (SwiftUI/React-inspired)" >> docs/README.md
	@echo "- GPU acceleration with Vulkan" >> docs/README.md
	@echo "- Modern theming system" >> docs/README.md
	@echo "- Smooth animations and effects" >> docs/README.md
	@echo "- AI integration support" >> docs/README.md
	@echo "- Accessibility built-in" >> docs/README.md
	@echo "" >> docs/README.md
	@echo "## Quick Start" >> docs/README.md
	@echo "" >> docs/README.md
	@echo "\`\`\`c" >> docs/README.md
	@echo "#include <raeenui.h>" >> docs/README.md
	@echo "" >> docs/README.md
	@echo "// Create UI context" >> docs/README.md
	@echo "RaeenUIContext* ui = raeenui_create_context(graphics_pipeline);" >> docs/README.md
	@echo "raeenui_initialize(ui);" >> docs/README.md
	@echo "" >> docs/README.md
	@echo "// Create window" >> docs/README.md
	@echo "RaeenUIWindow* window = raeenui_create_window(ui, \"Hello World\", " >> docs/README.md
	@echo "    raeenui_rect_make(100, 100, 800, 600));" >> docs/README.md
	@echo "" >> docs/README.md
	@echo "// Create UI" >> docs/README.md
	@echo "RaeenUIView* vstack = RaeenUIVStack(16);" >> docs/README.md
	@echo "RaeenUIView* title = RaeenUIText(\"Welcome to RaeenUI!\");" >> docs/README.md
	@echo "RaeenUIView* button = RaeenUIButton(\"Click Me\", handle_click);" >> docs/README.md
	@echo "" >> docs/README.md
	@echo "raeenui_add_child_view(vstack, title);" >> docs/README.md
	@echo "raeenui_add_child_view(vstack, button);" >> docs/README.md
	@echo "raeenui_add_child_view(window->root_view, vstack);" >> docs/README.md
	@echo "" >> docs/README.md
	@echo "// Show window" >> docs/README.md
	@echo "raeenui_show_window(window);" >> docs/README.md
	@echo "\`\`\`" >> docs/README.md
	@echo "Documentation generated in docs/"

# Run tests
test: $(RAEENUI_LIB)
	@echo "Running RaeenUI tests..."
	@echo "Building test suite..."
	@$(CC) $(CFLAGS) tests/test_core.c -L. -lraeenui -o tests/test_core
	@$(CC) $(CFLAGS) tests/test_rendering.c -L. -lraeenui -o tests/test_rendering
	@$(CC) $(CFLAGS) tests/test_themes.c -L. -lraeenui -o tests/test_themes
	@echo "Running tests..."
	@./tests/test_core
	@./tests/test_rendering
	@./tests/test_themes
	@echo "All tests passed!"

# Performance benchmarks
benchmark: $(RAEENUI_LIB)
	@echo "Running RaeenUI performance benchmarks..."
	@$(CC) $(CFLAGS) -O3 benchmarks/render_benchmark.c -L. -lraeenui -o benchmarks/render_benchmark
	@$(CC) $(CFLAGS) -O3 benchmarks/layout_benchmark.c -L. -lraeenui -o benchmarks/layout_benchmark
	@./benchmarks/render_benchmark
	@./benchmarks/layout_benchmark

# Create package
package: $(RAEENUI_LIB) docs
	@echo "Creating RaeenUI package..."
	@mkdir -p package/raeenui-1.0.0
	@cp -r *.h *.c Makefile package/raeenui-1.0.0/
	@cp -r docs package/raeenui-1.0.0/
	@cp -r examples package/raeenui-1.0.0/
	@cd package && tar -czf raeenui-1.0.0.tar.gz raeenui-1.0.0
	@echo "Package created: package/raeenui-1.0.0.tar.gz"

# Development helpers
dev-setup:
	@echo "Setting up RaeenUI development environment..."
	@mkdir -p examples tests benchmarks docs
	@echo "Development environment ready"

# Format code
format:
	@echo "Formatting RaeenUI code..."
	@clang-format -i *.c *.h
	@echo "Code formatting complete"

# Static analysis
analyze:
	@echo "Running static analysis on RaeenUI..."
	@clang-static-analyzer *.c
	@echo "Static analysis complete"

# Memory check
memcheck: examples
	@echo "Running memory checks on RaeenUI examples..."
	@valgrind --leak-check=full ./examples/hello_world
	@echo "Memory check complete"

# Integration with RaeenOS build system
raeenos-integration:
	@echo "Integrating RaeenUI with RaeenOS build system..."
	@cp $(RAEENUI_LIB) ../libs/
	@cp raeenui.h ../include/ui/
	@echo "RaeenUI integrated with RaeenOS"

# Help target
help:
	@echo "RaeenUI Framework Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all           - Build RaeenUI library and examples"
	@echo "  $(RAEENUI_LIB)     - Build RaeenUI library only"
	@echo "  examples      - Build example programs"
	@echo "  install       - Install RaeenUI framework"
	@echo "  uninstall     - Uninstall RaeenUI framework"
	@echo "  clean         - Clean build artifacts"
	@echo "  docs          - Generate documentation"
	@echo "  test          - Run test suite"
	@echo "  benchmark     - Run performance benchmarks"
	@echo "  package       - Create distribution package"
	@echo "  dev-setup     - Setup development environment"
	@echo "  format        - Format source code"
	@echo "  analyze       - Run static analysis"
	@echo "  memcheck      - Run memory leak checks"
	@echo "  raeenos-integration - Integrate with RaeenOS build"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Variables:"
	@echo "  DEBUG=1       - Enable debug build"
	@echo "  ARCH=x86_64   - Target architecture"
	@echo "  DESTDIR=path  - Installation prefix"

# Dependencies
raeenui_core.o: raeenui.h ../gpu/graphics_pipeline.h ../memory.h ../string.h
raeenui_render.o: raeenui.h ../gpu/graphics_pipeline.h ../memory.h ../string.h
raeenui_theme.o: raeenui.h ../memory.h ../string.h ../time.h
raeenui_declarative.o: raeenui.h ../memory.h ../string.h

.PHONY: all examples install uninstall clean docs test benchmark package dev-setup format analyze memcheck raeenos-integration help
