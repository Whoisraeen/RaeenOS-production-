# RaePkg Package Format Specification

## Overview

RaePkg is the native package format for RaeenOS, designed for security, efficiency, and compatibility. It supports atomic installations, dependency resolution, and seamless integration with the RaeenOS ecosystem.

## Package Structure

A RaePkg package is a compressed archive (`.raepkg`) containing:

```
package.raepkg
├── META-INF/
│   ├── MANIFEST.json          # Package metadata
│   ├── SIGNATURE.sig          # Digital signature
│   ├── CHECKSUM.sha256        # File checksums
│   └── DEPENDENCIES.json     # Dependency information
├── data/                      # Package files
│   ├── usr/
│   │   ├── bin/
│   │   ├── lib/
│   │   └── share/
│   └── etc/
└── scripts/                   # Installation scripts
    ├── pre-install.sh
    ├── post-install.sh
    ├── pre-remove.sh
    └── post-remove.sh
```

## Manifest Format (MANIFEST.json)

```json
{
  "format_version": "1.0",
  "package": {
    "name": "example-app",
    "display_name": "Example Application",
    "version": "2.1.0",
    "architecture": "x86_64",
    "category": "productivity",
    "license": "MIT",
    "homepage": "https://example.com",
    "description": "An example application for demonstration",
    "summary": "Example app",
    "maintainer": {
      "name": "John Doe",
      "email": "john@example.com"
    },
    "size": {
      "installed": 52428800,
      "download": 15728640
    },
    "files": {
      "count": 156,
      "executable": [
        "usr/bin/example-app"
      ],
      "config": [
        "etc/example-app/config.conf"
      ]
    }
  },
  "build": {
    "timestamp": "2024-08-01T20:00:00Z",
    "builder": "raepkg-build 1.0.0",
    "source_url": "https://github.com/example/app",
    "commit": "abc123def456"
  },
  "security": {
    "signature_algorithm": "RSA-4096",
    "checksum_algorithm": "SHA-256",
    "publisher_id": "com.example.publisher",
    "permissions": [
      "network.access",
      "filesystem.read",
      "filesystem.write:/home/*/Documents"
    ]
  }
}
```

## Dependencies Format (DEPENDENCIES.json)

```json
{
  "dependencies": [
    {
      "name": "libssl",
      "version": ">=1.1.0",
      "type": "required",
      "architecture": "x86_64"
    },
    {
      "name": "gtk4",
      "version": ">=4.0.0",
      "type": "required",
      "provides": "gui-toolkit"
    },
    {
      "name": "ffmpeg",
      "version": ">=4.0.0",
      "type": "optional",
      "description": "For video processing features"
    }
  ],
  "conflicts": [
    {
      "name": "old-example-app",
      "version": "*",
      "reason": "Incompatible configuration format"
    }
  ],
  "provides": [
    {
      "name": "example-api",
      "version": "2.1.0",
      "description": "Example API interface"
    }
  ],
  "replaces": [
    {
      "name": "legacy-example",
      "version": "<2.0.0"
    }
  ]
}
```

## Installation Scripts

### pre-install.sh
```bash
#!/bin/bash
# Pre-installation script
# Runs before package files are extracted

set -e

# Check system requirements
if ! command -v systemctl &> /dev/null; then
    echo "Error: systemd is required"
    exit 1
fi

# Create user if needed
if ! id -u example-user &> /dev/null; then
    useradd -r -s /bin/false example-user
fi

exit 0
```

### post-install.sh
```bash
#!/bin/bash
# Post-installation script
# Runs after package files are extracted

set -e

# Enable and start service
systemctl enable example-app.service
systemctl start example-app.service

# Update desktop database
update-desktop-database /usr/share/applications

# Update icon cache
gtk-update-icon-cache /usr/share/icons/hicolor

echo "Example App installed successfully"
exit 0
```

## Security Model

### Digital Signatures
- All packages must be signed with RSA-4096 or Ed25519 keys
- Signature covers the entire package content
- Publisher certificates are verified against trusted CA

### Checksums
- SHA-256 checksums for all files
- Integrity verification during installation
- Tamper detection for installed packages

### Permissions
Packages declare required permissions:
- `network.access` - Network connectivity
- `filesystem.read` - Read filesystem access
- `filesystem.write:/path` - Write access to specific paths
- `hardware.camera` - Camera access
- `hardware.microphone` - Microphone access
- `system.admin` - Administrative privileges

## Compatibility Layers

### Flatpak Compatibility
```json
{
  "compatibility": {
    "flatpak": {
      "app_id": "com.example.App",
      "runtime": "org.freedesktop.Platform/x86_64/22.08",
      "permissions": [
        "--share=network",
        "--filesystem=home"
      ]
    }
  }
}
```

### AppImage Compatibility
```json
{
  "compatibility": {
    "appimage": {
      "desktop_file": "example-app.desktop",
      "icon": "example-app.png",
      "categories": ["Office", "Productivity"]
    }
  }
}
```

## Build Tools

### raepkg-build
Command-line tool for creating packages:

```bash
# Build from source directory
raepkg-build --source ./src --output example-app-2.1.0.raepkg

# Build with custom manifest
raepkg-build --manifest custom-manifest.json --data ./files

# Sign package
raepkg-build --sign --key publisher.key --cert publisher.crt
```

### Build Configuration (build.yaml)
```yaml
package:
  name: example-app
  version: 2.1.0
  architecture: x86_64

source:
  type: git
  url: https://github.com/example/app
  tag: v2.1.0

build:
  commands:
    - ./configure --prefix=/usr
    - make -j$(nproc)
    - make install DESTDIR=$PKG_BUILD_DIR

dependencies:
  build:
    - gcc
    - make
    - pkg-config
  runtime:
    - libssl
    - gtk4

files:
  include:
    - usr/bin/example-app
    - usr/share/applications/example-app.desktop
    - usr/share/icons/hicolor/*/apps/example-app.*
  exclude:
    - usr/share/doc
    - "*.la"

scripts:
  post_install: |
    systemctl enable example-app.service
    update-desktop-database
```

## Repository Format

### Repository Structure
```
repository/
├── metadata.json              # Repository metadata
├── packages/                  # Package files
│   ├── example-app-2.1.0.raepkg
│   └── other-package-1.0.0.raepkg
├── indices/                   # Search indices
│   ├── by-name.json
│   ├── by-category.json
│   └── by-dependency.json
└── security/
    ├── repository.key         # Repository signing key
    └── trusted-publishers.json
```

### Repository Metadata (metadata.json)
```json
{
  "repository": {
    "name": "raeen-main",
    "description": "Official RaeenOS package repository",
    "url": "https://packages.raeenos.com/main",
    "architecture": ["x86_64", "arm64"],
    "components": ["main", "universe", "restricted"],
    "last_updated": "2024-08-01T20:00:00Z",
    "package_count": 15420
  },
  "packages": [
    {
      "name": "example-app",
      "version": "2.1.0",
      "architecture": "x86_64",
      "category": "productivity",
      "download_size": 15728640,
      "installed_size": 52428800,
      "checksum": "sha256:abc123...",
      "filename": "packages/example-app-2.1.0.raepkg",
      "dependencies": ["libssl>=1.1.0", "gtk4>=4.0.0"],
      "description": "An example application for demonstration"
    }
  ]
}
```

## Delta Updates

For efficient updates, RaePkg supports delta packages:

```json
{
  "delta": {
    "from_version": "2.0.0",
    "to_version": "2.1.0",
    "delta_size": 2097152,
    "operations": [
      {
        "type": "add",
        "file": "usr/bin/new-feature"
      },
      {
        "type": "modify",
        "file": "usr/bin/example-app",
        "patch": "binary-diff.patch"
      },
      {
        "type": "remove",
        "file": "usr/share/old-file"
      }
    ]
  }
}
```

## Validation Rules

1. **Package Name**: Must match `^[a-z0-9][a-z0-9+.-]*$`
2. **Version**: Must follow semantic versioning (major.minor.patch)
3. **Architecture**: Must be one of: x86_64, arm64, x86, universal
4. **File Paths**: Must be relative and not contain `..`
5. **Scripts**: Must be executable and have proper shebang
6. **Signatures**: Must be valid and from trusted publisher
7. **Dependencies**: Must specify valid version constraints

## Error Codes

- `PKG_OK` (0): Success
- `PKG_ERROR_INVALID_FORMAT` (1): Invalid package format
- `PKG_ERROR_SIGNATURE_INVALID` (2): Invalid or missing signature
- `PKG_ERROR_CHECKSUM_MISMATCH` (3): File checksum mismatch
- `PKG_ERROR_DEPENDENCY_UNMET` (4): Unmet dependencies
- `PKG_ERROR_CONFLICT` (5): Package conflicts
- `PKG_ERROR_INSUFFICIENT_SPACE` (6): Insufficient disk space
- `PKG_ERROR_PERMISSION_DENIED` (7): Permission denied
- `PKG_ERROR_NETWORK` (8): Network error
- `PKG_ERROR_REPOSITORY_UNAVAILABLE` (9): Repository unavailable

## Best Practices

1. **Always sign packages** in production environments
2. **Use semantic versioning** for predictable updates
3. **Minimize dependencies** to reduce complexity
4. **Test installation scripts** thoroughly
5. **Provide meaningful descriptions** and metadata
6. **Use appropriate categories** for discoverability
7. **Follow filesystem hierarchy** standards
8. **Handle configuration files** properly
9. **Provide uninstall cleanup** in removal scripts
10. **Document breaking changes** in changelogs
