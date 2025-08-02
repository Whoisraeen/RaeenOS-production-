#ifndef INPUT_H
#define INPUT_H

/**
 * @file input.h
 * @brief Advanced Input Device Framework for RaeenOS
 * 
 * This framework provides comprehensive input device support including:
 * - Gaming-optimized keyboard and mouse with ultra-low latency
 * - Multi-touch support with gesture recognition
 * - Game controller support (Xbox, PlayStation, Nintendo, generic HID)
 * - Pen/stylus input with pressure sensitivity
 * - Haptic feedback and force feedback
 * - Adaptive refresh rate and polling rate optimization
 * - Input device hot-plugging with instant recognition
 * - Superior performance to Windows DirectInput/XInput
 * 
 * Author: RaeenOS Input Team
 * License: MIT
 * Version: 2.0.0
 */

#include "../kernel/include/types.h"
#include "../kernel/include/driver_framework.h"
#include "../kernel/include/hal_interface.h"
#include "../usb/usb.h"

#ifdef __cplusplus
extern "C" {
#endif

// Input subsystem constants
#define INPUT_MAX_DEVICES       256
#define INPUT_MAX_TOUCH_POINTS  10
#define INPUT_MAX_CONTROLLERS   8
#define INPUT_MAX_KEYS          256
#define INPUT_MAX_MOUSE_BUTTONS 32
#define INPUT_MAX_GAMEPAD_BUTTONS 32
#define INPUT_MAX_AXES          16
#define INPUT_EVENT_QUEUE_SIZE  1024

// Input device types
typedef enum {
    INPUT_DEVICE_UNKNOWN = 0,
    INPUT_DEVICE_KEYBOARD,          // Keyboard
    INPUT_DEVICE_MOUSE,             // Mouse
    INPUT_DEVICE_TOUCHSCREEN,       // Touchscreen
    INPUT_DEVICE_TOUCHPAD,          // Touchpad
    INPUT_DEVICE_GAMEPAD,           // Game controller
    INPUT_DEVICE_JOYSTICK,          // Joystick
    INPUT_DEVICE_WHEEL,             // Racing wheel
    INPUT_DEVICE_PEN,               // Stylus/pen
    INPUT_DEVICE_TABLET,            // Graphics tablet
    INPUT_DEVICE_TRACKBALL,         // Trackball
    INPUT_DEVICE_GYROSCOPE,         // Motion sensor
    INPUT_DEVICE_ACCELEROMETER,     // Accelerometer
    INPUT_DEVICE_LIGHT_GUN,         // Light gun
    INPUT_DEVICE_DANCE_PAD,         // Dance pad
    INPUT_DEVICE_MIDI,              // MIDI controller
    INPUT_DEVICE_REMOTE,            // Remote control
    INPUT_DEVICE_CUSTOM             // Custom device
} input_device_type_t;

// Input event types
typedef enum {
    INPUT_EVENT_NONE = 0,
    INPUT_EVENT_KEY_DOWN,           // Key pressed
    INPUT_EVENT_KEY_UP,             // Key released
    INPUT_EVENT_KEY_REPEAT,         // Key repeat
    INPUT_EVENT_MOUSE_MOVE,         // Mouse movement
    INPUT_EVENT_MOUSE_BUTTON_DOWN,  // Mouse button pressed
    INPUT_EVENT_MOUSE_BUTTON_UP,    // Mouse button released
    INPUT_EVENT_MOUSE_WHEEL,        // Mouse wheel
    INPUT_EVENT_TOUCH_DOWN,         // Touch start
    INPUT_EVENT_TOUCH_UP,           // Touch end
    INPUT_EVENT_TOUCH_MOVE,         // Touch move
    INPUT_EVENT_TOUCH_CANCEL,       // Touch cancelled
    INPUT_EVENT_GAMEPAD_BUTTON_DOWN, // Gamepad button pressed
    INPUT_EVENT_GAMEPAD_BUTTON_UP,  // Gamepad button released
    INPUT_EVENT_GAMEPAD_AXIS,       // Gamepad axis movement
    INPUT_EVENT_GAMEPAD_TRIGGER,    // Gamepad trigger
    INPUT_EVENT_PEN_DOWN,           // Pen touch
    INPUT_EVENT_PEN_UP,             // Pen lift
    INPUT_EVENT_PEN_MOVE,           // Pen movement
    INPUT_EVENT_GESTURE,            // Gesture recognized
    INPUT_EVENT_HAPTIC,             // Haptic feedback
    INPUT_EVENT_DEVICE_CONNECT,     // Device connected
    INPUT_EVENT_DEVICE_DISCONNECT,  // Device disconnected
    INPUT_EVENT_CUSTOM              // Custom event
} input_event_type_t;

// Key codes (based on HID usage tables)
typedef enum {
    KEY_NONE = 0,
    KEY_A = 4, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
    KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
    KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    KEY_1 = 30, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
    KEY_ENTER = 40, KEY_ESCAPE, KEY_BACKSPACE, KEY_TAB, KEY_SPACE,
    KEY_MINUS, KEY_EQUAL, KEY_LEFTBRACE, KEY_RIGHTBRACE, KEY_BACKSLASH,
    KEY_SEMICOLON = 51, KEY_APOSTROPHE, KEY_GRAVE, KEY_COMMA, KEY_DOT, KEY_SLASH,
    KEY_CAPSLOCK = 57,
    KEY_F1 = 58, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
    KEY_PRINTSCREEN = 70, KEY_SCROLLLOCK, KEY_PAUSE,
    KEY_INSERT, KEY_HOME, KEY_PAGEUP, KEY_DELETE, KEY_END, KEY_PAGEDOWN,
    KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_UP,
    KEY_NUMLOCK = 83,
    KEY_KP_SLASH, KEY_KP_ASTERISK, KEY_KP_MINUS, KEY_KP_PLUS, KEY_KP_ENTER,
    KEY_KP_1, KEY_KP_2, KEY_KP_3, KEY_KP_4, KEY_KP_5,
    KEY_KP_6, KEY_KP_7, KEY_KP_8, KEY_KP_9, KEY_KP_0, KEY_KP_DOT,
    KEY_LEFTCTRL = 224, KEY_LEFTSHIFT, KEY_LEFTALT, KEY_LEFTMETA,
    KEY_RIGHTCTRL, KEY_RIGHTSHIFT, KEY_RIGHTALT, KEY_RIGHTMETA
} input_key_code_t;

// Mouse button codes
typedef enum {
    MOUSE_BUTTON_LEFT = 0x01,
    MOUSE_BUTTON_RIGHT = 0x02,
    MOUSE_BUTTON_MIDDLE = 0x04,
    MOUSE_BUTTON_4 = 0x08,
    MOUSE_BUTTON_5 = 0x10,
    MOUSE_BUTTON_6 = 0x20,
    MOUSE_BUTTON_7 = 0x40,
    MOUSE_BUTTON_8 = 0x80
} input_mouse_button_t;

// Gamepad button codes (Xbox-style)
typedef enum {
    GAMEPAD_BUTTON_A = 0x0001,
    GAMEPAD_BUTTON_B = 0x0002,
    GAMEPAD_BUTTON_X = 0x0004,
    GAMEPAD_BUTTON_Y = 0x0008,
    GAMEPAD_BUTTON_LB = 0x0010,
    GAMEPAD_BUTTON_RB = 0x0020,
    GAMEPAD_BUTTON_BACK = 0x0040,
    GAMEPAD_BUTTON_START = 0x0080,
    GAMEPAD_BUTTON_LS = 0x0100,  // Left stick click
    GAMEPAD_BUTTON_RS = 0x0200,  // Right stick click
    GAMEPAD_BUTTON_DPAD_UP = 0x0400,
    GAMEPAD_BUTTON_DPAD_DOWN = 0x0800,
    GAMEPAD_BUTTON_DPAD_LEFT = 0x1000,
    GAMEPAD_BUTTON_DPAD_RIGHT = 0x2000,
    GAMEPAD_BUTTON_HOME = 0x4000,
    GAMEPAD_BUTTON_SHARE = 0x8000
} input_gamepad_button_t;

// Gamepad axes
typedef enum {
    GAMEPAD_AXIS_LEFT_X = 0,
    GAMEPAD_AXIS_LEFT_Y,
    GAMEPAD_AXIS_RIGHT_X,
    GAMEPAD_AXIS_RIGHT_Y,
    GAMEPAD_AXIS_LEFT_TRIGGER,
    GAMEPAD_AXIS_RIGHT_TRIGGER,
    GAMEPAD_AXIS_DPAD_X,
    GAMEPAD_AXIS_DPAD_Y
} input_gamepad_axis_t;

// Touch point states
typedef enum {
    TOUCH_STATE_UP = 0,
    TOUCH_STATE_DOWN,
    TOUCH_STATE_MOVE,
    TOUCH_STATE_CANCEL
} input_touch_state_t;

// Gesture types
typedef enum {
    GESTURE_NONE = 0,
    GESTURE_TAP,
    GESTURE_DOUBLE_TAP,
    GESTURE_LONG_PRESS,
    GESTURE_SWIPE_LEFT,
    GESTURE_SWIPE_RIGHT,
    GESTURE_SWIPE_UP,
    GESTURE_SWIPE_DOWN,
    GESTURE_PINCH_IN,
    GESTURE_PINCH_OUT,
    GESTURE_ROTATE,
    GESTURE_THREE_FINGER_TAP,
    GESTURE_FOUR_FINGER_TAP,
    GESTURE_EDGE_SWIPE,
    GESTURE_CUSTOM
} input_gesture_type_t;

// Forward declarations
typedef struct input_device input_device_t;
typedef struct input_event input_event_t;
typedef struct input_touch_point input_touch_point_t;
typedef struct input_gamepad_state input_gamepad_state_t;
typedef struct input_manager input_manager_t;

// Touch point structure
struct input_touch_point {
    uint32_t id;                    // Touch point ID
    input_touch_state_t state;      // Touch state
    int32_t x, y;                   // Position
    int32_t pressure;               // Pressure (0-1023)
    int32_t width, height;          // Contact area
    int32_t orientation;            // Orientation angle
    uint64_t timestamp;             // Timestamp
};

// Gamepad state structure
struct input_gamepad_state {
    uint32_t buttons;               // Button state bitmask
    int16_t axes[INPUT_MAX_AXES];   // Axis values (-32768 to 32767)
    uint8_t triggers[2];            // Trigger values (0-255)
    bool connected;                 // Connection state
    uint32_t packet_number;         // Packet number for change detection
    
    // Haptic feedback
    struct {
        uint16_t left_motor;        // Left motor speed (0-65535)
        uint16_t right_motor;       // Right motor speed (0-65535)
        uint32_t duration;          // Duration in milliseconds
    } haptic;
};

// Keyboard event data
typedef struct {
    input_key_code_t key_code;      // Key code
    uint32_t scan_code;             // Scan code
    uint32_t unicode;               // Unicode character
    uint32_t modifiers;             // Modifier keys
    bool is_repeat;                 // Key repeat
    uint64_t timestamp;             // Timestamp
} input_keyboard_event_t;

// Mouse event data
typedef struct {
    int32_t x, y;                   // Absolute position
    int32_t delta_x, delta_y;       // Relative movement
    int32_t wheel_delta;            // Wheel delta
    uint32_t buttons;               // Button state
    uint64_t timestamp;             // Timestamp
} input_mouse_event_t;

// Touch event data
typedef struct {
    input_touch_point_t points[INPUT_MAX_TOUCH_POINTS];
    uint32_t point_count;           // Number of active touch points
    uint64_t timestamp;             // Timestamp
} input_touch_event_t;

// Pen/stylus event data
typedef struct {
    int32_t x, y;                   // Position
    int32_t pressure;               // Pressure (0-8191)
    int32_t tilt_x, tilt_y;         // Tilt angles
    int32_t twist;                  // Barrel rotation
    uint32_t buttons;               // Pen buttons
    bool in_range;                  // Pen in proximity
    bool touching;                  // Pen touching surface
    bool eraser;                    // Eraser mode
    uint64_t timestamp;             // Timestamp
} input_pen_event_t;

// Gesture event data
typedef struct {
    input_gesture_type_t type;      // Gesture type
    int32_t x, y;                   // Gesture center
    int32_t width, height;          // Gesture area
    float scale;                    // Scale factor (for pinch)
    float rotation;                 // Rotation angle (for rotate)
    int32_t velocity_x, velocity_y; // Velocity (for swipe)
    uint32_t finger_count;          // Number of fingers
    uint64_t timestamp;             // Timestamp
} input_gesture_event_t;

// Gamepad event data
typedef struct {
    uint32_t controller_id;         // Controller ID
    input_gamepad_state_t state;    // Controller state
    uint64_t timestamp;             // Timestamp
} input_gamepad_event_t;

// Generic input event structure
struct input_event {
    input_event_type_t type;        // Event type
    input_device_t* device;         // Source device
    uint32_t sequence;              // Event sequence number
    
    union {
        input_keyboard_event_t keyboard;
        input_mouse_event_t mouse;
        input_touch_event_t touch;
        input_pen_event_t pen;
        input_gesture_event_t gesture;
        input_gamepad_event_t gamepad;
        uint8_t raw_data[256];      // Raw data for custom events
    } data;
};

// Input device capabilities
typedef struct {
    // Basic capabilities
    bool has_keys;                  // Has keyboard keys
    bool has_buttons;               // Has mouse/gamepad buttons
    bool has_axes;                  // Has analog axes
    bool has_wheel;                 // Has scroll wheel
    bool has_touch;                 // Has touch capability
    bool has_multitouch;            // Has multi-touch
    bool has_pen;                   // Has pen/stylus support
    bool has_haptic;                // Has haptic feedback
    bool has_force_feedback;        // Has force feedback
    bool has_gyroscope;             // Has gyroscope
    bool has_accelerometer;         // Has accelerometer
    
    // Specific capabilities
    uint32_t max_touch_points;      // Maximum touch points
    uint32_t max_pressure;          // Maximum pressure level
    uint32_t max_x, max_y;          // Maximum coordinates
    uint32_t resolution_x, resolution_y; // Resolution (DPI)
    uint32_t polling_rate;          // Polling rate (Hz)
    uint32_t report_rate;           // Report rate (Hz)
    
    // Gaming-specific
    bool has_rgb_lighting;          // Has RGB lighting
    bool has_programmable_buttons;  // Has programmable buttons
    bool has_adjustable_weight;     // Has adjustable weight
    bool has_wireless;              // Supports wireless
    uint32_t battery_level;         // Battery level (0-100, 255 = N/A)
} input_device_caps_t;

// Input device structure
struct input_device {
    // Device identification
    uint32_t id;                    // Device ID
    input_device_type_t type;       // Device type
    char name[128];                 // Device name
    char manufacturer[64];          // Manufacturer
    char serial[64];                // Serial number
    uint16_t vendor_id;             // Vendor ID
    uint16_t product_id;            // Product ID
    uint16_t version;               // Version
    
    // Device capabilities
    input_device_caps_t caps;       // Capabilities
    
    // Device state
    bool connected;                 // Connection state
    bool enabled;                   // Enabled state
    bool suspended;                 // Suspended state
    uint64_t last_activity;         // Last activity timestamp
    
    // Hardware interface
    usb_device_t* usb_device;       // USB device (if USB)
    void* hw_private;               // Hardware-specific data
    
    // Input processing
    struct {
        uint32_t polling_rate;      // Current polling rate
        uint32_t report_rate;       // Current report rate
        bool low_latency_mode;      // Gaming mode
        bool raw_input_mode;        // Raw input mode
        uint32_t deadzone[INPUT_MAX_AXES]; // Axis deadzones
        int32_t calibration[INPUT_MAX_AXES]; // Axis calibration
    } config;
    
    // Event handling
    struct {
        input_event_t* queue;       // Event queue
        uint32_t queue_head;        // Queue head
        uint32_t queue_tail;        // Queue tail
        uint32_t queue_size;        // Queue size
        void* queue_lock;           // Queue lock
        uint32_t dropped_events;    // Dropped event count
    } events;
    
    // Statistics
    struct {
        uint64_t events_processed;
        uint64_t events_dropped;
        uint64_t avg_latency_us;
        uint32_t error_count;
    } stats;
    
    // Device operations
    struct {
        int (*open)(input_device_t* dev);
        int (*close)(input_device_t* dev);
        int (*read)(input_device_t* dev, input_event_t* event);
        int (*write)(input_device_t* dev, const void* data, size_t len);
        int (*ioctl)(input_device_t* dev, uint32_t cmd, void* arg);
        int (*set_config)(input_device_t* dev, const void* config);
        int (*get_config)(input_device_t* dev, void* config);
        int (*calibrate)(input_device_t* dev);
        int (*set_haptic)(input_device_t* dev, const void* haptic_data);
    } ops;
    
    // Driver framework integration
    device_t* device_obj;
    driver_t* driver;
    
    // Synchronization
    void* lock;
    
    // Linked list
    input_device_t* next;
};

// Input manager structure
struct input_manager {
    // Device management
    input_device_t* devices;
    uint32_t device_count;
    uint32_t next_device_id;
    
    // Event processing
    struct {
        input_event_t* global_queue;
        uint32_t queue_head;
        uint32_t queue_tail;
        uint32_t queue_size;
        void* queue_lock;
        void* processing_thread;
        bool processing_enabled;
    } events;
    
    // Gaming optimizations
    struct {
        bool gaming_mode;           // Gaming mode enabled
        uint32_t target_latency_us; // Target latency
        bool exclusive_mode;        // Exclusive mode
        uint32_t priority_boost;    // Priority boost
    } gaming;
    
    // Gesture recognition
    struct {
        bool enabled;               // Gesture recognition enabled
        uint32_t min_gesture_time;  // Minimum gesture time
        uint32_t max_gesture_time;  // Maximum gesture time
        float min_swipe_distance;   // Minimum swipe distance
        void* recognizer_state;     // Recognizer state
    } gestures;
    
    // Hot-plug support
    struct {
        bool enabled;               // Hot-plug enabled
        void* monitor_thread;       // Monitor thread
        void (*callback)(input_device_t* dev, bool connected);
    } hotplug;
    
    // Statistics
    struct {
        uint64_t total_events;
        uint64_t total_devices;
        uint32_t peak_device_count;
        uint64_t avg_processing_time_us;
    } stats;
    
    // Synchronization
    void* lock;
};

// Function prototypes

// Core initialization and cleanup
int input_init(void);
void input_cleanup(void);

// Device management
int input_register_device(input_device_t* device);
int input_unregister_device(input_device_t* device);
input_device_t* input_find_device(uint32_t device_id);
input_device_t* input_find_device_by_name(const char* name);
int input_enable_device(input_device_t* device);
int input_disable_device(input_device_t* device);

// Event processing
int input_read_event(input_event_t* event);
int input_peek_event(input_event_t* event);
int input_post_event(input_device_t* device, const input_event_t* event);
int input_flush_events(input_device_t* device);

// Device enumeration
int input_enumerate_devices(input_device_t** devices, uint32_t* count);
int input_get_device_count_by_type(input_device_type_t type);

// Configuration and calibration
int input_set_polling_rate(input_device_t* device, uint32_t rate);
int input_get_polling_rate(input_device_t* device, uint32_t* rate);
int input_set_gaming_mode(input_device_t* device, bool enable);
int input_calibrate_device(input_device_t* device);
int input_set_deadzone(input_device_t* device, uint32_t axis, uint32_t deadzone);

// Gaming optimizations
int input_enable_gaming_mode(bool enable);
int input_set_target_latency(uint32_t latency_us);
int input_enable_exclusive_mode(input_device_t* device, bool enable);
int input_boost_priority(input_device_t* device, uint32_t boost);

// Multi-touch and gestures
int input_enable_gestures(bool enable);
int input_configure_gesture_recognition(uint32_t min_time, uint32_t max_time, float min_distance);
int input_get_touch_points(input_device_t* device, input_touch_point_t* points, uint32_t* count);

// Haptic feedback
int input_set_haptic_feedback(input_device_t* device, uint16_t left_motor, uint16_t right_motor, uint32_t duration);
int input_set_force_feedback(input_device_t* device, const void* effect_data);
int input_enable_rumble(input_device_t* device, bool enable);

// Keyboard-specific functions
int input_get_key_state(input_device_t* device, input_key_code_t key);
int input_set_key_repeat(input_device_t* device, uint32_t initial_delay, uint32_t repeat_rate);
int input_enable_n_key_rollover(input_device_t* device, bool enable);

// Mouse-specific functions
int input_get_mouse_position(input_device_t* device, int32_t* x, int32_t* y);
int input_set_mouse_sensitivity(input_device_t* device, float sensitivity);
int input_set_mouse_acceleration(input_device_t* device, float acceleration);
int input_enable_mouse_raw_input(input_device_t* device, bool enable);

// Gamepad-specific functions
int input_get_gamepad_state(input_device_t* device, input_gamepad_state_t* state);
int input_set_gamepad_deadzone(input_device_t* device, input_gamepad_axis_t axis, uint16_t deadzone);
int input_enable_gamepad_vibration(input_device_t* device, bool enable);
int input_set_gamepad_led(input_device_t* device, uint32_t color);

// Hot-plug support
int input_enable_hotplug(bool enable);
int input_set_hotplug_callback(void (*callback)(input_device_t* dev, bool connected));
int input_scan_for_devices(void);

// Power management
int input_suspend_device(input_device_t* device);
int input_resume_device(input_device_t* device);
int input_set_power_timeout(input_device_t* device, uint32_t timeout_ms);

// RGB lighting (gaming devices)
int input_set_rgb_lighting(input_device_t* device, uint32_t color, uint32_t effect);
int input_set_rgb_profile(input_device_t* device, const void* profile_data);

// Performance monitoring
int input_get_latency_stats(input_device_t* device, uint64_t* avg_latency, uint64_t* max_latency);
int input_get_throughput_stats(input_device_t* device, uint64_t* events_per_second);
int input_reset_stats(input_device_t* device);

// Utility functions
const char* input_device_type_to_string(input_device_type_t type);
const char* input_event_type_to_string(input_event_type_t type);
const char* input_key_to_string(input_key_code_t key);
bool input_is_gaming_device(input_device_t* device);
bool input_supports_feature(input_device_t* device, uint32_t feature);

// Raw input access (for advanced applications)
int input_enable_raw_input(input_device_t* device, bool enable);
int input_read_raw_data(input_device_t* device, void* buffer, size_t* length);

// Device-specific drivers
int input_usb_hid_probe(usb_device_t* usb_dev);
int input_ps5_controller_init(input_device_t* device);
int input_xbox_controller_init(input_device_t* device);
int input_nintendo_controller_init(input_device_t* device);
int input_racing_wheel_init(input_device_t* device);
int input_graphics_tablet_init(input_device_t* device);

// Legacy compatibility functions
void input_init_legacy(void);
int input_read_event_legacy(input_event_t* event);

// IOCTL commands
#define INPUT_IOCTL_GET_CAPS        0x1001
#define INPUT_IOCTL_SET_CONFIG      0x1002
#define INPUT_IOCTL_GET_CONFIG      0x1003
#define INPUT_IOCTL_CALIBRATE       0x1004
#define INPUT_IOCTL_SET_HAPTIC      0x1005
#define INPUT_IOCTL_GET_STATS       0x1006
#define INPUT_IOCTL_RESET_STATS     0x1007
#define INPUT_IOCTL_SET_RGB         0x1008
#define INPUT_IOCTL_RAW_MODE        0x1009

// Feature flags
#define INPUT_FEATURE_HAPTIC        0x0001
#define INPUT_FEATURE_FORCE_FEEDBACK 0x0002
#define INPUT_FEATURE_RGB_LIGHTING  0x0004
#define INPUT_FEATURE_WIRELESS      0x0008
#define INPUT_FEATURE_GYROSCOPE     0x0010
#define INPUT_FEATURE_ACCELEROMETER 0x0020
#define INPUT_FEATURE_TOUCH         0x0040
#define INPUT_FEATURE_PRESSURE      0x0080
#define INPUT_FEATURE_TILT          0x0100
#define INPUT_FEATURE_ROTATION      0x0200

// Error codes
#define INPUT_SUCCESS              0
#define INPUT_ERR_NO_DEVICE       -6001
#define INPUT_ERR_NO_MEMORY       -6002
#define INPUT_ERR_TIMEOUT         -6003
#define INPUT_ERR_NOT_SUPPORTED   -6004
#define INPUT_ERR_BUSY            -6005
#define INPUT_ERR_CONFIG          -6006
#define INPUT_ERR_CALIBRATION     -6007
#define INPUT_ERR_QUEUE_FULL      -6008
#define INPUT_ERR_INVALID_PARAM   -6009
#define INPUT_ERR_HARDWARE        -6010

#ifdef __cplusplus
}
#endif

#endif // INPUT_H
