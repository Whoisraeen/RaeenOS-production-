#!/bin/bash
# RaeenOS Repository Structure Migration Script
# Migrates existing code to the new 42-agent repository architecture
# Version: 1.0

set -e

# Color output functions
red() { echo -e "\033[31m$1\033[0m"; }
green() { echo -e "\033[32m$1\033[0m"; }
yellow() { echo -e "\033[33m$1\033[0m"; }
blue() { echo -e "\033[34m$1\033[0m"; }
cyan() { echo -e "\033[36m$1\033[0m"; }

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BACKUP_DIR="$REPO_ROOT/migration-backup-$(date +%Y%m%d-%H%M%S)"
DRY_RUN=false
VERBOSE=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --help)
            echo "RaeenOS Repository Structure Migration Script"
            echo ""
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --dry-run    Show what would be done without making changes"
            echo "  --verbose    Show detailed output"
            echo "  --help       Show this help message"
            echo ""
            echo "This script migrates the existing RaeenOS repository structure"
            echo "to the new 42-agent coordinated development architecture."
            exit 0
            ;;
        *)
            red "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Logging function
log() {
    if [[ "$VERBOSE" == "true" ]]; then
        echo "$1"
    fi
}

# Execute command (respecting dry-run mode)
execute() {
    local cmd="$1"
    local desc="$2"
    
    if [[ "$DRY_RUN" == "true" ]]; then
        yellow "[DRY RUN] $desc"
        yellow "  Command: $cmd"
    else
        log "$desc"
        eval "$cmd"
    fi
}

# Create directory structure
create_directory_structure() {
    blue "📁 Creating new directory structure..."
    
    local dirs=(
        # Core OS components
        "core/kernel/arch/x86_64"
        "core/kernel/arch/arm64"
        "core/kernel/mm"
        "core/kernel/sched"
        "core/kernel/sync"
        "core/kernel/syscall"
        "core/kernel/interrupt"
        "core/kernel/time"
        "core/kernel/debug"
        "core/kernel/init"
        "core/kernel/include"
        
        "core/hal/include"
        "core/hal/cpu"
        "core/hal/memory"
        "core/hal/interrupt"
        "core/hal/io"
        "core/hal/platform/pc"
        "core/hal/platform/embedded"
        "core/hal/tests"
        
        "core/drivers/framework"
        "core/drivers/bus/pci"
        "core/drivers/bus/usb"
        "core/drivers/bus/i2c"
        "core/drivers/bus/spi"
        "core/drivers/storage/ata"
        "core/drivers/storage/nvme"
        "core/drivers/storage/scsi"
        "core/drivers/storage/usb_storage"
        "core/drivers/network/ethernet"
        "core/drivers/network/wireless"
        "core/drivers/network/bluetooth"
        "core/drivers/graphics/intel"
        "core/drivers/graphics/amd"
        "core/drivers/graphics/nvidia"
        "core/drivers/graphics/generic"
        "core/drivers/audio/hda"
        "core/drivers/audio/usb_audio"
        "core/drivers/audio/bluetooth_audio"
        "core/drivers/input/keyboard"
        "core/drivers/input/mouse"
        "core/drivers/input/touchpad"
        "core/drivers/input/touchscreen"
        "core/drivers/power/acpi"
        "core/drivers/power/battery"
        "core/drivers/power/thermal"
        "core/drivers/misc/serial"
        "core/drivers/misc/parallel"
        "core/drivers/misc/sensors"
        
        "core/security/sandboxing"
        "core/security/access_control"
        "core/security/crypto"
        "core/security/audit"
        
        # System services
        "system/services/init/systemd_compat"
        "system/services/package"
        "system/services/authentication"
        "system/services/logging"
        "system/services/backup"
        "system/services/telemetry"
        "system/services/notification"
        
        "system/ipc/shared_memory"
        "system/ipc/message_queues"
        "system/ipc/pipes"
        "system/ipc/rpc"
        
        "system/filesystem/raeenfs"
        "system/filesystem/fat32"
        "system/filesystem/ext4"
        "system/filesystem/ntfs"
        "system/filesystem/fuse"
        "system/filesystem/cloud"
        
        "system/network/stack"
        "system/network/security"
        "system/network/wireless"
        "system/network/protocols"
        "system/network/management"
        "system/network/tools"
        
        # User-space components
        "userspace/applications/raeen_studio/notes"
        "userspace/applications/raeen_studio/docs"
        "userspace/applications/raeen_studio/canvas"
        "userspace/applications/raeen_studio/journal"
        "userspace/applications/raeen_studio/plan"
        "userspace/applications/raeen_studio/shared"
        "userspace/applications/app_store/frontend"
        "userspace/applications/app_store/backend"
        "userspace/applications/app_store/payment"
        "userspace/applications/app_store/security"
        "userspace/applications/terminal"
        "userspace/applications/calculator"
        "userspace/applications/calendar"
        "userspace/applications/media_player"
        "userspace/applications/text_editor"
        "userspace/applications/image_viewer"
        "userspace/applications/web_browser/engine"
        "userspace/applications/web_browser/ui"
        "userspace/applications/web_browser/extensions"
        
        "userspace/libraries/libc"
        "userspace/libraries/libm"
        "userspace/libraries/libpthread"
        "userspace/libraries/libssl"
        
        "userspace/desktop/compositor/wayland_compat"
        "userspace/desktop/compositor/x11_compat"
        "userspace/desktop/window_manager"
        "userspace/desktop/shell"
        "userspace/desktop/file_manager"
        "userspace/desktop/themes"
        "userspace/desktop/accessibility"
        "userspace/desktop/settings"
        
        "userspace/shell/raeshell"
        "userspace/shell/builtins"
        "userspace/shell/scripting"
        "userspace/shell/ai_integration"
        
        # Platform-specific code
        "platform/x86_64/boot"
        "platform/x86_64/kernel"
        "platform/x86_64/drivers"
        "platform/arm64/boot"
        "platform/arm64/kernel"
        "platform/arm64/drivers"
        "platform/boot/grub"
        "platform/boot/systemd-boot"
        "platform/firmware/uefi"
        "platform/firmware/bios"
        
        # AI integration
        "ai/core/framework"
        "ai/core/models"
        "ai/core/backends/local"
        "ai/core/backends/openai"
        "ai/core/backends/anthropic"
        "ai/core/backends/ollama"
        "ai/core/security"
        "ai/core/interfaces"
        
        "ai/services/voice_assistant"
        "ai/services/text_processing"
        "ai/services/computer_vision"
        "ai/services/automation"
        
        "ai/integration/filesystem"
        "ai/integration/shell"
        "ai/integration/desktop"
        "ai/integration/applications"
        
        # Virtualization
        "virtualization/hypervisor/core"
        "virtualization/hypervisor/memory"
        "virtualization/hypervisor/cpu"
        "virtualization/hypervisor/io"
        
        "virtualization/containers/runtime"
        "virtualization/containers/registry"
        "virtualization/containers/networking"
        
        "virtualization/security/isolation"
        "virtualization/security/attestation"
        
        # Compatibility layers
        "compatibility/windows/pe_loader"
        "compatibility/windows/win32_api"
        "compatibility/windows/registry"
        "compatibility/windows/filesystem"
        
        "compatibility/macos/macho_loader"
        "compatibility/macos/cocoa_api"
        "compatibility/macos/bundles"
        
        "compatibility/linux/elf_loader"
        "compatibility/linux/syscall_compat"
        "compatibility/linux/filesystem"
        
        "compatibility/android/art_runtime"
        "compatibility/android/framework "
        "compatibility/android/security"
        
        # Enterprise features
        "enterprise/deployment/active_directory"
        "enterprise/deployment/fleet_management"
        "enterprise/deployment/policies"
        
        "enterprise/management/monitoring"
        "enterprise/management/updates"
        "enterprise/management/configuration"
        
        "enterprise/compliance/auditing"
        "enterprise/compliance/reporting"
        "enterprise/compliance/certification"
        
        "enterprise/integration/ldap"
        "enterprise/integration/sso"
        "enterprise/integration/third_party"
        
        # Development tools
        "development/sdk/c"
        "development/sdk/cpp"
        "development/sdk/python"
        "development/sdk/rust"
        
        "development/tools/raedev"
        "development/tools/compiler"
        "development/tools/build_system"
        
        "development/debugger/kernel"
        "development/debugger/userspace"
        "development/debugger/remote"
        
        "development/profiler/cpu"
        "development/profiler/memory"
        "development/profiler/io"
        
        # Testing infrastructure
        "testing/unit/core"
        "testing/unit/system"
        "testing/unit/userspace"
        "testing/unit/ai"
        "testing/unit/virtualization"
        
        "testing/integration/kernel_hal"
        "testing/integration/driver_kernel"
        "testing/integration/service_kernel"
        "testing/integration/full_stack"
        
        "testing/security/penetration"
        "testing/security/fuzzing"
        "testing/security/static_analysis"
        "testing/security/dynamic_analysis"
        
        "testing/performance/benchmarks"
        "testing/performance/stress"
        "testing/performance/load"
        "testing/performance/profiling"
        
        "testing/compatibility/hardware"
        "testing/compatibility/software"
        "testing/compatibility/standards"
        
        "testing/system/boot"
        "testing/system/stability"
        "testing/system/recovery"
        "testing/system/upgrade"
        
        "testing/automation/ci_cd"
        "testing/automation/regression"
        "testing/automation/nightly"
        "testing/automation/reporting"
        
        "testing/frameworks/unity"
        "testing/frameworks/googletest"
        "testing/frameworks/pytest"
        "testing/frameworks/custom"
        
        # Tools and utilities
        "tools/build/cmake"
        "tools/build/autotools"
        "tools/build/ninja"
        
        "tools/quality/formatting"
        "tools/quality/linting"
        "tools/quality/security"
        "tools/quality/coverage"
        
        "tools/automation/ci_cd"
        "tools/automation/deployment"
        "tools/automation/testing"
        
        "tools/monitoring/performance"
        "tools/monitoring/quality"
        "tools/monitoring/security"
        
        # Documentation
        "documentation/architecture"
        "documentation/api/kernel"
        "documentation/api/system"
        "documentation/api/userspace"
        "documentation/api/ai"
        
        "documentation/guides/user"
        "documentation/guides/developer"
        "documentation/guides/administrator"
        
        "documentation/specifications"
        "documentation/tutorials/driver_development"
        "documentation/tutorials/application_development"
        "documentation/tutorials/ai_integration"
        
        "documentation/reference"
        "documentation/changelog/v1.0"
        "documentation/changelog/migration"
        
        # Deployment
        "deployment/installer/gui"
        "deployment/installer/cli"
        "deployment/installer/automated"
        
        "deployment/packages/deb"
        "deployment/packages/rpm"
        "deployment/packages/arch"
        "deployment/packages/flatpak"
        
        "deployment/images/iso"
        "deployment/images/usb"
        "deployment/images/netboot"
        
        "deployment/oem/dell"
        "deployment/oem/hp"
        "deployment/oem/lenovo"
        
        # External dependencies
        "external/third-party"
        "external/opensource"
        "external/licenses"
    )
    
    for dir in "${dirs[@]}"; do
        execute "mkdir -p '$REPO_ROOT/$dir'" "Creating directory: $dir"
    done
}

# Backup existing structure
backup_existing_structure() {
    blue "💾 Creating backup of existing structure..."
    
    if [[ "$DRY_RUN" == "false" ]]; then
        mkdir -p "$BACKUP_DIR"
        
        # Backup existing directories that will be moved
        for dir in kernel drivers userland; do
            if [[ -d "$REPO_ROOT/$dir" ]]; then
                cp -r "$REPO_ROOT/$dir" "$BACKUP_DIR/"
                log "Backed up $dir to $BACKUP_DIR"
            fi
        done
        
        green "Backup created at: $BACKUP_DIR"
    else
        yellow "[DRY RUN] Would create backup at: $BACKUP_DIR"
    fi
}

# Migrate existing files
migrate_existing_files() {
    blue "🔄 Migrating existing files to new structure..."
    
    # Migrate kernel files
    if [[ -d "$REPO_ROOT/kernel" ]]; then
        cyan "Migrating kernel files..."
        
        # Core kernel files
        for file in "$REPO_ROOT/kernel"/*.c "$REPO_ROOT/kernel"/*.h; do
            if [[ -f "$file" ]]; then
                filename=$(basename "$file")
                case "$filename" in
                    memory.* | pmm.* | vmm.* | heap.*)
                        execute "mv '$file' '$REPO_ROOT/core/kernel/mm/'" "Moving $filename to mm/"
                        ;;
                    process.* | scheduler.* | thread.*)
                        execute "mv '$file' '$REPO_ROOT/core/kernel/sched/'" "Moving $filename to sched/"
                        ;;
                    syscall.*)
                        execute "mv '$file' '$REPO_ROOT/core/kernel/syscall/'" "Moving $filename to syscall/"
                        ;;
                    idt.* | interrupt.*)
                        execute "mv '$file' '$REPO_ROOT/core/kernel/interrupt/'" "Moving $filename to interrupt/"
                        ;;
                    timer.*)
                        execute "mv '$file' '$REPO_ROOT/core/kernel/time/'" "Moving $filename to time/"
                        ;;
                    kernel.* | init.* | main.*)
                        execute "mv '$file' '$REPO_ROOT/core/kernel/init/'" "Moving $filename to init/"
                        ;;
                    *)
                        # Default location for other kernel files
                        execute "mv '$file' '$REPO_ROOT/core/kernel/'" "Moving $filename to kernel/"
                        ;;
                esac
            fi
        done
        
        # Migrate kernel include files
        if [[ -d "$REPO_ROOT/kernel/include" ]]; then
            execute "mv '$REPO_ROOT/kernel/include'/* '$REPO_ROOT/core/kernel/include/'" "Moving kernel includes"
        fi
        
        # Migrate other kernel subdirectories
        for subdir in "$REPO_ROOT/kernel"/*; do
            if [[ -d "$subdir" ]]; then
                dirname=$(basename "$subdir")
                if [[ "$dirname" != "include" ]]; then
                    execute "mv '$subdir' '$REPO_ROOT/core/kernel/'" "Moving kernel subdirectory: $dirname"
                fi
            fi
        done
    fi
    
    # Migrate driver files
    if [[ -d "$REPO_ROOT/drivers" ]]; then
        cyan "Migrating driver files..."
        execute "mv '$REPO_ROOT/drivers'/* '$REPO_ROOT/core/drivers/'" "Moving driver files"
    fi
    
    # Migrate userland files
    if [[ -d "$REPO_ROOT/userland" ]]; then
        cyan "Migrating userland files..."
        
        # Move shell-related files
        for file in "$REPO_ROOT/userland/shell"*; do
            if [[ -f "$file" ]]; then
                execute "mv '$file' '$REPO_ROOT/userspace/shell/raeshell/'" "Moving shell file: $(basename "$file")"
            fi
        done
        
        # Move application files
        for subdir in "$REPO_ROOT/userland"/*; do
            if [[ -d "$subdir" ]]; then
                dirname=$(basename "$subdir")
                case "$dirname" in
                    raeenstudio*)
                        execute "mv '$subdir'/* '$REPO_ROOT/userspace/applications/raeen_studio/'" "Moving Raeen Studio files"
                        ;;
                    app_store*)
                        execute "mv '$subdir'/* '$REPO_ROOT/userspace/applications/app_store/'" "Moving App Store files"
                        ;;
                    *)
                        execute "mv '$subdir' '$REPO_ROOT/userspace/applications/'" "Moving userland directory: $dirname"
                        ;;
                esac
            fi
        done
        
        # Move remaining userland files
        for file in "$REPO_ROOT/userland"/*.c "$REPO_ROOT/userland"/*.h; do
            if [[ -f "$file" ]]; then
                execute "mv '$file' '$REPO_ROOT/userspace/libraries/libc/'" "Moving userland file: $(basename "$file")"
            fi
        done
    fi
    
    # Migrate package manager files
    if [[ -d "$REPO_ROOT/pkg" ]]; then
        cyan "Migrating package manager files..."
        execute "mv '$REPO_ROOT/pkg'/* '$REPO_ROOT/system/services/package/'" "Moving package manager files"
    fi
    
    # Migrate test files
    if [[ -d "$REPO_ROOT/tests" ]]; then
        cyan "Migrating test files..."
        
        # Organize tests by type
        for testdir in "$REPO_ROOT/tests"/*; do
            if [[ -d "$testdir" ]]; then
                dirname=$(basename "$testdir")
                case "$dirname" in
                    unit*)
                        execute "mv '$testdir'/* '$REPO_ROOT/testing/unit/'" "Moving unit tests"
                        ;;
                    integration*)
                        execute "mv '$testdir'/* '$REPO_ROOT/testing/integration/'" "Moving integration tests"
                        ;;
                    system*)
                        execute "mv '$testdir'/* '$REPO_ROOT/testing/system/'" "Moving system tests"
                        ;;
                    *)
                        execute "mv '$testdir' '$REPO_ROOT/testing/'" "Moving test directory: $dirname"
                        ;;
                esac
            fi
        done
        
        # Move test framework files
        for file in "$REPO_ROOT/tests"/*.c "$REPO_ROOT/tests"/*.h; do
            if [[ -f "$file" ]]; then
                execute "mv '$file' '$REPO_ROOT/testing/frameworks/custom/'" "Moving test file: $(basename "$file")"
            fi
        done
    fi
    
    # Migrate tools
    if [[ -d "$REPO_ROOT/tools" ]]; then
        cyan "Migrating tool files..."
        
        for tooldir in "$REPO_ROOT/tools"/*; do
            if [[ -d "$tooldir" ]]; then
                dirname=$(basename "$tooldir")
                case "$dirname" in
                    *quality*)
                        execute "mv '$tooldir'/* '$REPO_ROOT/tools/quality/'" "Moving quality tools"
                        ;;
                    *build*)
                        execute "mv '$tooldir'/* '$REPO_ROOT/tools/build/'" "Moving build tools"
                        ;;
                    *automation*)
                        execute "mv '$tooldir'/* '$REPO_ROOT/tools/automation/'" "Moving automation tools"
                        ;;
                    *)
                        execute "mv '$tooldir' '$REPO_ROOT/tools/'" "Moving tool directory: $dirname"
                        ;;
                esac
            fi
        done
    fi
    
    # Migrate documentation
    if [[ -d "$REPO_ROOT/docs" ]]; then
        cyan "Migrating documentation..."
        execute "mv '$REPO_ROOT/docs' '$REPO_ROOT/documentation'" "Moving documentation directory"
    fi
}

# Create module templates
create_module_templates() {
    blue "📝 Creating module templates..."
    
    # Create kernel module template
    cat > "$REPO_ROOT/core/kernel/mm/Makefile" << 'EOF'
# Memory Management Module Makefile
# Owner: kernel-architect, memory-manager

include $(TOPDIR)/Makefile.config
include $(TOPDIR)/Makefile.rules

MODULE_NAME = mm
MODULE_SOURCES = $(wildcard *.c)
MODULE_INCLUDES = -I../include -I../../hal/include
MODULE_LIBS = 

# Build module library
$(eval $(call build-module,$(MODULE_NAME),$(MODULE_SOURCES),$(MODULE_INCLUDES),$(MODULE_LIBS)))

.PHONY: all clean test
all: $(LIB_DIR)/lib$(MODULE_NAME).a

test:
	$(MAKE) -C $(TOPDIR)/testing/unit/core/$(MODULE_NAME)

clean: clean-$(MODULE_NAME)
EOF

    # Create driver module template
    cat > "$REPO_ROOT/core/drivers/graphics/intel/Makefile" << 'EOF'
# Intel Graphics Driver Makefile
# Owner: gaming-layer-engineer, driver-integration-specialist

include $(TOPDIR)/Makefile.config
include $(TOPDIR)/Makefile.rules

MODULE_NAME = intel_graphics
MODULE_SOURCES = $(wildcard *.c)
MODULE_INCLUDES = -I../../framework -I../../../hal/include -I../../../kernel/include
MODULE_LIBS = -lhal -lkernel

# Build driver module
$(eval $(call build-module,$(MODULE_NAME),$(MODULE_SOURCES),$(MODULE_INCLUDES),$(MODULE_LIBS)))

.PHONY: all clean test install
all: $(LIB_DIR)/lib$(MODULE_NAME).a

install: all
	$(call install-lib-$(MODULE_NAME))

test:
	$(MAKE) -C $(TOPDIR)/testing/unit/drivers/graphics/intel

clean: clean-$(MODULE_NAME)
EOF

    # Create service module template
    cat > "$REPO_ROOT/system/services/package/Makefile" << 'EOF'
# Package Management Service Makefile
# Owner: package-manager-dev

include $(TOPDIR)/Makefile.config
include $(TOPDIR)/Makefile.rules

MODULE_NAME = package_service
MODULE_SOURCES = $(wildcard *.c)
MODULE_INCLUDES = -I../../include -I../../../core/kernel/include -I../../../core/security/include
MODULE_LIBS = -lkernel -lsecurity

# Build service module
$(eval $(call build-module,$(MODULE_NAME),$(MODULE_SOURCES),$(MODULE_INCLUDES),$(MODULE_LIBS)))

.PHONY: all clean test install
all: $(LIB_DIR)/lib$(MODULE_NAME).a

install: all
	$(call install-lib-$(MODULE_NAME))

test:
	$(MAKE) -C $(TOPDIR)/testing/unit/system/services/package

clean: clean-$(MODULE_NAME)
EOF

    # Create application module template
    cat > "$REPO_ROOT/userspace/applications/raeen_studio/notes/Makefile" << 'EOF'
# Raeen Notes Application Makefile
# Owner: raeen-studio-lead, ai-orchestrator

include $(TOPDIR)/Makefile.config
include $(TOPDIR)/Makefile.rules

MODULE_NAME = raeen_notes
MODULE_SOURCES = $(wildcard *.c)
MODULE_INCLUDES = -I../shared -I../../../../system/include -I../../../../ai/core/include
MODULE_LIBS = -lraeen_studio_shared -lai_core

# Build application
$(eval $(call build-module,$(MODULE_NAME),$(MODULE_SOURCES),$(MODULE_INCLUDES),$(MODULE_LIBS)))

.PHONY: all clean test install
all: $(BIN_DIR)/$(MODULE_NAME)

$(BIN_DIR)/$(MODULE_NAME): $(LIB_DIR)/lib$(MODULE_NAME).a
	$(LD) $(LDFLAGS) -o $@ $< $(MODULE_LIBS)

install: all
	$(call install-bin-$(MODULE_NAME))

test:
	$(MAKE) -C $(TOPDIR)/testing/unit/userspace/applications/raeen_studio/notes

clean: clean-$(MODULE_NAME)
EOF

    log "Created module templates"
}

# Create agent coordination files
create_agent_coordination_files() {
    blue "👥 Creating agent coordination files..."
    
    # Create CODEOWNERS file
    cat > "$REPO_ROOT/.github/CODEOWNERS" << 'EOF'
# RaeenOS Agent Code Ownership
# Defines which agents are responsible for reviewing changes to specific paths

# Global rules
* @lead-os-developer

# Core OS Components
/core/kernel/ @kernel-architect @privacy-security-engineer
/core/hal/ @hardware-compat-expert @driver-integration-specialist
/core/drivers/framework/ @driver-integration-specialist @hardware-compat-expert
/core/drivers/graphics/ @gaming-layer-engineer @driver-integration-specialist
/core/drivers/audio/ @audio-subsystem-engineer @creator-tools-specialist
/core/drivers/network/ @network-architect @privacy-security-engineer
/core/security/ @privacy-security-engineer @compliance-certification-specialist

# System Services
/system/services/package/ @package-manager-dev @app-store-architect
/system/services/backup/ @backup-recovery-engineer @filesystem-engineer
/system/services/telemetry/ @data-telemetry-engineer @privacy-security-engineer
/system/network/ @network-architect @privacy-security-engineer
/system/filesystem/ @filesystem-engineer @backup-recovery-engineer

# User-space Components
/userspace/desktop/ @ux-wizard @multitasking-maestro @brand-identity-guru
/userspace/desktop/accessibility/ @accessibility-champion @ux-wizard
/userspace/applications/raeen_studio/ @raeen-studio-lead @ai-orchestrator
/userspace/applications/app_store/ @app-store-architect @package-manager-dev
/userspace/shell/ @shell-cli-engineer @ai-orchestrator

# AI Integration
/ai/core/ @ai-orchestrator @shell-cli-engineer
/ai/services/ @ai-orchestrator @mobile-sync-integration-engineer
/ai/integration/ @ai-orchestrator

# Virtualization
/virtualization/hypervisor/ @virtualization-architect @gaming-layer-engineer
/virtualization/containers/ @virtualization-architect @privacy-security-engineer
/virtualization/security/ @privacy-security-engineer @virtualization-architect

# Compatibility Layers
/compatibility/windows/ @app-framework-engineer @virtualization-architect
/compatibility/macos/ @app-framework-engineer @virtualization-architect
/compatibility/android/ @app-framework-engineer @virtualization-architect

# Enterprise Features
/enterprise/deployment/ @enterprise-deployment-specialist @installer-wizard
/enterprise/compliance/ @compliance-certification-specialist @privacy-security-engineer
/enterprise/management/ @enterprise-deployment-specialist

# Development Tools
/development/sdk/ @api-sdk-architect @app-framework-engineer
/development/tools/ @various-specialists @api-sdk-architect
/development/debugger/ @performance-optimization-analyst
/development/profiler/ @performance-optimization-analyst

# Testing Infrastructure
/testing/ @testing-qa-automation-lead @code-quality-analyst
/testing/security/ @privacy-security-engineer @testing-qa-automation-lead
/testing/performance/ @performance-optimization-analyst @testing-qa-automation-lead

# Build System and Tools
/Makefile* @lead-os-developer @kernel-architect
/tools/build/ @lead-os-developer @testing-qa-automation-lead
/tools/quality/ @code-quality-analyst @testing-qa-automation-lead

# Documentation
/documentation/ @all-agents @lead-os-developer
/documentation/api/ @api-sdk-architect
/documentation/architecture/ @kernel-architect @lead-os-developer

# Deployment
/deployment/installer/ @installer-wizard @system-update-engineer
/deployment/packages/ @package-manager-dev @installer-wizard
/deployment/images/ @installer-wizard @enterprise-deployment-specialist
EOF

    # Create agent responsibility matrix
    cat > "$REPO_ROOT/AGENT_RESPONSIBILITIES.md" << 'EOF'
# RaeenOS Agent Responsibility Matrix

This document defines the primary and secondary responsibilities for each of the 42 specialized development agents.

## Core OS Components

### kernel-architect
- **Primary**: `/core/kernel/`, overall kernel design and architecture
- **Secondary**: `/core/hal/`, `/core/security/`, memory management design
- **Review**: All kernel-related changes, architectural decisions

### privacy-security-engineer
- **Primary**: `/core/security/`, security framework, sandboxing
- **Secondary**: All modules (security review), `/enterprise/compliance/`
- **Review**: All security-sensitive code, privacy implementations

### hardware-compat-expert
- **Primary**: `/core/hal/`, hardware abstraction layer
- **Secondary**: `/core/drivers/`, platform compatibility
- **Review**: Hardware-related changes, driver interfaces

### driver-integration-specialist
- **Primary**: `/core/drivers/framework/`, device management
- **Secondary**: All driver modules, hardware integration
- **Review**: Driver implementations, device management

### gaming-layer-engineer
- **Primary**: `/core/drivers/graphics/`, GPU optimization
- **Secondary**: `/virtualization/hypervisor/`, gaming performance
- **Review**: Graphics-related changes, performance optimizations

### audio-subsystem-engineer
- **Primary**: `/core/drivers/audio/`, audio system
- **Secondary**: `/userspace/applications/` (audio apps)
- **Review**: Audio-related changes, low-latency implementations

### network-architect
- **Primary**: `/system/network/`, `/core/drivers/network/`
- **Secondary**: Security networking, virtualization networking
- **Review**: Network-related changes, protocol implementations

## System Services

### package-manager-dev
- **Primary**: `/system/services/package/`, package management
- **Secondary**: `/userspace/applications/app_store/`, software distribution
- **Review**: Package management changes, software installation

### backup-recovery-engineer
- **Primary**: `/system/services/backup/`, backup systems
- **Secondary**: `/system/filesystem/`, data recovery
- **Review**: Backup-related changes, recovery mechanisms

### filesystem-engineer
- **Primary**: `/system/filesystem/`, file system implementations
- **Secondary**: `/system/services/backup/`, storage management
- **Review**: File system changes, storage implementations

### data-telemetry-engineer
- **Primary**: `/system/services/telemetry/`, system analytics
- **Secondary**: Privacy-compliant data collection
- **Review**: Telemetry implementations, data collection

## User Interface & Experience

### ux-wizard
- **Primary**: `/userspace/desktop/`, UI design and implementation
- **Secondary**: `/userspace/applications/`, user experience
- **Review**: UI/UX changes, design consistency

### multitasking-maestro
- **Primary**: `/userspace/desktop/window_manager/`, window management
- **Secondary**: Virtual desktops, workspace management
- **Review**: Window management changes, multitasking features

### brand-identity-guru
- **Primary**: `/userspace/desktop/themes/`, visual identity
- **Secondary**: Branding consistency across applications
- **Review**: Visual design changes, branding elements

### accessibility-champion
- **Primary**: `/userspace/desktop/accessibility/`, accessibility features
- **Secondary**: Inclusive design across all components
- **Review**: Accessibility implementations, inclusive design

### notification-center-architect
- **Primary**: `/system/services/notification/`, notification system
- **Secondary**: Cross-application notifications
- **Review**: Notification-related changes, alert systems

## Applications & Services

### raeen-studio-lead
- **Primary**: `/userspace/applications/raeen_studio/`, productivity suite
- **Secondary**: AI integration in applications
- **Review**: Raeen Studio changes, productivity features

### app-store-architect
- **Primary**: `/userspace/applications/app_store/`, application marketplace
- **Secondary**: Software distribution, app security
- **Review**: App Store changes, software marketplace

### shell-cli-engineer
- **Primary**: `/userspace/shell/`, command-line interface
- **Secondary**: AI-powered shell features
- **Review**: Shell implementations, CLI tools

## AI & Intelligence

### ai-orchestrator
- **Primary**: `/ai/core/`, AI framework and integration
- **Secondary**: System-wide AI features
- **Review**: AI-related changes, intelligence features

### mobile-sync-integration-engineer
- **Primary**: Cross-device synchronization, mobile integration
- **Secondary**: Cloud services integration
- **Review**: Sync implementations, mobile compatibility

## Virtualization & Compatibility

### virtualization-architect
- **Primary**: `/virtualization/hypervisor/`, virtualization systems
- **Secondary**: Container runtime, VM security
- **Review**: Virtualization changes, hypervisor implementations

### app-framework-engineer
- **Primary**: `/compatibility/`, cross-platform compatibility
- **Secondary**: Application runtime environments
- **Review**: Compatibility implementations, runtime environments

### third-party-integration-architect
- **Primary**: External software integration
- **Secondary**: Third-party service compatibility
- **Review**: Integration implementations, external APIs

## Enterprise & Deployment

### enterprise-deployment-specialist
- **Primary**: `/enterprise/deployment/`, enterprise features
- **Secondary**: Fleet management, corporate integration
- **Review**: Enterprise implementations, deployment tools

### installer-wizard
- **Primary**: `/deployment/installer/`, installation systems
- **Secondary**: OS deployment, bootable media
- **Review**: Installer changes, deployment mechanisms

### compliance-certification-specialist
- **Primary**: `/enterprise/compliance/`, regulatory compliance
- **Secondary**: Security standards, certification requirements
- **Review**: Compliance implementations, regulatory requirements

## Development & Quality

### api-sdk-architect
- **Primary**: `/development/sdk/`, developer tools and APIs
- **Secondary**: Public API design, developer experience
- **Review**: API changes, SDK implementations

### code-quality-analyst
- **Primary**: `/tools/quality/`, code quality assurance
- **Secondary**: Code review processes, quality standards
- **Review**: All code for quality standards, testing requirements

### testing-qa-automation-lead
- **Primary**: `/testing/`, testing infrastructure
- **Secondary**: Quality assurance processes
- **Review**: Testing implementations, QA processes

### performance-optimization-analyst
- **Primary**: Performance analysis and optimization
- **Secondary**: `/development/profiler/`, performance tools
- **Review**: Performance-related changes, optimization implementations

### lead-os-developer
- **Primary**: Overall project coordination, architectural oversight
- **Secondary**: All major architectural decisions
- **Review**: All significant changes, architectural decisions

---

## Collaboration Guidelines

1. **Primary owners** are responsible for implementation and maintenance
2. **Secondary owners** provide support and specialized knowledge
3. **All agents** must coordinate with security and quality agents
4. **Cross-module changes** require coordination between relevant agents
5. **Regular sync meetings** ensure alignment and prevent conflicts

## Escalation Process

1. **Direct coordination** between involved agents (preferred)
2. **Module lead** mediation for conflicts
3. **Lead OS developer** for architectural decisions
4. **Architecture board** for major system changes
EOF

    log "Created agent coordination files"
}

# Update build system
update_build_system() {
    blue "🔧 Updating build system..."
    
    # Backup original Makefile
    execute "cp '$REPO_ROOT/Makefile' '$REPO_ROOT/Makefile.original'" "Backing up original Makefile"
    
    # Use enhanced Makefile
    execute "cp '$REPO_ROOT/Makefile.enhanced' '$REPO_ROOT/Makefile'" "Installing enhanced Makefile"
    
    log "Build system updated"
}

# Clean up old directories
cleanup_old_directories() {
    blue "🧹 Cleaning up old directory structure..."
    
    local old_dirs=("kernel" "drivers" "userland" "pkg" "tests")
    
    for dir in "${old_dirs[@]}"; do
        if [[ -d "$REPO_ROOT/$dir" ]] && [[ -z "$(ls -A "$REPO_ROOT/$dir")" ]]; then
            execute "rmdir '$REPO_ROOT/$dir'" "Removing empty directory: $dir"
        elif [[ -d "$REPO_ROOT/$dir" ]]; then
            yellow "Warning: $dir is not empty, skipping removal"
            yellow "Please manually check and remove after verifying migration"
        fi
    done
}

# Validate migration
validate_migration() {
    blue "✅ Validating migration..."
    
    local issues=0
    
    # Check critical directories exist
    local critical_dirs=(
        "core/kernel"
        "core/hal"
        "core/drivers"
        "system/services"
        "userspace/desktop"
        "testing/unit"
    )
    
    for dir in "${critical_dirs[@]}"; do
        if [[ ! -d "$REPO_ROOT/$dir" ]]; then
            red "❌ Critical directory missing: $dir"
            ((issues++))
        else
            log "✅ Directory exists: $dir"
        fi
    done
    
    # Check build system files
    local build_files=(
        "Makefile"
        "Makefile.config"
        "Makefile.rules"
    )
    
    for file in "${build_files[@]}"; do
        if [[ ! -f "$REPO_ROOT/$file" ]]; then
            red "❌ Build file missing: $file"
            ((issues++))
        else
            log "✅ Build file exists: $file"
        fi
    done
    
    # Check agent coordination files
    if [[ ! -f "$REPO_ROOT/.github/CODEOWNERS" ]]; then
        red "❌ CODEOWNERS file missing"
        ((issues++))
    else
        log "✅ CODEOWNERS file exists"
    fi
    
    if [[ ! -f "$REPO_ROOT/AGENT_RESPONSIBILITIES.md" ]]; then
        red "❌ Agent responsibilities file missing"
        ((issues++))
    else
        log "✅ Agent responsibilities file exists"
    fi
    
    if [[ $issues -eq 0 ]]; then
        green "✅ Migration validation successful!"
        return 0
    else
        red "❌ Migration validation failed with $issues issues"
        return 1
    fi
}

# Main migration function
main() {
    cyan "🚀 RaeenOS Repository Structure Migration"
    cyan "======================================="
    echo
    
    if [[ "$DRY_RUN" == "true" ]]; then
        yellow "Running in DRY RUN mode - no changes will be made"
    else
        yellow "This will modify your repository structure!"
        echo "Press Enter to continue or Ctrl+C to abort..."
        read
    fi
    
    echo
    cyan "Starting migration process..."
    
    # Execute migration steps
    backup_existing_structure
    create_directory_structure
    migrate_existing_files
    create_module_templates
    create_agent_coordination_files
    update_build_system
    cleanup_old_directories
    
    if validate_migration; then
        echo
        green "🎉 Migration completed successfully!"
        echo
        cyan "Next steps:"
        echo "1. Review the migrated structure"
        echo "2. Update any build scripts or IDE configurations"
        echo "3. Test the new build system: make help"
        echo "4. Commit the changes to version control"
        echo "5. Inform all 42 agents about the new structure"
        echo
        if [[ "$DRY_RUN" == "false" ]]; then
            cyan "Backup created at: $BACKUP_DIR"
        fi
    else
        red "❌ Migration completed with issues. Please review and fix."
        exit 1
    fi
}

# Execute main function
main "$@"