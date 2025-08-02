# RaeCompat - Windows Compatibility Layer for RaeenOS

RaeCompat is RaeenOS's comprehensive Windows compatibility layer that enables seamless execution of Windows applications and games with near-native performance. Built on Wine, DXVK, VKD3D-Proton, and other proven technologies, RaeCompat provides an integrated, user-friendly experience for running Windows software on RaeenOS.

## Overview

RaeCompat consists of several integrated components:

- **Wine Integration** - Core Windows API compatibility layer
- **DXVK** - DirectX 9/10/11 to Vulkan translation
- **VKD3D-Proton** - DirectX 12 to Vulkan translation  
- **Proton-GE** - Enhanced Wine builds with gaming optimizations
- **RaeenGameManager** - Native GUI for game management
- **ProtonCTL** - Command-line management tool
- **Automated Setup** - Winetricks integration for dependencies

## Quick Start

### 1. Installation

Run the setup script to install all components:

```bash
cd /path/to/RaeenOS/raecompat
chmod +x setup_raecompat.sh
sudo ./setup_raecompat.sh
```

This will:
- Install Wine, DXVK, VKD3D-Proton, and Proton-GE
- Set up system directories and user environment
- Create a default Wine prefix
- Install common dependencies via Winetricks

### 2. Launch RaeenGameManager

```bash
raeen-game-manager
```

The native GUI provides an intuitive Steam-like interface for:
- Scanning and importing games
- Installing new applications
- Configuring per-game settings
- Monitoring performance

### 3. Command-Line Management

Use `protonctl` for advanced management:

```bash
# List Wine prefixes
protonctl list-prefixes

# Create a new prefix for a game
protonctl create-prefix mygame --dxvk --vkd3d

# Install an application
protonctl install-app /path/to/game.exe -p mygame

# Launch an application
protonctl launch-app "My Game"
```

## Architecture

### Core Components

#### RaeCompat Core (`raecompat_core.c/h`)
- Central compatibility framework
- Wine prefix management
- Application registration and launching
- Performance monitoring
- System diagnostics

#### RaeenGameManager (`raeen_game_manager.c/h`)
- Native GUI built with RaeenUI
- Game library management
- Steam/Epic/GOG integration
- ProtonDB compatibility database
- Installation management

#### ProtonCTL (`protonctl.c`)
- Command-line interface
- Prefix and application management
- Batch operations
- Scripting support

### Directory Structure

```
/usr/lib/raecompat/          # System binaries
├── wine/                    # Wine installation
├── dxvk/                    # DXVK libraries
├── vkd3d/                   # VKD3D-Proton libraries
├── proton/                  # Proton-GE versions
└── winetricks/              # Winetricks script

~/.raecompat/                # User data
├── prefixes/                # Wine prefixes
├── games/                   # Game installations
├── downloads/               # Download cache
├── logs/                    # Application logs
└── config/                  # Configuration files
```

## Configuration

### Wine Prefixes

Each application runs in its own isolated Wine prefix, providing:
- Separate Windows environments
- Independent DLL configurations
- Isolated registry settings
- Per-application Wine versions

### Graphics Configuration

RaeCompat automatically detects and configures:
- **Vulkan Support** - Required for DXVK/VKD3D
- **DXVK** - DirectX 9/10/11 → Vulkan translation
- **VKD3D-Proton** - DirectX 12 → Vulkan translation
- **GPU Drivers** - NVIDIA, AMD, Intel support

### Performance Optimization

Built-in optimizations include:
- **esync/fsync** - Reduced CPU overhead
- **DXVK State Cache** - Faster shader compilation
- **MangoHUD** - Performance overlay
- **GameMode** - CPU governor optimization

## Game Compatibility

### Compatibility Tiers

RaeCompat uses ProtonDB compatibility ratings:

- **Platinum** - Works perfectly out of the box
- **Gold** - Works perfectly after tweaks
- **Silver** - Works with minor issues
- **Bronze** - Works but with significant issues
- **Borked** - Doesn't work

### Anti-Cheat Support

Limited support for anti-cheat systems:
- **EasyAntiCheat** - Some games work with userspace version
- **BattlEye** - Limited support, requires per-game configuration
- **Kernel-mode AC** - Not supported (requires Windows kernel)

### Supported Launchers

- **Steam** - Full support via Proton integration
- **Epic Games** - Works with some configuration
- **GOG Galaxy** - Basic functionality
- **Origin/EA App** - Limited support
- **Ubisoft Connect** - Basic functionality

## Advanced Usage

### Custom Wine Versions

Install and use different Wine versions:

```bash
# Install Wine Staging
protonctl install-wine staging

# Create prefix with specific version
protonctl create-prefix mygame --wine-version staging
```

### DLL Overrides

Configure DLL overrides for compatibility:

```bash
# Set DLL override via ProtonCTL
protonctl configure mygame --dll-override "d3d11=native"

# Or use winetricks
WINEPREFIX=~/.raecompat/prefixes/mygame winetricks
```

### Performance Tuning

Enable performance features:

```bash
# Enable DXVK and VKD3D
protonctl configure mygame --dxvk --vkd3d

# Enable MangoHUD overlay
export MANGOHUD=1
protonctl launch-app "My Game"

# Enable GameMode
gamemoded &
protonctl launch-app "My Game"
```

## Troubleshooting

### Common Issues

#### Game Won't Start
1. Check Wine prefix configuration
2. Verify required dependencies are installed
3. Check compatibility database
4. Review application logs

#### Poor Performance
1. Enable DXVK for DirectX games
2. Enable VKD3D for DirectX 12 games
3. Check Vulkan driver installation
4. Monitor system resources with MangoHUD

#### Graphics Issues
1. Update GPU drivers
2. Verify Vulkan support: `vulkaninfo`
3. Check DXVK/VKD3D logs
4. Try different Wine versions

### Diagnostic Tools

```bash
# Run system diagnostics
protonctl diagnostics

# Check Wine configuration
winecfg

# Test Vulkan support
vulkaninfo --summary

# Monitor performance
mangohud glxgears
```

### Log Files

Application logs are stored in:
- `~/.raecompat/logs/` - RaeCompat logs
- `~/.raecompat/prefixes/[name]/` - Wine prefix logs
- `/tmp/wine-[user]/` - Wine runtime logs

## Development

### Building from Source

```bash
# Clone RaeenOS repository
git clone https://github.com/RaeenOS/RaeenOS.git
cd RaeenOS/raecompat

# Build RaeCompat components
make raecompat

# Build RaeenGameManager
make raeen-game-manager

# Build ProtonCTL
make protonctl
```

### API Integration

RaeCompat provides C APIs for integration:

```c
#include "raecompat_core.h"

// Initialize compatibility layer
RaeCompatContext* ctx = raecompat_init();

// Create Wine prefix
RaeCompatPrefix* prefix = raecompat_create_prefix(ctx, "mygame");

// Register application
RaeCompatAppConfig config = {
    .name = "My Game",
    .executable_path = "/path/to/game.exe",
    .app_type = RAECOMPAT_APP_GAME,
    .dxvk_enabled = true
};
RaeCompatApplication* app = raecompat_register_application(ctx, &config);

// Launch application
RaeCompatProcessInfo* process = raecompat_launch_application(ctx, "My Game");
```

## Contributing

### Reporting Issues

When reporting compatibility issues:

1. Run diagnostics: `protonctl diagnostics`
2. Include system information
3. Provide application logs
4. Specify exact steps to reproduce
5. Include Wine/DXVK versions used

### Adding Game Configurations

Submit game-specific configurations:

1. Test the game thoroughly
2. Document required settings
3. Create configuration profile
4. Submit pull request with test results

### ProtonDB Integration

Help improve compatibility data:

1. Test games and report results
2. Submit working configurations
3. Update compatibility ratings
4. Share performance optimizations

## FAQ

### Q: Can I run all Windows games?
A: Most games work well, but compatibility varies. Check ProtonDB for specific games.

### Q: Will anti-cheat games work?
A: Some work with userspace anti-cheat, but kernel-mode anti-cheat is not supported.

### Q: How is performance compared to Windows?
A: Performance is typically 80-95% of native Windows, sometimes better due to optimizations.

### Q: Can I use my existing Steam library?
A: Yes, Steam Proton integration allows using your existing Steam library.

### Q: Do I need a Windows license?
A: No, RaeCompat uses Wine which implements Windows APIs without requiring Windows.

## Support

- **Documentation**: `/usr/share/doc/raecompat/`
- **Community Forum**: https://forum.raeenos.org/c/raecompat
- **Bug Reports**: https://github.com/RaeenOS/RaeenOS/issues
- **Discord**: https://discord.gg/raeenos

## License

RaeCompat is part of RaeenOS and is licensed under the GPL v3. Individual components may have different licenses:

- Wine: LGPL v2.1+
- DXVK: zlib License
- VKD3D-Proton: LGPL v2.1+
- Proton-GE: Various (see component licenses)

---

*For the latest updates and detailed technical documentation, visit the [RaeenOS Wiki](https://wiki.raeenos.org/raecompat).*
