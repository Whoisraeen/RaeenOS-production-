#ifndef USB_H
#define USB_H

/**
 * @file usb.h
 * @brief Advanced USB 3.2/4.0 Subsystem for RaeenOS
 * 
 * Comprehensive USB implementation supporting:
 * - USB 3.2 Gen 2x2 (20 Gbps) and USB4 (40 Gbps)
 * - Thunderbolt 4 integration and USB-C Power Delivery
 * - Advanced power management and device enumeration
 * - Hot-plug with instant device recognition
 * - Multiple host controller support (xHCI, EHCI, OHCI)
 * - USB Type-C alternate modes
 * - DisplayPort over USB-C
 * - Superior performance to Windows USB stack
 * 
 * Author: RaeenOS USB Team
 * License: MIT
 * Version: 3.0.0
 */

#include "../kernel/include/types.h"
#include "../kernel/include/driver_framework.h"
#include "../kernel/include/hal_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// USB Standards and Versions
#define USB_VERSION_1_0     0x0100
#define USB_VERSION_1_1     0x0110
#define USB_VERSION_2_0     0x0200
#define USB_VERSION_3_0     0x0300
#define USB_VERSION_3_1     0x0310
#define USB_VERSION_3_2     0x0320
#define USB_VERSION_4_0     0x0400

// USB Speeds
typedef enum {
    USB_SPEED_UNKNOWN = 0,
    USB_SPEED_LOW,          // 1.5 Mbps (USB 1.0)
    USB_SPEED_FULL,         // 12 Mbps (USB 1.1)
    USB_SPEED_HIGH,         // 480 Mbps (USB 2.0)
    USB_SPEED_SUPER,        // 5 Gbps (USB 3.0)
    USB_SPEED_SUPER_PLUS,   // 10 Gbps (USB 3.1)
    USB_SPEED_SUPER_PLUS_2X2, // 20 Gbps (USB 3.2)
    USB_SPEED_USB4          // 40 Gbps (USB4)
} usb_speed_t;

// USB Host Controller Interface (HCI) types
typedef enum {
    USB_HCI_UNKNOWN = 0,
    USB_HCI_UHCI,           // Universal HCI (USB 1.1)
    USB_HCI_OHCI,           // Open HCI (USB 1.1)
    USB_HCI_EHCI,           // Enhanced HCI (USB 2.0)
    USB_HCI_XHCI,           // eXtensible HCI (USB 3.0+)
    USB_HCI_CUSTOM          // Custom implementation
} usb_hci_type_t;

// USB Device Classes
typedef enum {
    USB_CLASS_PER_INTERFACE = 0x00,
    USB_CLASS_AUDIO = 0x01,
    USB_CLASS_CDC = 0x02,
    USB_CLASS_HID = 0x03,
    USB_CLASS_PID = 0x05,
    USB_CLASS_IMAGE = 0x06,
    USB_CLASS_PRINTER = 0x07,
    USB_CLASS_MASS_STORAGE = 0x08,
    USB_CLASS_HUB = 0x09,
    USB_CLASS_CDC_DATA = 0x0A,
    USB_CLASS_SMART_CARD = 0x0B,
    USB_CLASS_CONTENT_SECURITY = 0x0D,
    USB_CLASS_VIDEO = 0x0E,
    USB_CLASS_PERSONAL_HEALTHCARE = 0x0F,
    USB_CLASS_AUDIO_VIDEO = 0x10,
    USB_CLASS_BILLBOARD = 0x11,
    USB_CLASS_USB_TYPE_C_BRIDGE = 0x12,
    USB_CLASS_DIAGNOSTIC = 0xDC,
    USB_CLASS_WIRELESS = 0xE0,
    USB_CLASS_MISCELLANEOUS = 0xEF,
    USB_CLASS_APPLICATION_SPECIFIC = 0xFE,
    USB_CLASS_VENDOR_SPECIFIC = 0xFF
} usb_device_class_t;

// USB Request Types
typedef enum {
    USB_REQ_GET_STATUS = 0,
    USB_REQ_CLEAR_FEATURE = 1,
    USB_REQ_SET_FEATURE = 3,
    USB_REQ_SET_ADDRESS = 5,
    USB_REQ_GET_DESCRIPTOR = 6,
    USB_REQ_SET_DESCRIPTOR = 7,
    USB_REQ_GET_CONFIGURATION = 8,
    USB_REQ_SET_CONFIGURATION = 9,
    USB_REQ_GET_INTERFACE = 10,
    USB_REQ_SET_INTERFACE = 11,
    USB_REQ_SYNCH_FRAME = 12
} usb_request_type_t;

// USB Descriptor Types
typedef enum {
    USB_DESC_DEVICE = 1,
    USB_DESC_CONFIGURATION = 2,
    USB_DESC_STRING = 3,
    USB_DESC_INTERFACE = 4,
    USB_DESC_ENDPOINT = 5,
    USB_DESC_DEVICE_QUALIFIER = 6,
    USB_DESC_OTHER_SPEED_CONFIG = 7,
    USB_DESC_INTERFACE_POWER = 8,
    USB_DESC_OTG = 9,
    USB_DESC_DEBUG = 10,
    USB_DESC_INTERFACE_ASSOCIATION = 11,
    USB_DESC_BOS = 15,
    USB_DESC_DEVICE_CAPABILITY = 16,
    USB_DESC_SUPERSPEED_USB_ENDPOINT_COMPANION = 48,
    USB_DESC_SUPERSPEEDPLUS_ISOCHRONOUS_ENDPOINT_COMPANION = 49
} usb_descriptor_type_t;

// USB Transfer Types
typedef enum {
    USB_TRANSFER_CONTROL = 0,
    USB_TRANSFER_ISOCHRONOUS = 1,
    USB_TRANSFER_BULK = 2,
    USB_TRANSFER_INTERRUPT = 3
} usb_transfer_type_t;

// USB-C and USB4 specific definitions
#define USB_C_CONNECTOR_TYPE_C          0x01
#define USB_C_CONNECTOR_TYPE_A          0x02
#define USB_C_CONNECTOR_TYPE_B          0x03

// Thunderbolt 4 definitions
#define TB4_CAPABILITY_ID               0x01
#define TB4_MAX_BANDWIDTH               40000   // 40 Gbps
#define TB4_MAX_DAISY_CHAIN_DEVICES     6

// Power Delivery definitions
#define USB_PD_MAX_VOLTAGE              20000   // 20V
#define USB_PD_MAX_CURRENT              5000    // 5A
#define USB_PD_MAX_POWER                100000  // 100W

// Forward declarations
typedef struct usb_device usb_device_t;
typedef struct usb_interface usb_interface_t;
typedef struct usb_endpoint usb_endpoint_t;
typedef struct usb_transfer usb_transfer_t;
typedef struct usb_host_controller usb_host_controller_t;
typedef struct usb_hub usb_hub_t;

// USB Device Descriptor
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} __attribute__((packed)) usb_device_descriptor_t;

// USB Configuration Descriptor
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
} __attribute__((packed)) usb_config_descriptor_t;

// USB Interface Descriptor
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} __attribute__((packed)) usb_interface_descriptor_t;

// USB Endpoint Descriptor
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} __attribute__((packed)) usb_endpoint_descriptor_t;

// USB SuperSpeed Endpoint Companion Descriptor
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bMaxBurst;
    uint8_t bmAttributes;
    uint16_t wBytesPerInterval;
} __attribute__((packed)) usb_ss_endpoint_companion_descriptor_t;

// USB BOS (Binary Object Store) Descriptor
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumDeviceCaps;
} __attribute__((packed)) usb_bos_descriptor_t;

// USB Setup Packet
typedef struct {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} __attribute__((packed)) usb_setup_packet_t;

// USB Transfer Request Block (TRB) for xHCI
typedef struct {
    uint64_t parameter;
    uint32_t status;
    uint32_t control;
} __attribute__((packed)) usb_trb_t;

// USB Transfer structure
struct usb_transfer {
    usb_device_t* device;
    usb_endpoint_t* endpoint;
    void* buffer;
    size_t length;
    size_t actual_length;
    usb_transfer_type_t type;
    uint32_t flags;
    int status;
    
    // Callback for completion
    void (*complete)(usb_transfer_t* transfer);
    void* context;
    
    // Internal fields
    usb_trb_t* trbs;
    uint32_t num_trbs;
    uint64_t submit_time;
    struct usb_transfer* next;
};

// USB Endpoint structure
struct usb_endpoint {
    uint8_t address;
    usb_transfer_type_t type;
    usb_speed_t speed;
    uint16_t max_packet_size;
    uint8_t interval;
    bool is_input;
    
    // SuperSpeed specific
    uint8_t max_burst;
    uint16_t max_streams;
    
    // Transfer management
    usb_transfer_t* pending_transfers;
    void* hc_private;           // Host controller private data
    
    usb_interface_t* interface;
};

// USB Interface structure
struct usb_interface {
    uint8_t number;
    uint8_t alternate_setting;
    usb_device_class_t class;
    uint8_t subclass;
    uint8_t protocol;
    
    usb_endpoint_t* endpoints;
    uint32_t num_endpoints;
    
    usb_device_t* device;
    driver_t* driver;           // Bound driver
    void* driver_data;
};

// USB Hub structure
struct usb_hub {
    usb_device_t* device;
    uint8_t num_ports;
    uint8_t characteristics;
    uint16_t power_on_delay;
    uint8_t current_per_port;
    
    // Port status
    struct {
        bool connected;
        bool enabled;
        bool suspended;
        bool reset;
        bool power;
        usb_speed_t speed;
        usb_device_t* device;
    } *ports;
    
    // Hub interrupt endpoint
    usb_endpoint_t* int_endpoint;
    usb_transfer_t* status_transfer;
};

// USB Device structure
struct usb_device {
    // Device identification
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t device_version;
    uint8_t device_class;
    uint8_t device_subclass;
    uint8_t device_protocol;
    
    // Device hierarchy
    usb_device_t* parent;
    usb_hub_t* hub;
    uint8_t port_number;
    uint8_t address;            // USB address (1-127)
    usb_speed_t speed;
    
    // Configuration
    usb_device_descriptor_t descriptor;
    usb_config_descriptor_t* config;
    usb_interface_t* interfaces;
    uint32_t num_interfaces;
    uint8_t current_config;
    
    // String descriptors
    char* manufacturer;
    char* product;
    char* serial_number;
    
    // USB-C and Power Delivery
    struct {
        bool is_usb_c;
        bool supports_pd;
        uint16_t pd_voltage;        // mV
        uint16_t pd_current;        // mA
        uint32_t pd_power;          // mW
        bool supports_alt_mode;
        uint32_t alt_modes;         // Bitmask of supported alternate modes
    } usb_c;
    
    // Thunderbolt 4
    struct {
        bool is_tb4;
        uint8_t generation;
        uint32_t bandwidth;         // Mbps
        uint8_t daisy_chain_position;
        bool supports_pcie_tunneling;
        bool supports_dp_tunneling;
        bool supports_usb_tunneling;
    } thunderbolt;
    
    // Host controller
    usb_host_controller_t* hc;
    
    // Driver framework integration
    device_t* device_obj;
    
    // State management
    enum {
        USB_DEVICE_STATE_ATTACHED,
        USB_DEVICE_STATE_POWERED,
        USB_DEVICE_STATE_DEFAULT,
        USB_DEVICE_STATE_ADDRESS,
        USB_DEVICE_STATE_CONFIGURED,
        USB_DEVICE_STATE_SUSPENDED
    } state;
    
    // Synchronization
    void* lock;
    
    // Linked list for enumeration
    usb_device_t* next;
};

// USB Host Controller structure
struct usb_host_controller {
    usb_hci_type_t type;
    uint32_t version;
    const char* name;
    
    // Hardware information
    void* registers;
    size_t register_size;
    int irq;
    pci_device_t* pci_dev;      // If PCI-based
    
    // Capabilities
    uint32_t max_devices;
    uint32_t max_endpoints;
    uint32_t max_transfers;
    bool supports_64bit;
    usb_speed_t max_speed;
    
    // Root hub
    usb_hub_t* root_hub;
    
    // Device management
    usb_device_t* devices;
    uint32_t device_count;
    uint8_t device_addresses[128]; // Address allocation bitmap
    
    // Transfer management
    usb_transfer_t* pending_transfers;
    uint32_t transfer_count;
    
    // Operations
    struct {
        int (*start)(usb_host_controller_t* hc);
        int (*stop)(usb_host_controller_t* hc);
        int (*reset)(usb_host_controller_t* hc);
        
        // Device management
        int (*enable_device)(usb_host_controller_t* hc, usb_device_t* dev);
        int (*disable_device)(usb_host_controller_t* hc, usb_device_t* dev);
        int (*reset_device)(usb_host_controller_t* hc, usb_device_t* dev);
        
        // Endpoint management
        int (*configure_endpoint)(usb_host_controller_t* hc, usb_endpoint_t* ep);
        int (*deconfigure_endpoint)(usb_host_controller_t* hc, usb_endpoint_t* ep);
        
        // Transfer operations
        int (*submit_transfer)(usb_host_controller_t* hc, usb_transfer_t* transfer);
        int (*cancel_transfer)(usb_host_controller_t* hc, usb_transfer_t* transfer);
        
        // Power management
        int (*suspend)(usb_host_controller_t* hc);
        int (*resume)(usb_host_controller_t* hc);
        
        // Hub operations
        int (*hub_status_change)(usb_host_controller_t* hc, usb_hub_t* hub);
    } ops;
    
    // Statistics and monitoring
    struct {
        uint64_t transfers_completed;
        uint64_t transfers_failed;
        uint64_t bytes_transferred;
        uint32_t devices_enumerated;
        uint32_t errors;
        uint64_t bandwidth_used;        // bps
    } stats;
    
    // Synchronization
    void* lock;
    
    // Driver framework integration
    driver_t* driver;
    
    // Linked list
    usb_host_controller_t* next;
};

// USB Subsystem Functions

// Core initialization and cleanup
int usb_init(void);
void usb_cleanup(void);

// Host controller management
int usb_register_host_controller(usb_host_controller_t* hc);
int usb_unregister_host_controller(usb_host_controller_t* hc);
usb_host_controller_t* usb_find_host_controller(const char* name);

// Device enumeration and management
int usb_enumerate_device(usb_host_controller_t* hc, usb_device_t* parent, uint8_t port);
int usb_configure_device(usb_device_t* dev, uint8_t config);
int usb_reset_device(usb_device_t* dev);
int usb_suspend_device(usb_device_t* dev);
int usb_resume_device(usb_device_t* dev);

// Device discovery
usb_device_t* usb_find_device(uint16_t vendor_id, uint16_t product_id);
usb_device_t* usb_find_device_by_class(usb_device_class_t class);
usb_device_t* usb_get_device_by_address(usb_host_controller_t* hc, uint8_t address);

// Transfer management
usb_transfer_t* usb_alloc_transfer(usb_device_t* dev, usb_endpoint_t* ep, size_t length);
void usb_free_transfer(usb_transfer_t* transfer);
int usb_submit_transfer(usb_transfer_t* transfer);
int usb_cancel_transfer(usb_transfer_t* transfer);
int usb_wait_for_transfer(usb_transfer_t* transfer, uint32_t timeout);

// Control transfers
int usb_control_transfer(usb_device_t* dev, uint8_t request_type, uint8_t request, 
                        uint16_t value, uint16_t index, void* data, uint16_t length);
int usb_get_descriptor(usb_device_t* dev, uint8_t desc_type, uint8_t desc_index, 
                      void* buffer, uint16_t length);
int usb_set_configuration(usb_device_t* dev, uint8_t config);
int usb_set_interface(usb_device_t* dev, uint8_t interface, uint8_t alt_setting);

// Bulk transfers
int usb_bulk_transfer(usb_device_t* dev, uint8_t endpoint, void* data, 
                     size_t length, size_t* actual_length, uint32_t timeout);

// Interrupt transfers
int usb_interrupt_transfer(usb_device_t* dev, uint8_t endpoint, void* data, 
                          size_t length, size_t* actual_length, uint32_t timeout);

// Isochronous transfers
int usb_iso_transfer(usb_device_t* dev, uint8_t endpoint, void* data, 
                    size_t length, uint32_t num_packets, uint32_t timeout);

// Hub management
int usb_register_hub(usb_hub_t* hub);
int usb_unregister_hub(usb_hub_t* hub);
int usb_hub_port_status_change(usb_hub_t* hub, uint8_t port);

// USB-C and Power Delivery
int usb_c_negotiate_power(usb_device_t* dev, uint16_t voltage, uint16_t current);
int usb_c_enter_alt_mode(usb_device_t* dev, uint32_t mode);
int usb_c_exit_alt_mode(usb_device_t* dev, uint32_t mode);
bool usb_c_supports_displayport(usb_device_t* dev);

// Thunderbolt 4
int usb_tb4_enumerate_tunnel(usb_device_t* dev);
int usb_tb4_setup_pcie_tunnel(usb_device_t* dev);
int usb_tb4_setup_dp_tunnel(usb_device_t* dev);
uint32_t usb_tb4_get_bandwidth(usb_device_t* dev);

// xHCI specific functions
int xhci_init(usb_host_controller_t* hc);
int xhci_configure_streams(usb_endpoint_t* ep, uint32_t num_streams);
int xhci_enable_slot(usb_host_controller_t* hc, usb_device_t* dev);
int xhci_configure_device(usb_host_controller_t* hc, usb_device_t* dev);

// Hot-plug support
int usb_hotplug_add_device(usb_device_t* dev);
int usb_hotplug_remove_device(usb_device_t* dev);
void usb_hotplug_handler(usb_host_controller_t* hc, uint8_t port, bool connected);

// Power management
int usb_pm_suspend_device(usb_device_t* dev);
int usb_pm_resume_device(usb_device_t* dev);
int usb_pm_set_power_state(usb_device_t* dev, uint8_t state);

// Performance optimization
int usb_optimize_bandwidth(usb_host_controller_t* hc);
int usb_enable_lpm(usb_device_t* dev); // Link Power Management
int usb_configure_streams(usb_endpoint_t* ep, uint32_t num_streams);

// Error handling and diagnostics
int usb_handle_error(usb_device_t* dev, uint32_t error_code);
void usb_dump_device_info(usb_device_t* dev);
void usb_dump_transfer_ring(usb_endpoint_t* ep);
int usb_get_statistics(usb_host_controller_t* hc, void* stats, size_t size);

// Utility functions
const char* usb_speed_to_string(usb_speed_t speed);
const char* usb_class_to_string(usb_device_class_t class);
const char* usb_transfer_type_to_string(usb_transfer_type_t type);
uint32_t usb_speed_to_bandwidth(usb_speed_t speed); // Returns bandwidth in Mbps
bool usb_is_superspeed(usb_speed_t speed);

// Driver registration helpers
#define USB_DRIVER_REGISTER(name, id_table) \
    static int __init name##_init(void) { \
        return usb_register_driver(&name##_driver); \
    } \
    static void __exit name##_exit(void) { \
        usb_unregister_driver(&name##_driver); \
    }

#define USB_DEVICE_ID(vendor, product) \
    { .vendor_id = (vendor), .product_id = (product) }

#define USB_DEVICE_CLASS_ID(class, subclass, protocol) \
    { .device_class = (class), .device_subclass = (subclass), \
      .device_protocol = (protocol) }

// Error codes
#define USB_SUCCESS            0
#define USB_ERR_NO_DEVICE     -4001
#define USB_ERR_NO_MEMORY     -4002
#define USB_ERR_TIMEOUT       -4003
#define USB_ERR_STALL         -4004
#define USB_ERR_BABBLE        -4005
#define USB_ERR_SHORT_PACKET  -4006
#define USB_ERR_PROTOCOL      -4007
#define USB_ERR_BANDWIDTH     -4008
#define USB_ERR_POWER         -4009
#define USB_ERR_NOT_SUPPORTED -4010

#ifdef __cplusplus
}
#endif

#endif // USB_H
