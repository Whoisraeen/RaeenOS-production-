#!/bin/bash
set -euo pipefail

# RaeenOS Development Environment Setup Script
# Installs all dependencies and tools needed for RaeenOS development

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if running as root
check_root() {
    if [[ $EUID -eq 0 ]]; then
        log_error "This script should not be run as root"
        exit 1
    fi
}

# Detect operating system
detect_os() {
    if [[ -f /etc/os-release ]]; then
        . /etc/os-release
        OS=$ID
        OS_VERSION=$VERSION_ID
    elif [[ -f /etc/redhat-release ]]; then
        OS="centos"
    elif [[ -f /etc/debian_version ]]; then
        OS="debian"
    else
        log_error "Unsupported operating system"
        exit 1
    fi
    
    log_info "Detected OS: $OS $OS_VERSION"
}

# Install system dependencies
install_system_deps() {
    log_info "Installing system dependencies..."
    
    case $OS in
        ubuntu|debian)
            sudo apt-get update
            sudo apt-get install -y \
                build-essential \
                nasm \
                qemu-system-x86 \
                qemu-system-arm \
                qemu-system-misc \
                qemu-utils \
                xorriso \
                grub-pc-bin \
                grub-efi-amd64-bin \
                mtools \
                dosfstools \
                gcc-multilib \
                clang \
                clang-tools \
                clang-tidy \
                clang-format \
                cppcheck \
                splint \
                valgrind \
                gdb \
                lcov \
                doxygen \
                graphviz \
                git \
                curl \
                wget \
                python3 \
                python3-pip \
                python3-venv \
                nodejs \
                npm \
                docker.io \
                docker-compose
            ;;
        fedora|centos|rhel)
            sudo dnf install -y \
                gcc \
                gcc-c++ \
                nasm \
                qemu-system-x86 \
                qemu-system-arm \
                grub2-tools \
                xorriso \
                clang \
                clang-tools-extra \
                cppcheck \
                valgrind \
                gdb \
                lcov \
                doxygen \
                graphviz \
                git \
                curl \
                wget \
                python3 \
                python3-pip \
                nodejs \
                npm \
                docker \
                docker-compose
            ;;
        arch)
            sudo pacman -S --needed \
                base-devel \
                nasm \
                qemu \
                grub \
                libisoburn \
                clang \
                cppcheck \
                valgrind \
                gdb \
                lcov \
                doxygen \
                graphviz \
                git \
                curl \
                wget \
                python \
                python-pip \
                nodejs \
                npm \
                docker \
                docker-compose
            ;;
        *)
            log_error "Package installation not supported for $OS"
            log_info "Please install the following packages manually:"
            log_info "- build-essential, nasm, qemu, grub tools"
            log_info "- clang, cppcheck, valgrind, gdb, lcov"
            log_info "- python3, nodejs, docker"
            ;;
    esac
    
    log_success "System dependencies installed"
}

# Install cross-compilation toolchains
install_cross_compilers() {
    log_info "Installing cross-compilation toolchains..."
    
    TOOLCHAIN_DIR="$HOME/.local/raeenos-toolchains"
    mkdir -p "$TOOLCHAIN_DIR"
    
    # Add toolchain directory to PATH
    if ! grep -q "$TOOLCHAIN_DIR/bin" ~/.bashrc; then
        echo "export PATH=\"$TOOLCHAIN_DIR/bin:\$PATH\"" >> ~/.bashrc
    fi
    
    # Install cross-compilers for different architectures
    case $OS in
        ubuntu|debian)
            sudo apt-get install -y \
                gcc-aarch64-linux-gnu \
                gcc-riscv64-linux-gnu \
                gcc-arm-linux-gnueabihf
            ;;
        fedora|centos|rhel)
            sudo dnf install -y \
                gcc-aarch64-linux-gnu \
                gcc-riscv64-linux-gnu \
                gcc-arm-linux-gnu
            ;;
        *)
            log_warning "Cross-compilers not available via package manager"
            log_info "Consider building cross-compilers manually or using crosstool-ng"
            ;;
    esac
    
    log_success "Cross-compilation toolchains installed"
}

# Install Python dependencies
install_python_deps() {
    log_info "Installing Python dependencies..."
    
    # Create virtual environment for RaeenOS development
    VENV_DIR="$PROJECT_ROOT/.venv"
    if [[ ! -d "$VENV_DIR" ]]; then
        python3 -m venv "$VENV_DIR"
    fi
    
    # Activate virtual environment
    source "$VENV_DIR/bin/activate"
    
    # Upgrade pip
    pip install --upgrade pip
    
    # Install testing and development dependencies
    pip install \
        pytest \
        pytest-cov \
        pytest-html \
        pytest-xdist \
        coverage \
        black \
        flake8 \
        mypy \
        pyyaml \
        requests \
        jinja2 \
        click \
        rich \
        tabulate
    
    # Install monitoring dependencies
    pip install \
        psutil \
        docker \
        kubernetes \
        prometheus-client \
        grafana-api
    
    log_success "Python dependencies installed in virtual environment"
}

# Install Node.js dependencies
install_nodejs_deps() {
    log_info "Installing Node.js dependencies..."
    
    # Install global tools
    sudo npm install -g \
        eslint \
        prettier \
        typescript \
        @typescript-eslint/parser \
        @typescript-eslint/eslint-plugin
    
    log_success "Node.js dependencies installed"
}

# Setup Docker for development
setup_docker() {
    log_info "Setting up Docker for development..."
    
    # Add user to docker group
    if ! groups $USER | grep -q docker; then
        sudo usermod -aG docker $USER
        log_warning "Added $USER to docker group. Please log out and back in for changes to take effect"
    fi
    
    # Enable and start Docker service
    if command -v systemctl >/dev/null 2>&1; then
        sudo systemctl enable docker
        sudo systemctl start docker
    fi
    
    # Create development Docker network
    if ! docker network ls | grep -q raeenos-dev; then
        docker network create raeenos-dev
        log_success "Created Docker network: raeenos-dev"
    fi
    
    log_success "Docker setup completed"
}

# Setup git hooks and configuration
setup_git() {
    log_info "Setting up Git configuration and hooks..."
    
    cd "$PROJECT_ROOT"
    
    # Set up pre-commit hooks
    HOOKS_DIR=".git/hooks"
    if [[ -d .git ]]; then
        # Pre-commit hook for code formatting and linting
        cat > "$HOOKS_DIR/pre-commit" << 'EOF'
#!/bin/bash
# RaeenOS pre-commit hook

set -e

echo "Running pre-commit checks..."

# Check for large files
git diff --cached --name-only | xargs -I {} find {} -size +10M 2>/dev/null | head -1 | grep -q . && {
    echo "Error: Large files detected. Please use Git LFS for files over 10MB"
    exit 1
}

# Run code formatting check
echo "Checking code formatting..."
if command -v clang-format >/dev/null 2>&1; then
    git diff --cached --name-only '*.c' '*.h' '*.cpp' '*.hpp' | while read file; do
        if [[ -f "$file" ]]; then
            clang-format --dry-run --Werror "$file" || {
                echo "Error: $file is not properly formatted. Run 'make format' to fix."
                exit 1
            }
        fi
    done
fi

# Run static analysis on changed files
echo "Running static analysis..."
if command -v cppcheck >/dev/null 2>&1; then
    git diff --cached --name-only '*.c' '*.cpp' | while read file; do
        if [[ -f "$file" ]]; then
            cppcheck --error-exitcode=1 --quiet "$file" || {
                echo "Error: Static analysis failed for $file"
                exit 1
            }
        fi
    done
fi

# Check commit message format
echo "Checking commit message format..."
commit_msg_file="$1"
if [[ -n "$commit_msg_file" ]]; then
    commit_msg=$(head -n1 "$commit_msg_file")
    if ! echo "$commit_msg" | grep -qE '^(feat|fix|docs|style|refactor|test|chore)(\(.+\))?: .{1,50}$'; then
        echo "Error: Commit message must follow conventional commit format"
        echo "Format: type(scope): description"
        echo "Types: feat, fix, docs, style, refactor, test, chore"
        exit 1
    fi
fi

echo "Pre-commit checks passed!"
EOF
        
        chmod +x "$HOOKS_DIR/pre-commit"
        
        # Pre-push hook for testing
        cat > "$HOOKS_DIR/pre-push" << 'EOF'
#!/bin/bash
# RaeenOS pre-push hook

set -e

echo "Running pre-push checks..."

# Run quick tests before push
if [[ -f "Makefile" ]]; then
    echo "Running quick build test..."
    make clean >/dev/null 2>&1 || true
    make kernel TARGET=x86-64 CC=gcc BUILD_TYPE=debug || {
        echo "Error: Build failed. Please fix build issues before pushing."
        exit 1
    }
fi

echo "Pre-push checks passed!"
EOF
        
        chmod +x "$HOOKS_DIR/pre-push"
        
        log_success "Git hooks configured"
    else
        log_warning "Not a git repository, skipping git hooks setup"
    fi
}

# Create development configuration files
create_dev_configs() {
    log_info "Creating development configuration files..."
    
    cd "$PROJECT_ROOT"
    
    # Create .editorconfig
    cat > .editorconfig << 'EOF'
root = true

[*]
charset = utf-8
end_of_line = lf
insert_final_newline = true
trim_trailing_whitespace = true
indent_style = space
indent_size = 4

[*.{c,h,cpp,hpp}]
indent_size = 4

[*.{js,ts,json}]
indent_size = 2

[*.{py}]
indent_size = 4

[*.{yml,yaml}]
indent_size = 2

[*.md]
trim_trailing_whitespace = false

[Makefile]
indent_style = tab
EOF
    
    # Create .clang-format configuration
    cat > .clang-format << 'EOF'
BasedOnStyle: LLVM
IndentWidth: 4
TabWidth: 4
UseTab: Never
ColumnLimit: 100
BreakBeforeBraces: Linux
AlignAfterOpenBracket: Align
AlignConsecutiveAssignments: true
AlignConsecutiveDeclarations: true
AlignTrailingComments: true
AllowShortBlocksOnASingleLine: false
AllowShortCaseLabelsOnASingleLine: false
AllowShortFunctionsOnASingleLine: None
AllowShortIfStatementsOnASingleLine: false
AllowShortLoopsOnASingleLine: false
BreakStringLiterals: true
SortIncludes: true
SpaceAfterCStyleCast: true
SpaceBeforeParens: ControlStatements
SpacesInContainerLiterals: false
SpacesInCStyleCastParentheses: false
SpacesInParentheses: false
SpacesInSquareBrackets: false
EOF
    
    # Create VS Code settings
    mkdir -p .vscode
    cat > .vscode/settings.json << 'EOF'
{
    "files.associations": {
        "*.h": "c",
        "*.c": "c"
    },
    "C_Cpp.default.configurationProvider": "ms-vscode.makefile-tools",
    "C_Cpp.default.cStandard": "c11",
    "C_Cpp.default.cppStandard": "c++17",
    "C_Cpp.default.includePath": [
        "${workspaceFolder}/kernel/include",
        "${workspaceFolder}/kernel",
        "${workspaceFolder}/drivers",
        "${workspaceFolder}/userland/include"
    ],
    "C_Cpp.default.defines": [
        "__RAEENOS__=1",
        "KERNEL_MODE=1"
    ],
    "files.trimTrailingWhitespace": true,
    "files.insertFinalNewline": true,
    "editor.formatOnSave": true,
    "editor.rulers": [100],
    "python.defaultInterpreterPath": "./.venv/bin/python",
    "python.terminal.activateEnvironment": true
}
EOF
    
    # Create launch configuration for debugging
    cat > .vscode/launch.json << 'EOF'
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug RaeenOS in QEMU",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/x86-64-gcc-debug/kernel.elf",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "miDebuggerPath": "gdb",
            "miDebuggerServerAddress": "localhost:1234",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                },
                {
                    "description": "Set architecture to i386:x86-64",
                    "text": "set architecture i386:x86-64",
                    "ignoreFailures": false
                }
            ],
            "preLaunchTask": "Start QEMU with GDB"
        }
    ]
}
EOF
    
    # Create tasks configuration
    cat > .vscode/tasks.json << 'EOF'
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build RaeenOS",
            "type": "shell",
            "command": "make",
            "args": ["all"],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            },
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "Clean Build",
            "type": "shell",
            "command": "make",
            "args": ["clean"],
            "group": "build"
        },
        {
            "label": "Run Tests",
            "type": "shell",
            "command": "make",
            "args": ["test"],
            "group": "test"
        },
        {
            "label": "Start QEMU with GDB",
            "type": "shell",
            "command": "make",
            "args": ["qemu-debug"],
            "isBackground": true,
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "new"
            }
        }
    ]
}
EOF
    
    log_success "Development configuration files created"
}

# Setup monitoring tools
setup_monitoring() {
    log_info "Setting up monitoring tools..."
    
    # Make monitoring scripts executable
    MONITORING_DIR="$PROJECT_ROOT/tools/monitoring"
    if [[ -d "$MONITORING_DIR" ]]; then
        chmod +x "$MONITORING_DIR"/*.py
        
        # Create monitoring configuration
        cat > "$MONITORING_DIR/monitor-config.yaml" << 'EOF'
github:
  token: "${GITHUB_TOKEN}"
  repository: "${GITHUB_REPOSITORY:-raeenos/raeenos}"

monitoring:
  interval_seconds: 900  # 15 minutes
  retention_hours: 168   # 7 days

alerts:
  enabled: true
  slack_webhook_url: "${SLACK_WEBHOOK_URL}"

thresholds:
  build_queue_warning: 10
  build_queue_critical: 20
  success_rate_warning: 80.0
  success_rate_critical: 70.0
  storage_usage_warning: 80.0
  storage_usage_critical: 90.0
EOF
        
        log_success "Monitoring tools configured"
    fi
}

# Create helpful scripts
create_scripts() {
    log_info "Creating helpful development scripts..."
    
    SCRIPTS_DIR="$PROJECT_ROOT/tools/scripts"
    mkdir -p "$SCRIPTS_DIR"
    
    # Quick build script
    cat > "$SCRIPTS_DIR/quick-build.sh" << 'EOF'
#!/bin/bash
# Quick build script for RaeenOS development

set -e

TARGET=${1:-x86-64}
BUILD_TYPE=${2:-debug}

echo "Building RaeenOS for $TARGET ($BUILD_TYPE)..."

make clean
make all TARGET="$TARGET" BUILD_TYPE="$BUILD_TYPE" -j$(nproc)

echo "Build completed successfully!"
echo "Kernel: build/$TARGET-gcc-$BUILD_TYPE/kernel.elf"
echo "OS Image: build/$TARGET-gcc-$BUILD_TYPE/raeenos-$TARGET-*.bin"
EOF
    
    # Test runner script
    cat > "$SCRIPTS_DIR/run-tests.sh" << 'EOF'
#!/bin/bash
# Test runner script for RaeenOS

set -e

TEST_TYPE=${1:-all}
VERBOSE=${2:-false}

echo "Running RaeenOS tests ($TEST_TYPE)..."

cd "$(dirname "$0")/../.."

# Activate Python virtual environment
if [[ -f .venv/bin/activate ]]; then
    source .venv/bin/activate
fi

case $TEST_TYPE in
    unit)
        make test ENABLE_TESTS=1
        ;;
    integration)
        echo "Running integration tests..."
        # Integration tests would go here
        ;;
    system)
        echo "Running system tests in QEMU..."
        make qemu-test
        ;;
    all)
        echo "Running all tests..."
        make test ENABLE_TESTS=1
        ;;
    *)
        echo "Unknown test type: $TEST_TYPE"
        echo "Usage: $0 [unit|integration|system|all] [verbose]"
        exit 1
        ;;
esac

echo "All tests completed!"
EOF
    
    # Development environment activation script
    cat > "$SCRIPTS_DIR/activate-dev.sh" << 'EOF'
#!/bin/bash
# Activate RaeenOS development environment

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

echo "Activating RaeenOS development environment..."

# Activate Python virtual environment
if [[ -f "$PROJECT_ROOT/.venv/bin/activate" ]]; then
    source "$PROJECT_ROOT/.venv/bin/activate"
    echo "Python virtual environment activated"
fi

# Set environment variables
export RAEENOS_ROOT="$PROJECT_ROOT"
export PATH="$HOME/.local/raeenos-toolchains/bin:$PATH"

# Set build defaults
export TARGET=${TARGET:-x86-64}
export BUILD_TYPE=${BUILD_TYPE:-debug}

echo "Development environment ready!"
echo "Project root: $PROJECT_ROOT"
echo "Default target: $TARGET"
echo "Default build type: $BUILD_TYPE"

# Start a new shell with the environment
bash --rcfile <(echo "PS1='[RaeenOS-dev] \u@\h:\w\$ '")
EOF
    
    # Make scripts executable
    chmod +x "$SCRIPTS_DIR"/*.sh
    
    log_success "Development scripts created"
}

# Print setup completion message
print_completion() {
    log_success "RaeenOS development environment setup completed!"
    echo
    echo "Next steps:"
    echo "1. Log out and back in (or run 'newgrp docker') to apply group changes"
    echo "2. Run 'source tools/scripts/activate-dev.sh' to activate the development environment"
    echo "3. Try building RaeenOS with 'make all' or 'tools/scripts/quick-build.sh'"
    echo "4. Run tests with 'tools/scripts/run-tests.sh'"
    echo
    echo "Available make targets:"
    echo "  make all          - Build complete OS"
    echo "  make kernel       - Build kernel only"
    echo "  make test         - Run test suite"
    echo "  make qemu-test    - Test in QEMU"
    echo "  make clean        - Clean build files"
    echo "  make format       - Format source code"
    echo "  make check        - Run static analysis"
    echo
    echo "For more information, see docs/CI_CD_INFRASTRUCTURE_GUIDE.md"
}

# Main setup function
main() {
    echo "=================================================="
    echo "  RaeenOS Development Environment Setup"
    echo "=================================================="
    echo
    
    check_root
    detect_os
    
    log_info "Starting development environment setup..."
    
    install_system_deps
    install_cross_compilers
    install_python_deps
    install_nodejs_deps
    setup_docker
    setup_git
    create_dev_configs
    setup_monitoring
    create_scripts
    
    print_completion
}

# Check if script is being sourced or executed
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi