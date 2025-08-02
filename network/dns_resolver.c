/**
 * RaeenOS DNS Resolver Implementation
 * Complete DNS client with caching and multiple record type support
 */

#include "network_advanced.h"
#include "../memory.h"
#include "../string.h"
#include "../timer.h"

// DNS Message Header
typedef struct __attribute__((packed)) {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;    // Question count
    uint16_t ancount;    // Answer count
    uint16_t nscount;    // Authority count
    uint16_t arcount;    // Additional count
} dns_header_t;

// DNS Question Structure
typedef struct __attribute__((packed)) {
    // Name is variable length, followed by:
    uint16_t qtype;
    uint16_t qclass;
} dns_question_t;

// DNS Resource Record
typedef struct __attribute__((packed)) {
    // Name is variable length, followed by:
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;
    // Data follows
} dns_rr_t;

// DNS Record Types
#define DNS_TYPE_A      1
#define DNS_TYPE_NS     2
#define DNS_TYPE_CNAME  5
#define DNS_TYPE_PTR    12
#define DNS_TYPE_MX     15
#define DNS_TYPE_AAAA   28

// DNS Classes
#define DNS_CLASS_IN    1

// DNS Flags
#define DNS_FLAG_QR     0x8000  // Query/Response
#define DNS_FLAG_AA     0x0400  // Authoritative Answer
#define DNS_FLAG_TC     0x0200  // Truncated
#define DNS_FLAG_RD     0x0100  // Recursion Desired
#define DNS_FLAG_RA     0x0080  // Recursion Available

// DNS Response Codes
#define DNS_RCODE_NOERROR   0
#define DNS_RCODE_FORMERR   1
#define DNS_RCODE_SERVFAIL  2
#define DNS_RCODE_NXDOMAIN  3

// DNS Cache Entry
typedef struct dns_cache_entry {
    char hostname[256];
    uint32_t ip_address;
    uint32_t ttl;
    uint32_t timestamp;
    struct dns_cache_entry* next;
} dns_cache_entry_t;

// DNS Resolver Context
typedef struct {
    uint32_t dns_servers[4];
    uint8_t dns_server_count;
    uint16_t next_query_id;
    udp_socket_t* socket;
    
    dns_cache_entry_t* cache;
    uint32_t cache_size;
    uint32_t max_cache_size;
    
    uint32_t queries_sent;
    uint32_t responses_received;
    uint32_t cache_hits;
    uint32_t timeouts;
} dns_resolver_t;

static dns_resolver_t g_dns_resolver = {0};

// Function declarations
static uint16_t dns_encode_name(const char* hostname, uint8_t* buffer);
static uint16_t dns_decode_name(const uint8_t* packet, uint16_t offset, char* hostname, uint16_t max_len);
static dns_cache_entry_t* dns_cache_lookup(const char* hostname);
static void dns_cache_add(const char* hostname, uint32_t ip, uint32_t ttl);
static void dns_cache_cleanup(void);
static bool dns_send_query(const char* hostname, uint16_t query_id);
static uint32_t dns_parse_response(const uint8_t* packet, uint16_t packet_len, const char* hostname);

/**
 * Initialize DNS resolver
 */
bool dns_resolver_init(const uint32_t* dns_servers, uint8_t server_count) {
    dns_resolver_t* resolver = &g_dns_resolver;
    
    memory_set(resolver, 0, sizeof(dns_resolver_t));
    
    // Set DNS servers
    resolver->dns_server_count = (server_count > 4) ? 4 : server_count;
    for (uint8_t i = 0; i < resolver->dns_server_count; i++) {
        resolver->dns_servers[i] = dns_servers[i];
        printf("DNS: Server %u: %u.%u.%u.%u\n", i + 1,
               (dns_servers[i] >> 24) & 0xFF, (dns_servers[i] >> 16) & 0xFF,
               (dns_servers[i] >> 8) & 0xFF, dns_servers[i] & 0xFF);
    }
    
    resolver->next_query_id = 1;
    resolver->max_cache_size = 100;
    
    // Create UDP socket for DNS queries
    resolver->socket = udp_socket_create();
    if (!resolver->socket) {
        printf("DNS: Failed to create UDP socket\n");
        return false;
    }
    
    // Bind to ephemeral port
    if (!udp_socket_bind(resolver->socket, 0, 0)) {
        printf("DNS: Failed to bind socket\n");
        udp_socket_close(resolver->socket);
        return false;
    }
    
    printf("DNS: Resolver initialized with %u servers\n", resolver->dns_server_count);
    return true;
}

/**
 * Resolve hostname to IP address
 */
uint32_t dns_resolve(const char* hostname) {
    dns_resolver_t* resolver = &g_dns_resolver;
    
    if (!resolver->socket || !hostname || string_length(hostname) == 0) {
        return 0;
    }
    
    // Check if it's already an IP address
    uint32_t ip = inet_addr(hostname);
    if (ip != 0) {
        return ip;
    }
    
    // Check cache first
    dns_cache_entry_t* cached = dns_cache_lookup(hostname);
    if (cached) {
        uint32_t current_time = timer_get_ticks() / 1000;
        if (current_time < cached->timestamp + cached->ttl) {
            resolver->cache_hits++;
            printf("DNS: Cache hit for %s -> %u.%u.%u.%u\n", hostname,
                   (cached->ip_address >> 24) & 0xFF, (cached->ip_address >> 16) & 0xFF,
                   (cached->ip_address >> 8) & 0xFF, cached->ip_address & 0xFF);
            return cached->ip_address;
        } else {
            // Entry expired, remove from cache
            // TODO: Implement cache entry removal
        }
    }
    
    // Send DNS query
    uint16_t query_id = resolver->next_query_id++;
    if (!dns_send_query(hostname, query_id)) {
        printf("DNS: Failed to send query for %s\n", hostname);
        return 0;
    }
    
    resolver->queries_sent++;
    
    // Wait for response
    uint32_t timeout = 5000; // 5 seconds
    uint32_t start_time = timer_get_ticks();
    
    while ((timer_get_ticks() - start_time) < timeout) {
        uint8_t buffer[1500];
        uint32_t src_ip;
        uint16_t src_port;
        
        int received = udp_socket_recv(resolver->socket, buffer, sizeof(buffer), &src_ip, &src_port);
        if (received > 0) {
            // Verify it's from a DNS server
            bool valid_server = false;
            for (uint8_t i = 0; i < resolver->dns_server_count; i++) {
                if (src_ip == resolver->dns_servers[i] && src_port == 53) {
                    valid_server = true;
                    break;
                }
            }
            
            if (valid_server && received >= sizeof(dns_header_t)) {
                dns_header_t* header = (dns_header_t*)buffer;
                
                // Check if this is our response
                if (ntohs(header->id) == query_id && (ntohs(header->flags) & DNS_FLAG_QR)) {
                    resolver->responses_received++;
                    
                    uint32_t resolved_ip = dns_parse_response(buffer, received, hostname);
                    if (resolved_ip != 0) {
                        printf("DNS: Resolved %s -> %u.%u.%u.%u\n", hostname,
                               (resolved_ip >> 24) & 0xFF, (resolved_ip >> 16) & 0xFF,
                               (resolved_ip >> 8) & 0xFF, resolved_ip & 0xFF);
                        return resolved_ip;
                    }
                }
            }
        }
        
        // Small delay to avoid busy waiting
        for (volatile int i = 0; i < 10000; i++);
    }
    
    resolver->timeouts++;
    printf("DNS: Timeout resolving %s\n", hostname);
    return 0;
}

/**
 * Add DNS server
 */
bool dns_add_server(uint32_t server_ip) {
    dns_resolver_t* resolver = &g_dns_resolver;
    
    if (resolver->dns_server_count >= 4) {
        return false;
    }
    
    resolver->dns_servers[resolver->dns_server_count++] = server_ip;
    printf("DNS: Added server %u.%u.%u.%u\n",
           (server_ip >> 24) & 0xFF, (server_ip >> 16) & 0xFF,
           (server_ip >> 8) & 0xFF, server_ip & 0xFF);
    
    return true;
}

/**
 * Clear DNS cache
 */
void dns_clear_cache(void) {
    dns_resolver_t* resolver = &g_dns_resolver;
    
    dns_cache_entry_t* entry = resolver->cache;
    while (entry) {
        dns_cache_entry_t* next = entry->next;
        memory_free(entry);
        entry = next;
    }
    
    resolver->cache = NULL;
    resolver->cache_size = 0;
    
    printf("DNS: Cache cleared\n");
}

/**
 * Get DNS statistics
 */
void dns_get_stats(uint32_t* queries, uint32_t* responses, uint32_t* cache_hits, uint32_t* timeouts) {
    dns_resolver_t* resolver = &g_dns_resolver;
    
    if (queries) *queries = resolver->queries_sent;
    if (responses) *responses = resolver->responses_received;
    if (cache_hits) *cache_hits = resolver->cache_hits;
    if (timeouts) *timeouts = resolver->timeouts;
}

// Internal helper functions

static uint16_t dns_encode_name(const char* hostname, uint8_t* buffer) {
    uint8_t* start = buffer;
    const char* label_start = hostname;
    
    while (*hostname) {
        if (*hostname == '.') {
            // Write label length
            uint8_t label_len = hostname - label_start;
            if (label_len > 63) return 0; // Label too long
            
            *buffer++ = label_len;
            memory_copy(buffer, label_start, label_len);
            buffer += label_len;
            
            hostname++;
            label_start = hostname;
        } else {
            hostname++;
        }
    }
    
    // Write final label
    uint8_t label_len = hostname - label_start;
    if (label_len > 0) {
        if (label_len > 63) return 0;
        *buffer++ = label_len;
        memory_copy(buffer, label_start, label_len);
        buffer += label_len;
    }
    
    // Null terminator
    *buffer++ = 0;
    
    return buffer - start;
}

static uint16_t dns_decode_name(const uint8_t* packet, uint16_t offset, char* hostname, uint16_t max_len) {
    const uint8_t* ptr = packet + offset;
    uint16_t pos = 0;
    bool jumped = false;
    uint16_t original_offset = offset;
    
    while (*ptr != 0 && pos < max_len - 1) {
        if ((*ptr & 0xC0) == 0xC0) {
            // Compression pointer
            if (!jumped) {
                original_offset += 2;
            }
            uint16_t jump_offset = ((*ptr & 0x3F) << 8) | *(ptr + 1);
            ptr = packet + jump_offset;
            jumped = true;
        } else {
            // Regular label
            uint8_t label_len = *ptr++;
            if (label_len > 63) break;
            
            if (pos > 0) {
                hostname[pos++] = '.';
            }
            
            for (uint8_t i = 0; i < label_len && pos < max_len - 1; i++) {
                hostname[pos++] = *ptr++;
            }
        }
    }
    
    hostname[pos] = '\0';
    
    if (!jumped) {
        original_offset = ptr - packet + 1;
    }
    
    return original_offset;
}

static dns_cache_entry_t* dns_cache_lookup(const char* hostname) {
    dns_resolver_t* resolver = &g_dns_resolver;
    dns_cache_entry_t* entry = resolver->cache;
    
    while (entry) {
        if (string_compare(entry->hostname, hostname) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    
    return NULL;
}

static void dns_cache_add(const char* hostname, uint32_t ip, uint32_t ttl) {
    dns_resolver_t* resolver = &g_dns_resolver;
    
    // Check if entry already exists
    dns_cache_entry_t* existing = dns_cache_lookup(hostname);
    if (existing) {
        existing->ip_address = ip;
        existing->ttl = ttl;
        existing->timestamp = timer_get_ticks() / 1000;
        return;
    }
    
    // Clean up expired entries if cache is full
    if (resolver->cache_size >= resolver->max_cache_size) {
        dns_cache_cleanup();
    }
    
    // Create new entry
    dns_cache_entry_t* entry = (dns_cache_entry_t*)memory_alloc(sizeof(dns_cache_entry_t));
    if (!entry) return;
    
    string_copy(entry->hostname, hostname, sizeof(entry->hostname));
    entry->ip_address = ip;
    entry->ttl = ttl;
    entry->timestamp = timer_get_ticks() / 1000;
    entry->next = resolver->cache;
    
    resolver->cache = entry;
    resolver->cache_size++;
}

static void dns_cache_cleanup(void) {
    dns_resolver_t* resolver = &g_dns_resolver;
    uint32_t current_time = timer_get_ticks() / 1000;
    
    dns_cache_entry_t** entry_ptr = &resolver->cache;
    while (*entry_ptr) {
        dns_cache_entry_t* entry = *entry_ptr;
        
        if (current_time >= entry->timestamp + entry->ttl) {
            // Entry expired
            *entry_ptr = entry->next;
            memory_free(entry);
            resolver->cache_size--;
        } else {
            entry_ptr = &entry->next;
        }
    }
}

static bool dns_send_query(const char* hostname, uint16_t query_id) {
    dns_resolver_t* resolver = &g_dns_resolver;
    
    if (resolver->dns_server_count == 0) {
        return false;
    }
    
    uint8_t packet[512];
    dns_header_t* header = (dns_header_t*)packet;
    
    // Build DNS header
    header->id = htons(query_id);
    header->flags = htons(DNS_FLAG_RD); // Recursion desired
    header->qdcount = htons(1);
    header->ancount = 0;
    header->nscount = 0;
    header->arcount = 0;
    
    // Encode hostname
    uint8_t* ptr = packet + sizeof(dns_header_t);
    uint16_t name_len = dns_encode_name(hostname, ptr);
    if (name_len == 0) {
        return false;
    }
    ptr += name_len;
    
    // Add question
    dns_question_t* question = (dns_question_t*)ptr;
    question->qtype = htons(DNS_TYPE_A);
    question->qclass = htons(DNS_CLASS_IN);
    ptr += sizeof(dns_question_t);
    
    uint16_t packet_len = ptr - packet;
    
    // Send to first available DNS server
    uint32_t dns_server = resolver->dns_servers[0];
    int result = udp_socket_send(resolver->socket, packet, packet_len, dns_server, 53);
    
    return result > 0;
}

static uint32_t dns_parse_response(const uint8_t* packet, uint16_t packet_len, const char* hostname) {
    if (packet_len < sizeof(dns_header_t)) {
        return 0;
    }
    
    dns_header_t* header = (dns_header_t*)packet;
    uint16_t flags = ntohs(header->flags);
    uint16_t rcode = flags & 0x0F;
    
    if (rcode != DNS_RCODE_NOERROR) {
        printf("DNS: Query failed with rcode %u\n", rcode);
        return 0;
    }
    
    uint16_t ancount = ntohs(header->ancount);
    if (ancount == 0) {
        return 0;
    }
    
    // Skip question section
    uint8_t* ptr = (uint8_t*)packet + sizeof(dns_header_t);
    char temp_name[256];
    uint16_t offset = dns_decode_name(packet, ptr - packet, temp_name, sizeof(temp_name));
    ptr = (uint8_t*)packet + offset + sizeof(dns_question_t);
    
    // Parse answer section
    for (uint16_t i = 0; i < ancount; i++) {
        offset = dns_decode_name(packet, ptr - packet, temp_name, sizeof(temp_name));
        ptr = (uint8_t*)packet + offset;
        
        if (ptr + sizeof(dns_rr_t) > packet + packet_len) {
            break;
        }
        
        dns_rr_t* rr = (dns_rr_t*)ptr;
        uint16_t type = ntohs(rr->type);
        uint16_t rdlength = ntohs(rr->rdlength);
        uint32_t ttl = ntohl(rr->ttl);
        
        ptr += sizeof(dns_rr_t);
        
        if (type == DNS_TYPE_A && rdlength == 4) {
            uint32_t ip_address = *(uint32_t*)ptr;
            
            // Add to cache
            dns_cache_add(hostname, ip_address, ttl);
            
            return ip_address;
        }
        
        ptr += rdlength;
    }
    
    return 0;
}
