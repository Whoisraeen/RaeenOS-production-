#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>
#include <stdbool.h>

// Initialize the Wi-Fi driver
void wifi_init(void);

// Scan for Wi-Fi networks
int wifi_scan_networks(void);

// Connect to a Wi-Fi network
int wifi_connect(const char* ssid, const char* password);

// Disconnect from Wi-Fi network
int wifi_disconnect(void);

#endif // WIFI_H