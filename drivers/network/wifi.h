/**
 * RaeenOS WiFi Driver - Production Implementation
 * Supports Intel, Realtek, Broadcom, Atheros chipsets
 * 802.11a/b/g/n/ac/ax standards
 *
 * NOTE: This header defines a comprehensive API for a production-grade WiFi driver.
 * The actual hardware-specific implementations are complex and are placeholders in wifi.c.
 */

#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>
#include <stdbool.h>
#include "../pci/pci.h"
#include "../../kernel/memory.h"
#include "string.h"

// Forward declaration for interrupt registers
typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

// WiFi Standards
#define WIFI_STANDARD_80211A    0x01
#define WIFI_STANDARD_80211B    0x02
#define WIFI_STANDARD_80211G    0x04
#define WIFI_STANDARD_80211N    0x08
#define WIFI_STANDARD_80211AC   0x10
#define WIFI_STANDARD_80211AX   0x20

// Security Types
typedef enum {
    WIFI_SECURITY_NONE = 0,
    WIFI_SECURITY_WEP,
    WIFI_SECURITY_WPA,
    WIFI_SECURITY_WPA2,
    WIFI_SECURITY_WPA3,
    WIFI_SECURITY_WPS
} wifi_security_t;

// WiFi States
typedef enum {
    WIFI_STATE_DISABLED = 0,
    WIFI_STATE_DISCONNECTED,
    WIFI_STATE_SCANNING,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_ERROR
} wifi_state_t;

// WiFi Chipset Types
typedef enum {
    WIFI_CHIPSET_UNKNOWN = 0,
    WIFI_CHIPSET_INTEL_AX200,
    WIFI_CHIPSET_INTEL_AC9560,
    WIFI_CHIPSET_INTEL_AC8265,
    WIFI_CHIPSET_REALTEK_8821CE,
    WIFI_CHIPSET_REALTEK_8822CE,
    WIFI_CHIPSET_BROADCOM_BCM4360,
    WIFI_CHIPSET_ATHEROS_AR9485,
    WIFI_CHIPSET_MEDIATEK_MT7921
} wifi_chipset_t;

// Network Information
typedef struct {
    char ssid[33];              // Network name (32 chars + null)
    uint8_t bssid[6];           // MAC address of access point
    int8_t rssi;                // Signal strength in dBm
    uint8_t channel;            // WiFi channel (1-14 for 2.4GHz, 36-165 for 5GHz)
    wifi_security_t security;   // Security type
    uint32_t frequency;         // Frequency in MHz
    uint16_t capabilities;      // Capability flags
    bool hidden;                // Hidden network flag
} wifi_network_t;

// WiFi Configuration
typedef struct {
    char ssid[33];
    char password[64];
    wifi_security_t security;
    bool auto_connect;
    uint32_t timeout_ms;
} wifi_config_t;

// WiFi Statistics
typedef struct {
    uint64_t tx_packets;
    uint64_t rx_packets;
    uint64_t tx_bytes;
    uint64_t rx_bytes;
    uint32_t tx_errors;
    uint32_t rx_errors;
    uint32_t tx_dropped;
    uint32_t rx_dropped;
    int8_t signal_strength;
    uint32_t link_speed_mbps;
    uint32_t frequency;
    uint8_t channel;
} wifi_stats_t;

// WiFi Device Structure
typedef struct {
    pci_device_t* pci_dev;
    wifi_chipset_t chipset;
    uint32_t supported_standards;
    void* mmio_base;
    uint32_t mmio_size;
    uint8_t irq;
    
    // Current state
    wifi_state_t state;
    wifi_config_t current_config;
    wifi_stats_t stats;
    
    // Scan results
    wifi_network_t* scan_results;
    uint32_t scan_count;
    uint32_t scan_capacity;
    
    // Hardware registers
    volatile uint32_t* command_reg;
    volatile uint32_t* status_reg;
    volatile uint32_t* config_reg;
    volatile uint32_t* interrupt_reg;
    
    // Firmware
    void* firmware_data;
    uint32_t firmware_size;
    bool firmware_loaded;
    
    // Buffers
    void* tx_buffer;
    void* rx_buffer;
    uint32_t buffer_size;
    
    // Callbacks
    void (*scan_complete_callback)(wifi_network_t* networks, uint32_t count);
    void (*connect_callback)(bool success, const char* error);
    void (*disconnect_callback)(void);
} wifi_device_t;

// Function prototypes

// Core driver functions
bool wifi_init(void);
void wifi_shutdown(void);
bool wifi_detect_hardware(void);
wifi_device_t* wifi_get_device(void);

// Device management
bool wifi_device_init(wifi_device_t* dev);
void wifi_device_shutdown(wifi_device_t* dev);
bool wifi_load_firmware(wifi_device_t* dev);
bool wifi_reset_device(wifi_device_t* dev);

// Network operations
bool wifi_enable(void);
bool wifi_disable(void);
bool wifi_scan_networks(void);
wifi_network_t* wifi_get_scan_results(uint32_t* count);
bool wifi_connect(const wifi_config_t* config);
bool wifi_disconnect(void);
bool wifi_is_connected(void);

// Configuration
bool wifi_set_power_save(bool enabled);
bool wifi_set_channel(uint8_t channel);
bool wifi_set_tx_power(int8_t power_dbm);
bool wifi_get_stats(wifi_stats_t* stats);
wifi_state_t wifi_get_state(void);

// Security functions
bool wifi_validate_wpa_key(const char* key);
bool wifi_validate_wep_key(const char* key);
bool wifi_generate_psk(const char* ssid, const char* passphrase, uint8_t* psk);

// Utility functions
const char* wifi_security_to_string(wifi_security_t security);
const char* wifi_state_to_string(wifi_state_t state);
const char* wifi_chipset_to_string(wifi_chipset_t chipset);
int8_t wifi_rssi_to_percentage(int8_t rssi);
uint32_t wifi_channel_to_frequency(uint8_t channel);
uint8_t wifi_frequency_to_channel(uint32_t frequency);

// Chipset-specific functions
bool wifi_intel_init(wifi_device_t* dev);
bool wifi_realtek_init(wifi_device_t* dev);
bool wifi_broadcom_init(wifi_device_t* dev);
bool wifi_atheros_init(wifi_device_t* dev);
bool wifi_mediatek_init(wifi_device_t* dev);

// Interrupt handlers
void wifi_interrupt_handler(registers_t* regs);
void wifi_handle_scan_complete(wifi_device_t* dev);
void wifi_handle_connect_complete(wifi_device_t* dev);
void wifi_handle_disconnect(wifi_device_t* dev);
void wifi_handle_rx_packet(wifi_device_t* dev);
void wifi_handle_tx_complete(wifi_device_t* dev);

// 802.11 frame processing
bool wifi_send_management_frame(wifi_device_t* dev, uint8_t* frame, uint32_t length);
bool wifi_send_data_frame(wifi_device_t* dev, uint8_t* data, uint32_t length);
void wifi_process_beacon(wifi_device_t* dev, uint8_t* frame, uint32_t length);
void wifi_process_probe_response(wifi_device_t* dev, uint8_t* frame, uint32_t length);
void wifi_process_auth_response(wifi_device_t* dev, uint8_t* frame, uint32_t length);
void wifi_process_assoc_response(wifi_device_t* dev, uint8_t* frame, uint32_t length);

// WPA/WPA2 functions
bool wifi_wpa_handshake(wifi_device_t* dev, const wifi_config_t* config);
bool wifi_wpa_generate_ptk(const uint8_t* pmk, const uint8_t* anonce, 
                          const uint8_t* snonce, const uint8_t* aa, 
                          const uint8_t* spa, uint8_t* ptk);
bool wifi_wpa_encrypt_data(const uint8_t* key, const uint8_t* data, 
                          uint32_t length, uint8_t* encrypted);

// Power management
bool wifi_enter_power_save(wifi_device_t* dev);
bool wifi_exit_power_save(wifi_device_t* dev);
void wifi_update_power_state(wifi_device_t* dev);

// Debug and monitoring
void wifi_dump_registers(wifi_device_t* dev);
void wifi_dump_scan_results(void);
void wifi_dump_stats(void);
void wifi_enable_debug(bool enabled);

// Constants
#define WIFI_MAX_NETWORKS           64
#define WIFI_SCAN_TIMEOUT_MS        10000
#define WIFI_CONNECT_TIMEOUT_MS     30000
#define WIFI_MAX_SSID_LENGTH        32
#define WIFI_MAX_PASSWORD_LENGTH    63
#define WIFI_BUFFER_SIZE            (64 * 1024)  // 64KB buffers
#define WIFI_FIRMWARE_MAX_SIZE      (1024 * 1024) // 1MB max firmware

// PCI Vendor/Device IDs for common WiFi chipsets
#define WIFI_VENDOR_INTEL           0x8086
#define WIFI_VENDOR_REALTEK         0x10EC
#define WIFI_VENDOR_BROADCOM        0x14E4
#define WIFI_VENDOR_ATHEROS         0x168C
#define WIFI_VENDOR_MEDIATEK        0x14C3

// Intel WiFi device IDs
#define WIFI_INTEL_AX200_ID         0x2723
#define WIFI_INTEL_AC9560_ID        0x9DF0
#define WIFI_INTEL_AC8265_ID        0x24FD

// Realtek WiFi device IDs
#define WIFI_REALTEK_8821CE_ID      0xC821
#define WIFI_REALTEK_8822CE_ID      0xC822

// Broadcom WiFi device IDs
#define WIFI_BROADCOM_BCM4360_ID    0x43A0

// Atheros WiFi device IDs
#define WIFI_ATHEROS_AR9485_ID      0x0032

// MediaTek WiFi device IDs
#define WIFI_MEDIATEK_MT7921_ID     0x7961

#endif // WIFI_H