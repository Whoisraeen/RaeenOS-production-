# RaeCompat Makefile
# Windows Compatibility Layer for RaeenOS

CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -std=c11 -O2 -fPIC
CXXFLAGS = -Wall -Wextra -std=c++17 -O2 -fPIC
LDFLAGS = -shared
LIBS = -ljson-c -lpthread -lm -ldl

# Directories
SRCDIR = .
BUILDDIR = ../build_raecompat
INSTALLDIR = /usr/lib/raecompat
BINDIR = /usr/bin
DOCDIR = /usr/share/doc/raecompat

# Source files
CORE_SOURCES = raecompat_core.c
MANAGER_SOURCES = raeen_game_manager.c
PROTONCTL_SOURCES = protonctl.c

# Object files
CORE_OBJECTS = $(BUILDDIR)/raecompat_core.o
MANAGER_OBJECTS = $(BUILDDIR)/raeen_game_manager.o
PROTONCTL_OBJECTS = $(BUILDDIR)/protonctl.o

# Targets
CORE_LIB = $(BUILDDIR)/libraecompat.so
MANAGER_BIN = $(BUILDDIR)/raeen-game-manager
PROTONCTL_BIN = $(BUILDDIR)/protonctl

# UI library path (adjust as needed)
RAEENUI_LIBS = -L../libs/raeenui -lraeenui -lX11 -lGL -lGLU

.PHONY: all clean install uninstall setup

all: $(CORE_LIB) $(MANAGER_BIN) $(PROTONCTL_BIN)

# Create build directory
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Build core library
$(CORE_LIB): $(CORE_OBJECTS) | $(BUILDDIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BUILDDIR)/raecompat_core.o: raecompat_core.c raecompat_core.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -I../kernel -I../userland/include -c $< -o $@

# Build game manager
$(MANAGER_BIN): $(MANAGER_OBJECTS) $(CORE_LIB) | $(BUILDDIR)
	$(CXX) -o $@ $< -L$(BUILDDIR) -lraecompat $(RAEENUI_LIBS) $(LIBS)

$(BUILDDIR)/raeen_game_manager.o: raeen_game_manager.c raeen_game_manager.h raecompat_core.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -I../libs/raeenui -I../kernel -c $< -o $@

# Build ProtonCTL
$(PROTONCTL_BIN): $(PROTONCTL_OBJECTS) $(CORE_LIB) | $(BUILDDIR)
	$(CC) -o $@ $< -L$(BUILDDIR) -lraecompat $(LIBS)

$(BUILDDIR)/protonctl.o: protonctl.c raecompat_core.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Setup system (run setup script)
setup:
	@echo "Setting up RaeCompat Windows Compatibility Layer..."
	chmod +x setup_raecompat.sh
	sudo ./setup_raecompat.sh

# Install binaries and libraries
install: all
	@echo "Installing RaeCompat components..."
	
	# Create directories
	sudo mkdir -p $(INSTALLDIR)
	sudo mkdir -p $(BINDIR)
	sudo mkdir -p $(DOCDIR)
	sudo mkdir -p /usr/share/raeenos/icons
	
	# Install core library
	sudo cp $(CORE_LIB) $(INSTALLDIR)/
	sudo ldconfig
	
	# Install binaries
	sudo cp $(MANAGER_BIN) $(BINDIR)/
	sudo cp $(PROTONCTL_BIN) $(BINDIR)/
	sudo chmod +x $(BINDIR)/raeen-game-manager
	sudo chmod +x $(BINDIR)/protonctl
	
	# Install headers for development
	sudo mkdir -p /usr/include/raecompat
	sudo cp raecompat_core.h /usr/include/raecompat/
	sudo cp raeen_game_manager.h /usr/include/raecompat/
	
	# Install documentation
	sudo cp ../docs/compat/raecompat.md $(DOCDIR)/
	
	# Install desktop entry
	sudo mkdir -p /usr/share/applications
	sudo cp raeen-game-manager.desktop /usr/share/applications/ || true
	
	# Install systemd service
	sudo cp raecompat.service /etc/systemd/system/ || true
	sudo systemctl daemon-reload || true
	
	@echo "RaeCompat installation completed!"
	@echo "Run 'raeen-game-manager' to launch the GUI"
	@echo "Run 'protonctl --help' for command-line usage"

# Uninstall
uninstall:
	@echo "Uninstalling RaeCompat..."
	
	# Stop and disable service
	sudo systemctl stop raecompat || true
	sudo systemctl disable raecompat || true
	
	# Remove binaries
	sudo rm -f $(BINDIR)/raeen-game-manager
	sudo rm -f $(BINDIR)/protonctl
	
	# Remove libraries
	sudo rm -f $(INSTALLDIR)/libraecompat.so
	
	# Remove headers
	sudo rm -rf /usr/include/raecompat
	
	# Remove documentation
	sudo rm -rf $(DOCDIR)
	
	# Remove desktop entry
	sudo rm -f /usr/share/applications/raeen-game-manager.desktop
	
	# Remove systemd service
	sudo rm -f /etc/systemd/system/raecompat.service
	sudo systemctl daemon-reload || true
	
	# Update library cache
	sudo ldconfig
	
	@echo "RaeCompat uninstalled successfully"

# Clean build files
clean:
	rm -rf $(BUILDDIR)

# Development targets
debug: CFLAGS += -g -DDEBUG
debug: CXXFLAGS += -g -DDEBUG
debug: all

# Test target
test: all
	@echo "Running RaeCompat tests..."
	$(PROTONCTL_BIN) diagnostics
	@echo "Basic functionality test completed"

# Package target for distribution
package: all
	@echo "Creating RaeCompat package..."
	mkdir -p raecompat-package/bin
	mkdir -p raecompat-package/lib
	mkdir -p raecompat-package/doc
	mkdir -p raecompat-package/scripts
	
	cp $(MANAGER_BIN) raecompat-package/bin/
	cp $(PROTONCTL_BIN) raecompat-package/bin/
	cp $(CORE_LIB) raecompat-package/lib/
	cp ../docs/compat/raecompat.md raecompat-package/doc/
	cp setup_raecompat.sh raecompat-package/scripts/
	
	tar -czf raecompat-$(shell date +%Y%m%d).tar.gz raecompat-package/
	rm -rf raecompat-package/
	
	@echo "Package created: raecompat-$(shell date +%Y%m%d).tar.gz"

# Help target
help:
	@echo "RaeCompat Build System"
	@echo "======================"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build all components"
	@echo "  setup      - Run system setup script"
	@echo "  install    - Install to system directories"
	@echo "  uninstall  - Remove from system"
	@echo "  clean      - Clean build files"
	@echo "  debug      - Build with debug symbols"
	@echo "  test       - Run basic functionality tests"
	@echo "  package    - Create distribution package"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make setup     # First-time setup"
	@echo "  make all       # Build everything"
	@echo "  make install   # Install to system"
