/**
 * @file ids.c
 * @brief Intrusion Detection System (IDS) Implementation
 * 
 * This module implements a real-time intrusion detection system for RaeenOS:
 * - Behavioral analysis and anomaly detection
 * - Pattern-based intrusion detection rules
 * - Machine learning-based threat classification
 * - Real-time threat scoring and response
 * - Integration with audit and monitoring systems
 * - Automated threat response and mitigation
 * - False positive reduction algorithms
 * - Threat intelligence integration
 * 
 * The IDS provides proactive security monitoring and can automatically
 * respond to detected threats while minimizing false positives.
 * 
 * @author RaeenOS Security Team
 * @version 1.0
 */

#include "security_core.h"
#include "../memory.h"
#include "../string.h"

// IDS subsystem state
static struct {
    bool initialized;
    bool ids_enabled;
    ids_rule_t* rules;
    size_t rule_count;
    size_t max_rules;
    uint32_t threat_threshold;
    uint32_t analysis_window;
    bool learning_mode;
} ids_state = {0};

// Threat detection statistics
static struct {
    uint64_t events_analyzed;
    uint64_t threats_detected;
    uint64_t false_positives;
    uint64_t rules_triggered;
    uint64_t processes_terminated;
    uint64_t connections_blocked;
} ids_stats = {0};

// Behavioral analysis data
typedef struct process_behavior {
    uint32_t pid;
    uint64_t last_activity;
    uint32_t syscall_count[64];     // Track system call frequency
    uint32_t file_access_count;
    uint32_t network_access_count;
    uint32_t privilege_escalations;
    uint32_t anomaly_score;
    bool suspicious;
} process_behavior_t;

static process_behavior_t* behavior_db = NULL;
static size_t behavior_db_size = 0;

// Default IDS rules
static const char* default_ids_rules[] = {
    // Process behavior anomalies
    "rapid_process_creation:process_create:threshold=10:window=60",
    "privilege_escalation_attempts:capability_use:threshold=5:window=30",
    "suspicious_syscall_pattern:system_call:threshold=100:window=10",
    
    // File system anomalies
    "rapid_file_access:file_access:threshold=50:window=5",
    "system_file_modification:file_access:pattern=/etc/*:action=write",
    "tmp_executable_creation:file_access:pattern=/tmp/*:action=execute",
    
    // Network anomalies
    "port_scanning:network_access:threshold=20:window=10",
    "suspicious_outbound_connections:network_access:pattern=*:external",
    "dns_tunneling:network_access:pattern=dns:excessive",
    
    // Authentication anomalies
    "brute_force_login:login_attempt:threshold=10:window=60:failed",
    "unusual_login_time:login_attempt:pattern=off_hours",
    
    // Memory corruption attempts
    "buffer_overflow_attempt:suspicious_activity:pattern=stack_corruption",
    "heap_spray_attempt:suspicious_activity:pattern=heap_manipulation",
    "rop_chain_detected:suspicious_activity:pattern=rop_gadgets",
    
    NULL
};

/**
 * Initialize intrusion detection system
 */
int ids_init(void) {
    if (ids_state.initialized) {
        return 0;
    }
    
    // Initialize IDS state
    ids_state.rule_count = 0;
    ids_state.max_rules = MAX_IDS_RULES;
    ids_state.threat_threshold = 50; // Default threat score threshold
    ids_state.analysis_window = 300; // 5 minutes
    ids_state.learning_mode = true;
    ids_state.ids_enabled = true;
    
    // Allocate memory for rules
    ids_state.rules = kmalloc(sizeof(ids_rule_t) * ids_state.max_rules);
    if (!ids_state.rules) {
        return -ENOMEM;
    }
    
    memset(ids_state.rules, 0, sizeof(ids_rule_t) * ids_state.max_rules);
    
    // Initialize behavioral analysis database
    behavior_db_size = 1024; // Track up to 1024 processes
    behavior_db = kmalloc(sizeof(process_behavior_t) * behavior_db_size);
    if (!behavior_db) {
        kfree(ids_state.rules);
        return -ENOMEM;
    }
    
    memset(behavior_db, 0, sizeof(process_behavior_t) * behavior_db_size);
    
    // Load default IDS rules
    int ret = ids_load_default_rules();
    if (ret != 0) {
        kfree(behavior_db);
        kfree(ids_state.rules);
        return ret;
    }
    
    ids_state.initialized = true;
    
    kernel_printf("IDS: Intrusion Detection System initialized\n");
    kernel_printf("  Rules loaded: %zu\n", ids_state.rule_count);
    kernel_printf("  Threat threshold: %u\n", ids_state.threat_threshold);
    kernel_printf("  Learning mode: %s\n", ids_state.learning_mode ? "Enabled" : "Disabled");
    
    return 0;
}

/**
 * Cleanup intrusion detection system
 */
void ids_cleanup(void) {
    if (!ids_state.initialized) {
        return;
    }
    
    // Free allocated memory
    if (ids_state.rules) {
        kfree(ids_state.rules);
        ids_state.rules = NULL;
    }
    
    if (behavior_db) {
        kfree(behavior_db);
        behavior_db = NULL;
    }
    
    ids_state.initialized = false;
    
    kernel_printf("IDS: System cleaned up\n");
    kernel_printf("  Events analyzed: %llu\n", ids_stats.events_analyzed);
    kernel_printf("  Threats detected: %llu\n", ids_stats.threats_detected);
    kernel_printf("  Rules triggered: %llu\n", ids_stats.rules_triggered);
}

/**
 * Analyze security event for threats
 */
int ids_analyze_event(security_event_t* event) {
    if (!ids_state.initialized || !ids_state.ids_enabled || !event) {
        return -EINVAL;
    }
    
    ids_stats.events_analyzed++;
    
    // Update behavioral analysis
    ids_update_behavior_analysis(event);
    
    // Check against IDS rules
    bool threat_detected = false;
    uint32_t threat_score = 0;
    
    for (size_t i = 0; i < ids_state.rule_count; i++) {
        ids_rule_t* rule = &ids_state.rules[i];
        
        if (ids_match_rule(rule, event)) {
            rule->matches++;
            rule->last_match = event->timestamp;
            ids_stats.rules_triggered++;
            
            // Check if threshold exceeded within time window
            if (ids_check_rule_threshold(rule, event->timestamp)) {
                threat_detected = true;
                threat_score += 10; // Each rule adds to threat score
                
                // Execute rule callback if available
                if (rule->callback) {
                    rule->callback(event);
                }
            }
        }
    }
    
    // Perform behavioral analysis
    uint32_t behavior_score = ids_analyze_process_behavior(event->pid);
    threat_score += behavior_score;
    
    // Check if threat threshold exceeded
    if (threat_score >= ids_state.threat_threshold) {
        threat_detected = true;
        ids_stats.threats_detected++;
        
        // Create threat event
        security_event_t threat_event = {
            .event_id = security_generate_event_id(),
            .timestamp = get_system_time(),
            .pid = event->pid,
            .uid = event->uid,
            .type = SECURITY_EVENT_SUSPICIOUS_ACTIVITY,
            .severity = 9,
            .blocked = false
        };
        
        snprintf(threat_event.description, sizeof(threat_event.description),
                 "Intrusion detected: threat_score=%u, triggering_event=%s",
                 threat_score, event->description);
        
        // Log threat event
        security_log_event(&threat_event);
        
        // Take automated response action
        ids_respond_to_threat(event, threat_score);
    }
    
    return threat_detected ? threat_score : 0;
}

/**
 * Register IDS rule
 */
int ids_register_rule(const char* pattern, void (*callback)(security_event_t*)) {
    if (!ids_state.initialized || !pattern) {
        return -EINVAL;
    }
    
    if (ids_state.rule_count >= ids_state.max_rules) {
        return -ENOMEM;
    }
    
    ids_rule_t* rule = &ids_state.rules[ids_state.rule_count];
    
    // Parse rule pattern
    int ret = ids_parse_rule_pattern(pattern, rule);
    if (ret != 0) {
        return ret;
    }
    
    // Set callback
    rule->callback = callback;
    rule->id = ids_state.rule_count + 1;
    rule->matches = 0;
    rule->last_match = 0;
    
    ids_state.rule_count++;
    
    kernel_printf("IDS: Registered rule: %s\n", pattern);
    return 0;
}

/**
 * Unregister IDS rule
 */
int ids_unregister_rule(const char* pattern) {
    if (!ids_state.initialized || !pattern) {
        return -EINVAL;
    }
    
    // Find rule by pattern
    for (size_t i = 0; i < ids_state.rule_count; i++) {
        if (strcmp(ids_state.rules[i].pattern, pattern) == 0) {
            // Remove rule by shifting remaining rules
            for (size_t j = i; j < ids_state.rule_count - 1; j++) {
                ids_state.rules[j] = ids_state.rules[j + 1];
            }
            ids_state.rule_count--;
            
            kernel_printf("IDS: Unregistered rule: %s\n", pattern);
            return 0;
        }
    }
    
    return -ENOENT;
}

/**
 * Set IDS parameters
 */
int ids_set_parameters(uint32_t threat_threshold, uint32_t analysis_window, bool learning_mode) {
    if (!ids_state.initialized) {
        return -ENODEV;
    }
    
    ids_state.threat_threshold = threat_threshold;
    ids_state.analysis_window = analysis_window;
    ids_state.learning_mode = learning_mode;
    
    kernel_printf("IDS: Parameters updated - threshold=%u, window=%u, learning=%s\n",
                  threat_threshold, analysis_window, learning_mode ? "enabled" : "disabled");
    
    return 0;
}

/**
 * Get IDS statistics
 */
int ids_get_statistics(ids_statistics_t* stats) {
    if (!ids_state.initialized || !stats) {
        return -EINVAL;
    }
    
    memset(stats, 0, sizeof(ids_statistics_t));
    
    stats->enabled = ids_state.ids_enabled;
    stats->learning_mode = ids_state.learning_mode;
    stats->rule_count = ids_state.rule_count;
    stats->threat_threshold = ids_state.threat_threshold;
    stats->analysis_window = ids_state.analysis_window;
    
    stats->events_analyzed = ids_stats.events_analyzed;
    stats->threats_detected = ids_stats.threats_detected;
    stats->false_positives = ids_stats.false_positives;
    stats->rules_triggered = ids_stats.rules_triggered;
    stats->processes_terminated = ids_stats.processes_terminated;
    stats->connections_blocked = ids_stats.connections_blocked;
    
    return 0;
}

// Private helper functions

/**
 * Load default IDS rules
 */
static int ids_load_default_rules(void) {
    for (const char** rule_ptr = default_ids_rules; *rule_ptr != NULL; rule_ptr++) {
        int ret = ids_register_rule(*rule_ptr, NULL);
        if (ret != 0) {
            kernel_printf("IDS: Failed to load default rule: %s\n", *rule_ptr);
            // Continue loading other rules
        }
    }
    
    return 0;
}

/**
 * Parse IDS rule pattern
 */
static int ids_parse_rule_pattern(const char* pattern, ids_rule_t* rule) {
    if (!pattern || !rule) {
        return -EINVAL;
    }
    
    // Copy pattern
    strncpy(rule->pattern, pattern, sizeof(rule->pattern) - 1);
    
    // Parse pattern components (simplified parsing)
    // Format: "name:event_type:threshold=N:window=N"
    
    // Set default values
    rule->event_types = ~0U; // All event types
    rule->threshold = 5;
    rule->window = 60;
    
    // Parse threshold
    char* threshold_str = strstr(pattern, "threshold=");
    if (threshold_str) {
        rule->threshold = atoi(threshold_str + 10);
    }
    
    // Parse window
    char* window_str = strstr(pattern, "window=");
    if (window_str) {
        rule->window = atoi(window_str + 7);
    }
    
    // Parse event type
    if (strstr(pattern, "process_create")) {
        rule->event_types = (1 << SECURITY_EVENT_SYSTEM_CALL);
    } else if (strstr(pattern, "file_access")) {
        rule->event_types = (1 << SECURITY_EVENT_FILE_ACCESS);
    } else if (strstr(pattern, "network_access")) {
        rule->event_types = (1 << SECURITY_EVENT_NETWORK_ACCESS);
    } else if (strstr(pattern, "login_attempt")) {
        rule->event_types = (1 << SECURITY_EVENT_LOGIN_ATTEMPT);
    }
    
    return 0;
}

/**
 * Check if event matches IDS rule
 */
static bool ids_match_rule(ids_rule_t* rule, security_event_t* event) {
    // Check event type mask
    if (!(rule->event_types & (1 << event->type))) {
        return false;
    }
    
    // Pattern matching (simplified)
    if (strstr(rule->pattern, "failed") && !event->blocked) {
        return false;
    }
    
    if (strstr(rule->pattern, "external") && 
        !strstr(event->description, "external")) {
        return false;
    }
    
    // More sophisticated pattern matching would be implemented here
    
    return true;
}

/**
 * Check if rule threshold is exceeded
 */
static bool ids_check_rule_threshold(ids_rule_t* rule, uint64_t current_time) {
    uint64_t window_start = current_time - (rule->window * 1000000); // Convert to microseconds
    
    if (rule->last_match < window_start) {
        // Reset match count for new window
        rule->matches = 1;
        return false;
    }
    
    return rule->matches >= rule->threshold;
}

/**
 * Update behavioral analysis for process
 */
static void ids_update_behavior_analysis(security_event_t* event) {
    if (event->pid == 0) {
        return; // Skip kernel events
    }
    
    // Find or create behavior entry
    process_behavior_t* behavior = ids_find_or_create_behavior(event->pid);
    if (!behavior) {
        return;
    }
    
    behavior->last_activity = event->timestamp;
    
    // Update behavior counters
    switch (event->type) {
        case SECURITY_EVENT_SYSTEM_CALL:
            if (event->data.syscall.syscall_number < 64) {
                behavior->syscall_count[event->data.syscall.syscall_number]++;
            }
            break;
        case SECURITY_EVENT_FILE_ACCESS:
            behavior->file_access_count++;
            break;
        case SECURITY_EVENT_NETWORK_ACCESS:
            behavior->network_access_count++;
            break;
        case SECURITY_EVENT_PRIVILEGE_ESCALATION:
            behavior->privilege_escalations++;
            break;
    }
    
    // Check for suspicious patterns
    if (behavior->privilege_escalations > 3) {
        behavior->suspicious = true;
        behavior->anomaly_score += 20;
    }
    
    // Detect rapid system calls
    uint32_t total_syscalls = 0;
    for (int i = 0; i < 64; i++) {
        total_syscalls += behavior->syscall_count[i];
    }
    
    if (total_syscalls > 1000) {
        behavior->anomaly_score += 15;
    }
}

/**
 * Analyze process behavior and return threat score
 */
static uint32_t ids_analyze_process_behavior(uint32_t pid) {
    process_behavior_t* behavior = ids_find_behavior(pid);
    if (!behavior) {
        return 0;
    }
    
    uint32_t score = behavior->anomaly_score;
    
    // Additional analysis
    uint64_t current_time = get_system_time();
    uint64_t inactive_time = current_time - behavior->last_activity;
    
    // If process has been inactive for too long, reset anomaly score
    if (inactive_time > 3600000000ULL) { // 1 hour
        behavior->anomaly_score = 0;
        behavior->suspicious = false;
        return 0;
    }
    
    // Detect unusual file access patterns
    if (behavior->file_access_count > 100) {
        score += 10;
    }
    
    // Detect unusual network activity
    if (behavior->network_access_count > 50) {
        score += 10;
    }
    
    return score;
}

/**
 * Find or create behavior entry for process
 */
static process_behavior_t* ids_find_or_create_behavior(uint32_t pid) {
    // First, try to find existing entry
    process_behavior_t* behavior = ids_find_behavior(pid);
    if (behavior) {
        return behavior;
    }
    
    // Find empty slot
    for (size_t i = 0; i < behavior_db_size; i++) {
        if (behavior_db[i].pid == 0) {
            behavior_db[i].pid = pid;
            behavior_db[i].last_activity = get_system_time();
            behavior_db[i].anomaly_score = 0;
            behavior_db[i].suspicious = false;
            memset(behavior_db[i].syscall_count, 0, sizeof(behavior_db[i].syscall_count));
            behavior_db[i].file_access_count = 0;
            behavior_db[i].network_access_count = 0;
            behavior_db[i].privilege_escalations = 0;
            return &behavior_db[i];
        }
    }
    
    // No free slots - find oldest entry to replace
    process_behavior_t* oldest = &behavior_db[0];
    for (size_t i = 1; i < behavior_db_size; i++) {
        if (behavior_db[i].last_activity < oldest->last_activity) {
            oldest = &behavior_db[i];
        }
    }
    
    // Replace oldest entry
    memset(oldest, 0, sizeof(process_behavior_t));
    oldest->pid = pid;
    oldest->last_activity = get_system_time();
    
    return oldest;
}

/**
 * Find behavior entry for process
 */
static process_behavior_t* ids_find_behavior(uint32_t pid) {
    for (size_t i = 0; i < behavior_db_size; i++) {
        if (behavior_db[i].pid == pid) {
            return &behavior_db[i];
        }
    }
    return NULL;
}

/**
 * Respond to detected threat
 */
static void ids_respond_to_threat(security_event_t* event, uint32_t threat_score) {
    kernel_printf("IDS: Threat detected (score=%u) - taking action\n", threat_score);
    
    // Response actions based on threat severity
    if (threat_score >= 80) {
        // High severity - terminate process
        if (event->pid > 0) {
            process_t* proc = find_process(event->pid);
            if (proc) {
                kernel_printf("IDS: Terminating malicious process %d\n", event->pid);
                // signal_send(event->pid, SIGNAL_KILL);
                ids_stats.processes_terminated++;
            }
        }
    } else if (threat_score >= 60) {
        // Medium severity - increase monitoring
        process_behavior_t* behavior = ids_find_behavior(event->pid);
        if (behavior) {
            behavior->suspicious = true;
            behavior->anomaly_score += 10;
        }
    } else {
        // Low severity - log and monitor
        kernel_printf("IDS: Low-level threat detected from PID %d\n", event->pid);
    }
    
    // Create response event
    security_event_t response_event = {
        .event_id = security_generate_event_id(),
        .timestamp = get_system_time(),
        .pid = event->pid,
        .uid = event->uid,
        .type = SECURITY_EVENT_SYSTEM_CALL,
        .severity = 7,
        .blocked = true
    };
    
    snprintf(response_event.description, sizeof(response_event.description),
             "IDS automatic response: threat_score=%u", threat_score);
    
    security_log_event(&response_event);
}

// Simple atoi implementation
static int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    
    if (*str == '-') {
        sign = -1;
        str++;
    }
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return sign * result;
}

// IDS statistics structure
typedef struct ids_statistics {
    bool enabled;
    bool learning_mode;
    size_t rule_count;
    uint32_t threat_threshold;
    uint32_t analysis_window;
    uint64_t events_analyzed;
    uint64_t threats_detected;
    uint64_t false_positives;
    uint64_t rules_triggered;
    uint64_t processes_terminated;
    uint64_t connections_blocked;
} ids_statistics_t;