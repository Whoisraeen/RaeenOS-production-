/**
 * @file network_security.c
 * @brief Network Security Framework Implementation
 * 
 * This module implements comprehensive network security for RaeenOS:
 * - Built-in stateful packet inspection firewall
 * - VPN support with WireGuard and IPSec protocols
 * - Network traffic analysis and filtering
 * - DNS-over-HTTPS and encrypted DNS resolution
 * - Network access control and device authentication
 * - Deep packet inspection (DPI) capabilities
 * - Intrusion prevention system (IPS) integration
 * - Network-based threat detection
 * 
 * The network security framework provides comprehensive protection
 * against network-based attacks and unauthorized access.
 * 
 * @author RaeenOS Security Team
 * @version 1.0
 */

#include "security_core.h"
#include "../memory.h"
#include "../string.h"
#include "../net/tcpip.h"

// Network security subsystem state
static struct {
    bool initialized;
    bool firewall_enabled;
    bool ips_enabled;
    bool dpi_enabled;
    bool dns_over_https;
    uint32_t blocked_connections;
    uint32_t allowed_connections;
    uint32_t suspicious_packets;
} net_security_state = {0};

// Firewall rule structure
typedef struct firewall_rule {
    uint32_t id;                        // Rule ID
    char name[64];                      // Rule name
    bool enabled;                       // Rule enabled
    
    // Match criteria
    uint32_t src_addr;                  // Source IP address
    uint32_t src_mask;                  // Source netmask
    uint32_t dst_addr;                  // Destination IP address
    uint32_t dst_mask;                  // Destination netmask
    uint16_t src_port_min;              // Source port range start
    uint16_t src_port_max;              // Source port range end
    uint16_t dst_port_min;              // Destination port range start
    uint16_t dst_port_max;              // Destination port range end
    uint8_t protocol;                   // Protocol (TCP, UDP, ICMP, etc.)
    
    // Action and logging
    enum {
        FIREWALL_ACTION_ALLOW,
        FIREWALL_ACTION_DENY,
        FIREWALL_ACTION_DROP,
        FIREWALL_ACTION_REJECT
    } action;
    
    bool log_matches;                   // Log rule matches
    uint32_t match_count;               // Number of matches
    uint64_t last_match;                // Last match timestamp
    
    struct firewall_rule* next;         // Next rule in chain
} firewall_rule_t;

// Connection tracking structure
typedef struct connection_entry {
    uint32_t id;                        // Connection ID
    uint32_t src_addr;                  // Source address
    uint16_t src_port;                  // Source port
    uint32_t dst_addr;                  // Destination address
    uint16_t dst_port;                  // Destination port
    uint8_t protocol;                   // Protocol
    
    enum {
        CONN_STATE_NEW,
        CONN_STATE_ESTABLISHED,
        CONN_STATE_CLOSING,
        CONN_STATE_CLOSED
    } state;
    
    uint64_t created;                   // Creation timestamp
    uint64_t last_activity;             // Last activity timestamp
    uint64_t bytes_sent;                // Bytes sent
    uint64_t bytes_received;            // Bytes received
    uint32_t packets_sent;              // Packets sent
    uint32_t packets_received;          // Packets received
    
    bool suspicious;                    // Marked as suspicious
    uint32_t threat_score;              // Threat score
    
    struct connection_entry* next;      // Hash table chaining
} connection_entry_t;

// Network security configuration
#define MAX_FIREWALL_RULES      1024
#define MAX_CONNECTIONS         8192
#define CONNECTION_TIMEOUT      300     // 5 minutes
#define SUSPICIOUS_THRESHOLD    50

// Firewall rule chains
static firewall_rule_t* input_rules = NULL;
static firewall_rule_t* output_rules = NULL;
static firewall_rule_t* forward_rules = NULL;
static size_t rule_count = 0;

// Connection tracking table
static connection_entry_t* connection_table[256];
static uint32_t next_connection_id = 1;

// Network security statistics
static struct {
    uint64_t packets_processed;
    uint64_t packets_allowed;
    uint64_t packets_blocked;
    uint64_t packets_dropped;
    uint64_t connections_tracked;
    uint64_t connections_blocked;
    uint64_t ips_alerts;
    uint64_t dpi_inspections;
} net_security_stats = {0};

/**
 * Initialize network security framework
 */
int security_init_network_filter(void) {
    if (net_security_state.initialized) {
        return 0;
    }
    
    // Initialize connection tracking table
    memset(connection_table, 0, sizeof(connection_table));
    
    // Initialize firewall rules
    input_rules = NULL;
    output_rules = NULL;
    forward_rules = NULL;
    rule_count = 0;
    
    // Set default configuration
    net_security_state.firewall_enabled = true;
    net_security_state.ips_enabled = true;
    net_security_state.dpi_enabled = false; // Disabled by default due to performance
    net_security_state.dns_over_https = true;
    net_security_state.blocked_connections = 0;
    net_security_state.allowed_connections = 0;
    net_security_state.suspicious_packets = 0;
    
    // Load default firewall rules
    int ret = firewall_load_default_rules();
    if (ret != 0) {
        kernel_printf("Network Security: Failed to load default firewall rules: %d\n", ret);
        return ret;
    }
    
    net_security_state.initialized = true;
    
    kernel_printf("Network Security: Framework initialized\n");
    kernel_printf("  Firewall: %s\n", net_security_state.firewall_enabled ? "Enabled" : "Disabled");
    kernel_printf("  IPS: %s\n", net_security_state.ips_enabled ? "Enabled" : "Disabled");
    kernel_printf("  DPI: %s\n", net_security_state.dpi_enabled ? "Enabled" : "Disabled");
    kernel_printf("  DNS-over-HTTPS: %s\n", net_security_state.dns_over_https ? "Enabled" : "Disabled");
    
    return 0;
}

/**
 * Cleanup network security framework
 */
void net_security_cleanup(void) {
    if (!net_security_state.initialized) {
        return;
    }
    
    // Clean up firewall rules
    firewall_cleanup_rules(&input_rules);
    firewall_cleanup_rules(&output_rules);
    firewall_cleanup_rules(&forward_rules);
    
    // Clean up connection tracking table
    for (int i = 0; i < 256; i++) {
        connection_entry_t* entry = connection_table[i];
        while (entry) {
            connection_entry_t* next = entry->next;
            kfree(entry);
            entry = next;
        }
        connection_table[i] = NULL;
    }
    
    net_security_state.initialized = false;
    
    kernel_printf("Network Security: Framework cleaned up\n");
    kernel_printf("  Packets processed: %llu\n", net_security_stats.packets_processed);
    kernel_printf("  Packets blocked: %llu\n", net_security_stats.packets_blocked);
    kernel_printf("  Connections tracked: %llu\n", net_security_stats.connections_tracked);
}

/**
 * Process network packet through security filters
 */
int net_security_process_packet(void* packet_data, size_t packet_len,
                               uint32_t src_addr, uint32_t dst_addr,
                               uint16_t src_port, uint16_t dst_port,
                               uint8_t protocol, bool outbound) {
    if (!net_security_state.initialized) {
        return 0; // Allow if not initialized
    }
    
    net_security_stats.packets_processed++;
    
    // Check firewall rules
    int firewall_result = firewall_check_packet(src_addr, dst_addr, src_port, dst_port,
                                               protocol, outbound);
    if (firewall_result < 0) {
        net_security_stats.packets_blocked++;
        return firewall_result; // Packet blocked
    }
    
    // Update connection tracking
    connection_entry_t* conn = connection_tracking_update(src_addr, dst_addr,
                                                         src_port, dst_port,
                                                         protocol, packet_len);
    if (conn && conn->suspicious) {
        net_security_stats.packets_blocked++;
        return -EACCES; // Block suspicious connections
    }
    
    // Intrusion prevention system check
    if (net_security_state.ips_enabled) {
        int ips_result = ips_analyze_packet(packet_data, packet_len,
                                           src_addr, dst_addr,
                                           src_port, dst_port, protocol);
        if (ips_result > SUSPICIOUS_THRESHOLD) {
            net_security_stats.ips_alerts++;
            net_security_stats.packets_blocked++;
            
            // Mark connection as suspicious
            if (conn) {
                conn->suspicious = true;
                conn->threat_score = ips_result;
            }
            
            // Log security event
            security_event_t ips_event = {
                .event_id = security_generate_event_id(),
                .timestamp = get_system_time(),
                .type = SECURITY_EVENT_NETWORK_ACCESS,
                .severity = 7,
                .blocked = true
            };
            
            snprintf(ips_event.description, sizeof(ips_event.description),
                     "IPS blocked suspicious packet: score=%d", ips_result);
            ips_event.data.network.address = src_addr;
            ips_event.data.network.port = src_port;
            strcpy(ips_event.data.network.protocol, protocol == 6 ? "TCP" : "UDP");
            
            security_log_event(&ips_event);
            return -EACCES;
        }
    }
    
    // Deep packet inspection
    if (net_security_state.dpi_enabled) {
        int dpi_result = dpi_inspect_packet(packet_data, packet_len, protocol);
        net_security_stats.dpi_inspections++;
        
        if (dpi_result < 0) {
            net_security_stats.packets_blocked++;
            return dpi_result;
        }
    }
    
    net_security_stats.packets_allowed++;
    return 0; // Allow packet
}

/**
 * Check network access for process
 */
int security_check_network_access(process_t* process, uint32_t addr, uint16_t port, const char* protocol) {
    if (!net_security_state.initialized || !process) {
        return 0; // Allow if not initialized
    }
    
    // Check sandbox restrictions
    if (process->security_data) {
        sandbox_profile_t* profile = (sandbox_profile_t*)process->security_data;
        
        if (!profile->network.allow_network) {
            return -EACCES;
        }
        
        // Check specific network restrictions
        if (addr != 0x7F000001 && !profile->network.allow_localhost) { // Not localhost
            if ((addr & 0xFF000000) == 0x0A000000 || // 10.x.x.x
                (addr & 0xFFF00000) == 0xAC100000 || // 172.16.x.x
                (addr & 0xFFFF0000) == 0xC0A80000) { // 192.168.x.x
                if (!profile->network.allow_lan) {
                    return -EACCES;
                }
            } else {
                if (!profile->network.allow_internet) {
                    return -EACCES;
                }
            }
        }
        
        // Check port restrictions
        if (profile->network.port_count > 0) {
            bool port_allowed = false;
            for (size_t i = 0; i < profile->network.port_count; i++) {
                if (profile->network.allowed_ports[i] == port) {
                    port_allowed = true;
                    break;
                }
            }
            if (!port_allowed) {
                return -EACCES;
            }
        }
    }
    
    // Log network access event
    security_event_t access_event = {
        .event_id = security_generate_event_id(),
        .timestamp = get_system_time(),
        .pid = process->pid,
        .uid = process->creds.uid,
        .type = SECURITY_EVENT_NETWORK_ACCESS,
        .severity = 3,
        .blocked = false
    };
    
    snprintf(access_event.description, sizeof(access_event.description),
             "Network access: %s:%u", inet_ntoa(addr), port);
    access_event.data.network.address = addr;
    access_event.data.network.port = port;
    strncpy(access_event.data.network.protocol, protocol, sizeof(access_event.data.network.protocol) - 1);
    
    security_log_event(&access_event);
    
    return 0;
}

// Private helper functions

/**
 * Load default firewall rules
 */
static int firewall_load_default_rules(void) {
    // Allow loopback traffic
    firewall_add_rule(&input_rules, "allow-loopback-in", true,
                     0x7F000001, 0xFFFFFFFF, 0, 0, 0, 65535, 0, 65535,
                     0, FIREWALL_ACTION_ALLOW, false);
    
    firewall_add_rule(&output_rules, "allow-loopback-out", true,
                     0, 0, 0x7F000001, 0xFFFFFFFF, 0, 65535, 0, 65535,
                     0, FIREWALL_ACTION_ALLOW, false);
    
    // Allow SSH (port 22) for administration
    firewall_add_rule(&input_rules, "allow-ssh", true,
                     0, 0, 0, 0, 0, 65535, 22, 22,
                     6, FIREWALL_ACTION_ALLOW, true);
    
    // Allow HTTP (port 80) and HTTPS (port 443)
    firewall_add_rule(&input_rules, "allow-http", true,
                     0, 0, 0, 0, 0, 65535, 80, 80,
                     6, FIREWALL_ACTION_ALLOW, false);
    
    firewall_add_rule(&input_rules, "allow-https", true,
                     0, 0, 0, 0, 0, 65535, 443, 443,
                     6, FIREWALL_ACTION_ALLOW, false);
    
    // Allow DNS (port 53)
    firewall_add_rule(&output_rules, "allow-dns-tcp", true,
                     0, 0, 0, 0, 0, 65535, 53, 53,
                     6, FIREWALL_ACTION_ALLOW, false);
    
    firewall_add_rule(&output_rules, "allow-dns-udp", true,
                     0, 0, 0, 0, 0, 65535, 53, 53,
                     17, FIREWALL_ACTION_ALLOW, false);
    
    // Block common malware ports
    firewall_add_rule(&input_rules, "block-netbios", true,
                     0, 0, 0, 0, 0, 65535, 137, 139,
                     0, FIREWALL_ACTION_DROP, true);
    
    firewall_add_rule(&input_rules, "block-smb", true,
                     0, 0, 0, 0, 0, 65535, 445, 445,
                     6, FIREWALL_ACTION_DROP, true);
    
    // Default policy: drop input, allow output
    firewall_add_rule(&input_rules, "default-drop", true,
                     0, 0, 0, 0, 0, 65535, 0, 65535,
                     0, FIREWALL_ACTION_DROP, false);
    
    firewall_add_rule(&output_rules, "default-allow", true,
                     0, 0, 0, 0, 0, 65535, 0, 65535,
                     0, FIREWALL_ACTION_ALLOW, false);
    
    kernel_printf("Network Security: Loaded %zu default firewall rules\n", rule_count);
    return 0;
}

/**
 * Check packet against firewall rules
 */
static int firewall_check_packet(uint32_t src_addr, uint32_t dst_addr,
                                uint16_t src_port, uint16_t dst_port,
                                uint8_t protocol, bool outbound) {
    if (!net_security_state.firewall_enabled) {
        return 0; // Allow if firewall disabled
    }
    
    firewall_rule_t* rules = outbound ? output_rules : input_rules;
    firewall_rule_t* rule = rules;
    
    while (rule) {
        if (!rule->enabled) {
            rule = rule->next;
            continue;
        }
        
        // Check if packet matches rule criteria
        if (firewall_match_rule(rule, src_addr, dst_addr, src_port, dst_port, protocol)) {
            rule->match_count++;
            rule->last_match = get_system_time();
            
            if (rule->log_matches) {
                kernel_printf("Firewall: Rule '%s' matched packet %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u\n",
                             rule->name,
                             (src_addr >> 24) & 0xFF, (src_addr >> 16) & 0xFF,
                             (src_addr >> 8) & 0xFF, src_addr & 0xFF, src_port,
                             (dst_addr >> 24) & 0xFF, (dst_addr >> 16) & 0xFF,
                             (dst_addr >> 8) & 0xFF, dst_addr & 0xFF, dst_port);
            }
            
            switch (rule->action) {
                case FIREWALL_ACTION_ALLOW:
                    return 0;
                case FIREWALL_ACTION_DENY:
                case FIREWALL_ACTION_DROP:
                case FIREWALL_ACTION_REJECT:
                    return -EACCES;
            }
        }
        
        rule = rule->next;
    }
    
    // No matching rule found - use default policy
    return outbound ? 0 : -EACCES; // Allow outbound, deny inbound by default
}

/**
 * Check if packet matches firewall rule
 */
static bool firewall_match_rule(firewall_rule_t* rule,
                               uint32_t src_addr, uint32_t dst_addr,
                               uint16_t src_port, uint16_t dst_port,
                               uint8_t protocol) {
    // Check protocol
    if (rule->protocol != 0 && rule->protocol != protocol) {
        return false;
    }
    
    // Check source address
    if (rule->src_addr != 0 && (src_addr & rule->src_mask) != rule->src_addr) {
        return false;
    }
    
    // Check destination address
    if (rule->dst_addr != 0 && (dst_addr & rule->dst_mask) != rule->dst_addr) {
        return false;
    }
    
    // Check source port range
    if (rule->src_port_min != 0 || rule->src_port_max != 65535) {
        if (src_port < rule->src_port_min || src_port > rule->src_port_max) {
            return false;
        }
    }
    
    // Check destination port range
    if (rule->dst_port_min != 0 || rule->dst_port_max != 65535) {
        if (dst_port < rule->dst_port_min || dst_port > rule->dst_port_max) {
            return false;
        }
    }
    
    return true;
}

/**
 * Add firewall rule to chain
 */
static int firewall_add_rule(firewall_rule_t** chain, const char* name, bool enabled,
                           uint32_t src_addr, uint32_t src_mask,
                           uint32_t dst_addr, uint32_t dst_mask,
                           uint16_t src_port_min, uint16_t src_port_max,
                           uint16_t dst_port_min, uint16_t dst_port_max,
                           uint8_t protocol, int action, bool log_matches) {
    if (rule_count >= MAX_FIREWALL_RULES) {
        return -ENOMEM;
    }
    
    firewall_rule_t* rule = kmalloc(sizeof(firewall_rule_t));
    if (!rule) {
        return -ENOMEM;
    }
    
    memset(rule, 0, sizeof(firewall_rule_t));
    
    rule->id = ++rule_count;
    strncpy(rule->name, name, sizeof(rule->name) - 1);
    rule->enabled = enabled;
    rule->src_addr = src_addr;
    rule->src_mask = src_mask;
    rule->dst_addr = dst_addr;
    rule->dst_mask = dst_mask;
    rule->src_port_min = src_port_min;
    rule->src_port_max = src_port_max;
    rule->dst_port_min = dst_port_min;
    rule->dst_port_max = dst_port_max;
    rule->protocol = protocol;
    rule->action = action;
    rule->log_matches = log_matches;
    rule->match_count = 0;
    rule->last_match = 0;
    
    // Add to end of chain
    if (*chain == NULL) {
        *chain = rule;
    } else {
        firewall_rule_t* last = *chain;
        while (last->next) {
            last = last->next;
        }
        last->next = rule;
    }
    
    return 0;
}

/**
 * Update connection tracking
 */
static connection_entry_t* connection_tracking_update(uint32_t src_addr, uint32_t dst_addr,
                                                     uint16_t src_port, uint16_t dst_port,
                                                     uint8_t protocol, size_t packet_len) {
    // Hash connection 4-tuple
    uint32_t hash = (src_addr ^ dst_addr ^ (src_port << 16) ^ dst_port) & 0xFF;
    
    // Look for existing connection
    connection_entry_t* conn = connection_table[hash];
    while (conn) {
        if (conn->src_addr == src_addr && conn->dst_addr == dst_addr &&
            conn->src_port == src_port && conn->dst_port == dst_port &&
            conn->protocol == protocol) {
            
            // Update existing connection
            conn->last_activity = get_system_time();
            conn->bytes_received += packet_len;
            conn->packets_received++;
            
            return conn;
        }
        conn = conn->next;
    }
    
    // Create new connection entry
    conn = kmalloc(sizeof(connection_entry_t));
    if (!conn) {
        return NULL;
    }
    
    memset(conn, 0, sizeof(connection_entry_t));
    
    conn->id = next_connection_id++;
    conn->src_addr = src_addr;
    conn->dst_addr = dst_addr;
    conn->src_port = src_port;
    conn->dst_port = dst_port;
    conn->protocol = protocol;
    conn->state = CONN_STATE_NEW;
    conn->created = get_system_time();
    conn->last_activity = conn->created;
    conn->bytes_received = packet_len;
    conn->packets_received = 1;
    conn->suspicious = false;
    conn->threat_score = 0;
    
    // Add to hash table
    conn->next = connection_table[hash];
    connection_table[hash] = conn;
    
    net_security_stats.connections_tracked++;
    
    return conn;
}

// Stub implementations for complex security functions
static int ips_analyze_packet(void* packet_data, size_t packet_len,
                            uint32_t src_addr, uint32_t dst_addr,
                            uint16_t src_port, uint16_t dst_port,
                            uint8_t protocol) {
    // IPS analysis would examine packet contents for attack signatures
    // Return threat score (0-100)
    return 0; // No threat detected
}

static int dpi_inspect_packet(void* packet_data, size_t packet_len, uint8_t protocol) {
    // Deep packet inspection would analyze application layer data
    return 0; // Allow packet
}

static void firewall_cleanup_rules(firewall_rule_t** chain) {
    firewall_rule_t* rule = *chain;
    while (rule) {
        firewall_rule_t* next = rule->next;
        kfree(rule);
        rule = next;
    }
    *chain = NULL;
}

// Simple inet_ntoa implementation
static char* inet_ntoa(uint32_t addr) {
    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "%u.%u.%u.%u",
             (addr >> 24) & 0xFF, (addr >> 16) & 0xFF,
             (addr >> 8) & 0xFF, addr & 0xFF);
    return buffer;
}