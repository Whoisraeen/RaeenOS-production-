#ifndef USB_H
#define USB_H

#include <stdint.h>

// USB Host Controller Interface (HCI) types
typedef enum {
    USB_HCI_UHCI,
    USB_HCI_OHCI,
    USB_HCI_EHCI,
    USB_HCI_XHCI
} usb_hci_type_t;

// Initialize USB host controllers
void usb_init(void);

#endif // USB_H
