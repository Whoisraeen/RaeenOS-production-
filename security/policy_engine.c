/**
 * RaeenOS Security Framework - Policy Engine
 * Advanced security policy evaluation and enforcement system
 */

#include "raesec.h"
#include "../kernel/process_advanced.h"
#include "../memory.h"
#include "../string.h"
#include "../time.h"

// Policy evaluation cache
typedef struct {
    char cache_key[256];
    PolicyAction cached_action;
    time_t cache_time;
    uint32_t hit_count;
} PolicyCacheEntry;

static PolicyCacheEntry* g_policy_cache = NULL;
static uint32_t g_cache_size = 0;
static uint32_t g_cache_capacity = 1024;
static pthread_mutex_t g_cache_mutex = PTHREAD_MUTEX_INITIALIZER;

// Policy statistics
static uint64_t g_policy_evaluations = 0;
static uint64_t g_cache_hits = 0;
static uint64_t g_cache_misses = 0;

// Internal function declarations
static PolicyAction evaluate_single_rule(SecurityPolicyRule* rule, SecurityContext* context, const char* resource_path, CapabilityType capability);
static bool rule_matches_context(SecurityPolicyRule* rule, SecurityContext* context);
static bool rule_matches_resource(SecurityPolicyRule* rule, const char* resource_path);
static bool rule_matches_capability(SecurityPolicyRule* rule, CapabilityType capability);
static bool rule_matches_time(SecurityPolicyRule* rule);
static PolicyAction get_cached_policy(const char* cache_key);
static void cache_policy_result(const char* cache_key, PolicyAction action);
static void generate_cache_key(SecurityContext* context, const char* resource_path, CapabilityType capability, char* cache_key);
static bool load_policy_from_json(SecurityFramework* framework, const char* json_file);
static bool save_policy_to_json(SecurityFramework* framework, const char* json_file);

/**
 * Evaluate security policy for a request
 */
PolicyAction raesec_evaluate_policy(SecurityFramework* framework, SecurityContext* context, const char* resource_path, CapabilityType capability, PolicyAction* action) {
    if (!framework || !context || !resource_path || !action) {
        return POLICY_ACTION_DENY;
    }
    
    g_policy_evaluations++;
    
    // Generate cache key
    char cache_key[256];
    generate_cache_key(context, resource_path, capability, cache_key);
    
    // Check policy cache first
    PolicyAction cached_action = get_cached_policy(cache_key);
    if (cached_action != POLICY_ACTION_DENY || g_cache_hits > 0) {
        g_cache_hits++;
        *action = cached_action;
        return cached_action;
    }
    
    g_cache_misses++;
    
    pthread_rwlock_rdlock(&framework->policy_rwlock);
    
    PolicyAction final_action = POLICY_ACTION_DENY;
    uint32_t highest_priority = 0;
    bool rule_matched = false;
    
    // Evaluate all policy rules
    SecurityPolicyRule* current_rule = framework->policy_rules;
    while (current_rule) {
        if (current_rule->enabled && current_rule->priority >= highest_priority) {
            PolicyAction rule_action = evaluate_single_rule(current_rule, context, resource_path, capability);
            
            if (rule_action != POLICY_ACTION_DENY) {
                if (current_rule->priority > highest_priority || !rule_matched) {
                    final_action = rule_action;
                    highest_priority = current_rule->priority;
                    rule_matched = true;
                }
            }
        }
        current_rule = current_rule->next;
    }
    
    pthread_rwlock_unlock(&framework->policy_rwlock);
    
    // If no rules matched, apply default policy based on context type
    if (!rule_matched) {
        switch (context->type) {
            case SEC_CONTEXT_SYSTEM:
                final_action = POLICY_ACTION_ALLOW;
                break;
            case SEC_CONTEXT_ADMIN:
                final_action = POLICY_ACTION_ALLOW;
                break;
            case SEC_CONTEXT_USER:
                final_action = POLICY_ACTION_PROMPT;
                break;
            case SEC_CONTEXT_SANDBOX:
                final_action = POLICY_ACTION_DENY;
                break;
            case SEC_CONTEXT_RESTRICTED:
                final_action = POLICY_ACTION_DENY;
                break;
            case SEC_CONTEXT_UNTRUSTED:
                final_action = POLICY_ACTION_QUARANTINE;
                break;
        }
    }
    
    // Cache the result
    cache_policy_result(cache_key, final_action);
    
    *action = final_action;
    return final_action;
}

/**
 * Add a policy rule to the framework
 */
bool raesec_add_policy_rule(SecurityFramework* framework, SecurityPolicyRule* rule) {
    if (!framework || !rule) return false;
    
    pthread_rwlock_wrlock(&framework->policy_rwlock);
    
    // Assign rule ID if not set
    if (rule->rule_id == 0) {
        rule->rule_id = framework->rule_count + 1;
    }
    
    // Set creation time
    rule->created_time = time(NULL);
    rule->modified_time = rule->created_time;
    
    // Add to linked list
    rule->next = framework->policy_rules;
    framework->policy_rules = rule;
    framework->rule_count++;
    
    pthread_rwlock_unlock(&framework->policy_rwlock);
    
    printf("Added policy rule %lu: %s\n", rule->rule_id, rule->name);
    return true;
}

/**
 * Remove a policy rule from the framework
 */
bool raesec_remove_policy_rule(SecurityFramework* framework, uint64_t rule_id) {
    if (!framework || rule_id == 0) return false;
    
    pthread_rwlock_wrlock(&framework->policy_rwlock);
    
    SecurityPolicyRule* current = framework->policy_rules;
    SecurityPolicyRule* previous = NULL;
    
    while (current) {
        if (current->rule_id == rule_id) {
            if (previous) {
                previous->next = current->next;
            } else {
                framework->policy_rules = current->next;
            }
            
            printf("Removed policy rule %lu: %s\n", rule_id, current->name);
            free(current);
            framework->rule_count--;
            
            pthread_rwlock_unlock(&framework->policy_rwlock);
            return true;
        }
        
        previous = current;
        current = current->next;
    }
    
    pthread_rwlock_unlock(&framework->policy_rwlock);
    return false;
}

/**
 * Load policy rules from JSON file
 */
bool raesec_load_policies(SecurityFramework* framework, const char* policy_dir) {
    if (!framework || !policy_dir) return false;
    
    char policy_file[512];
    snprintf(policy_file, sizeof(policy_file), "%s/security_policies.json", policy_dir);
    
    return load_policy_from_json(framework, policy_file);
}

/**
 * Save policy rules to JSON file
 */
bool raesec_save_policies(SecurityFramework* framework, const char* policy_dir) {
    if (!framework || !policy_dir) return false;
    
    char policy_file[512];
    snprintf(policy_file, sizeof(policy_file), "%s/security_policies.json", policy_dir);
    
    return save_policy_to_json(framework, policy_file);
}

/**
 * Create a default policy rule
 */
SecurityPolicyRule* raesec_create_policy_rule(const char* name, const char* description, PolicyAction action, uint32_t priority) {
    SecurityPolicyRule* rule = calloc(1, sizeof(SecurityPolicyRule));
    if (!rule) return NULL;
    
    strncpy(rule->name, name ? name : "Unnamed Rule", sizeof(rule->name) - 1);
    strncpy(rule->description, description ? description : "", sizeof(rule->description) - 1);
    rule->action = action;
    rule->priority = priority;
    rule->enabled = true;
    rule->created_time = time(NULL);
    rule->modified_time = rule->created_time;
    
    return rule;
}

/**
 * Update policy rule
 */
bool raesec_update_policy_rule(SecurityFramework* framework, uint64_t rule_id, SecurityPolicyRule* updated_rule) {
    if (!framework || !updated_rule || rule_id == 0) return false;
    
    pthread_rwlock_wrlock(&framework->policy_rwlock);
    
    SecurityPolicyRule* current = framework->policy_rules;
    while (current) {
        if (current->rule_id == rule_id) {
            // Update fields
            strncpy(current->name, updated_rule->name, sizeof(current->name) - 1);
            strncpy(current->description, updated_rule->description, sizeof(current->description) - 1);
            strncpy(current->process_pattern, updated_rule->process_pattern, sizeof(current->process_pattern) - 1);
            strncpy(current->user_pattern, updated_rule->user_pattern, sizeof(current->user_pattern) - 1);
            strncpy(current->path_pattern, updated_rule->path_pattern, sizeof(current->path_pattern) - 1);
            
            current->capability = updated_rule->capability;
            current->context_type = updated_rule->context_type;
            current->action = updated_rule->action;
            current->priority = updated_rule->priority;
            current->enabled = updated_rule->enabled;
            current->modified_time = time(NULL);
            
            pthread_rwlock_unlock(&framework->policy_rwlock);
            printf("Updated policy rule %lu: %s\n", rule_id, current->name);
            return true;
        }
        current = current->next;
    }
    
    pthread_rwlock_unlock(&framework->policy_rwlock);
    return false;
}

/**
 * Get policy statistics
 */
void raesec_get_policy_statistics(uint64_t* evaluations, uint64_t* cache_hits, uint64_t* cache_misses, double* cache_hit_ratio) {
    if (evaluations) *evaluations = g_policy_evaluations;
    if (cache_hits) *cache_hits = g_cache_hits;
    if (cache_misses) *cache_misses = g_cache_misses;
    if (cache_hit_ratio) {
        *cache_hit_ratio = (g_policy_evaluations > 0) ? 
            ((double)g_cache_hits / (double)g_policy_evaluations) * 100.0 : 0.0;
    }
}

/**
 * Clear policy cache
 */
void raesec_clear_policy_cache(void) {
    pthread_mutex_lock(&g_cache_mutex);
    
    if (g_policy_cache) {
        memset(g_policy_cache, 0, g_cache_capacity * sizeof(PolicyCacheEntry));
        g_cache_size = 0;
    }
    
    pthread_mutex_unlock(&g_cache_mutex);
    
    printf("Policy cache cleared\n");
}

// Internal helper function implementations

/**
 * Evaluate a single policy rule
 */
static PolicyAction evaluate_single_rule(SecurityPolicyRule* rule, SecurityContext* context, const char* resource_path, CapabilityType capability) {
    if (!rule || !context || !resource_path) {
        return POLICY_ACTION_DENY;
    }
    
    // Check if rule matches the request
    if (!rule_matches_context(rule, context) ||
        !rule_matches_resource(rule, resource_path) ||
        !rule_matches_capability(rule, capability) ||
        !rule_matches_time(rule)) {
        return POLICY_ACTION_DENY;
    }
    
    // Check violation count
    if (rule->max_violations > 0 && rule->current_violations >= rule->max_violations) {
        return POLICY_ACTION_DENY;
    }
    
    return rule->action;
}

/**
 * Check if rule matches security context
 */
static bool rule_matches_context(SecurityPolicyRule* rule, SecurityContext* context) {
    // Check context type
    if (rule->context_type != SEC_CONTEXT_SYSTEM && rule->context_type != context->type) {
        return false;
    }
    
    // Check user pattern
    if (strlen(rule->user_pattern) > 0) {
        char user_str[32];
        snprintf(user_str, sizeof(user_str), "%u", context->user_id);
        if (fnmatch(rule->user_pattern, user_str, 0) != 0) {
            return false;
        }
    }
    
    // Check process pattern
    if (strlen(rule->process_pattern) > 0) {
        char process_str[32];
        snprintf(process_str, sizeof(process_str), "%u", context->process_id);
        if (fnmatch(rule->process_pattern, process_str, 0) != 0) {
            return false;
        }
    }
    
    return true;
}

/**
 * Check if rule matches resource path
 */
static bool rule_matches_resource(SecurityPolicyRule* rule, const char* resource_path) {
    if (strlen(rule->path_pattern) == 0) {
        return true; // No path restriction
    }
    
    return fnmatch(rule->path_pattern, resource_path, 0) == 0;
}

/**
 * Check if rule matches capability
 */
static bool rule_matches_capability(SecurityPolicyRule* rule, CapabilityType capability) {
    if (rule->capability == CAP_MAX_CAPABILITY) {
        return true; // Matches all capabilities
    }
    
    return rule->capability == capability;
}

/**
 * Check if rule matches current time
 */
static bool rule_matches_time(SecurityPolicyRule* rule) {
    time_t current_time = time(NULL);
    
    if (rule->start_time > 0 && current_time < rule->start_time) {
        return false;
    }
    
    if (rule->end_time > 0 && current_time > rule->end_time) {
        return false;
    }
    
    return true;
}

/**
 * Get cached policy result
 */
static PolicyAction get_cached_policy(const char* cache_key) {
    pthread_mutex_lock(&g_cache_mutex);
    
    if (!g_policy_cache) {
        g_policy_cache = calloc(g_cache_capacity, sizeof(PolicyCacheEntry));
    }
    
    for (uint32_t i = 0; i < g_cache_size; i++) {
        if (strcmp(g_policy_cache[i].cache_key, cache_key) == 0) {
            // Check if cache entry is still valid (5 minutes)
            if (time(NULL) - g_policy_cache[i].cache_time < 300) {
                g_policy_cache[i].hit_count++;
                PolicyAction action = g_policy_cache[i].cached_action;
                pthread_mutex_unlock(&g_cache_mutex);
                return action;
            }
        }
    }
    
    pthread_mutex_unlock(&g_cache_mutex);
    return POLICY_ACTION_DENY; // Cache miss
}

/**
 * Cache policy result
 */
static void cache_policy_result(const char* cache_key, PolicyAction action) {
    pthread_mutex_lock(&g_cache_mutex);
    
    if (!g_policy_cache) {
        g_policy_cache = calloc(g_cache_capacity, sizeof(PolicyCacheEntry));
    }
    
    // Find existing entry or create new one
    uint32_t cache_index = g_cache_size;
    for (uint32_t i = 0; i < g_cache_size; i++) {
        if (strcmp(g_policy_cache[i].cache_key, cache_key) == 0) {
            cache_index = i;
            break;
        }
    }
    
    // Add new entry if cache not full
    if (cache_index == g_cache_size && g_cache_size < g_cache_capacity) {
        g_cache_size++;
    } else if (cache_index == g_cache_size) {
        // Cache full, replace oldest entry
        cache_index = 0;
        time_t oldest_time = g_policy_cache[0].cache_time;
        for (uint32_t i = 1; i < g_cache_capacity; i++) {
            if (g_policy_cache[i].cache_time < oldest_time) {
                oldest_time = g_policy_cache[i].cache_time;
                cache_index = i;
            }
        }
    }
    
    // Update cache entry
    strncpy(g_policy_cache[cache_index].cache_key, cache_key, sizeof(g_policy_cache[cache_index].cache_key) - 1);
    g_policy_cache[cache_index].cached_action = action;
    g_policy_cache[cache_index].cache_time = time(NULL);
    g_policy_cache[cache_index].hit_count = 0;
    
    pthread_mutex_unlock(&g_cache_mutex);
}

/**
 * Generate cache key for policy lookup
 */
static void generate_cache_key(SecurityContext* context, const char* resource_path, CapabilityType capability, char* cache_key) {
    snprintf(cache_key, 256, "%lu_%s_%d_%u", 
             context->context_id, resource_path, capability, context->user_id);
}

/**
 * Load policy from JSON file
 */
static bool load_policy_from_json(SecurityFramework* framework, const char* json_file) {
    FILE* file = fopen(json_file, "r");
    if (!file) {
        printf("Policy file not found: %s\n", json_file);
        return false;
    }
    
    // Read file content
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* json_data = malloc(file_size + 1);
    fread(json_data, 1, file_size, file);
    json_data[file_size] = '\0';
    fclose(file);
    
    // Parse JSON
    json_object* root = json_tokener_parse(json_data);
    if (!root) {
        printf("Failed to parse policy JSON: %s\n", json_file);
        free(json_data);
        return false;
    }
    
    // Load policy rules
    json_object* rules_array;
    if (json_object_object_get_ex(root, "rules", &rules_array)) {
        int rule_count = json_object_array_length(rules_array);
        
        for (int i = 0; i < rule_count; i++) {
            json_object* rule_obj = json_object_array_get_idx(rules_array, i);
            
            SecurityPolicyRule* rule = calloc(1, sizeof(SecurityPolicyRule));
            
            // Parse rule fields
            json_object* name_obj, *desc_obj, *action_obj, *priority_obj;
            if (json_object_object_get_ex(rule_obj, "name", &name_obj)) {
                strncpy(rule->name, json_object_get_string(name_obj), sizeof(rule->name) - 1);
            }
            if (json_object_object_get_ex(rule_obj, "description", &desc_obj)) {
                strncpy(rule->description, json_object_get_string(desc_obj), sizeof(rule->description) - 1);
            }
            if (json_object_object_get_ex(rule_obj, "action", &action_obj)) {
                const char* action_str = json_object_get_string(action_obj);
                if (strcmp(action_str, "allow") == 0) rule->action = POLICY_ACTION_ALLOW;
                else if (strcmp(action_str, "deny") == 0) rule->action = POLICY_ACTION_DENY;
                else if (strcmp(action_str, "prompt") == 0) rule->action = POLICY_ACTION_PROMPT;
                else if (strcmp(action_str, "log") == 0) rule->action = POLICY_ACTION_LOG;
                else if (strcmp(action_str, "quarantine") == 0) rule->action = POLICY_ACTION_QUARANTINE;
            }
            if (json_object_object_get_ex(rule_obj, "priority", &priority_obj)) {
                rule->priority = json_object_get_int(priority_obj);
            }
            
            rule->enabled = true;
            raesec_add_policy_rule(framework, rule);
        }
    }
    
    json_object_put(root);
    free(json_data);
    
    printf("Loaded %u policy rules from %s\n", framework->rule_count, json_file);
    return true;
}

/**
 * Save policy to JSON file
 */
static bool save_policy_to_json(SecurityFramework* framework, const char* json_file) {
    json_object* root = json_object_new_object();
    json_object* rules_array = json_object_new_array();
    
    pthread_rwlock_rdlock(&framework->policy_rwlock);
    
    SecurityPolicyRule* current = framework->policy_rules;
    while (current) {
        json_object* rule_obj = json_object_new_object();
        
        json_object_object_add(rule_obj, "name", json_object_new_string(current->name));
        json_object_object_add(rule_obj, "description", json_object_new_string(current->description));
        json_object_object_add(rule_obj, "action", json_object_new_string(raesec_policy_action_to_string(current->action)));
        json_object_object_add(rule_obj, "priority", json_object_new_int(current->priority));
        json_object_object_add(rule_obj, "enabled", json_object_new_boolean(current->enabled));
        
        json_object_array_add(rules_array, rule_obj);
        current = current->next;
    }
    
    pthread_rwlock_unlock(&framework->policy_rwlock);
    
    json_object_object_add(root, "rules", rules_array);
    
    // Write to file
    const char* json_string = json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY);
    FILE* file = fopen(json_file, "w");
    if (file) {
        fprintf(file, "%s\n", json_string);
        fclose(file);
    }
    
    json_object_put(root);
    
    printf("Saved %u policy rules to %s\n", framework->rule_count, json_file);
    return true;
}

/**
 * Convert policy action to string
 */
const char* raesec_policy_action_to_string(PolicyAction action) {
    switch (action) {
        case POLICY_ACTION_ALLOW: return "allow";
        case POLICY_ACTION_DENY: return "deny";
        case POLICY_ACTION_LOG: return "log";
        case POLICY_ACTION_PROMPT: return "prompt";
        case POLICY_ACTION_QUARANTINE: return "quarantine";
        default: return "unknown";
    }
}
