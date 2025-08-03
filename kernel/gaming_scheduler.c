/**
 * @file gaming_scheduler.c
 * @brief Gaming-Optimized Ultra-Low Latency Scheduler for RaeenOS
 * 
 * This module implements gaming-specific scheduling optimizations designed to
 * achieve sub-microsecond context switching and ultra-low input latency.
 * Features include:
 * 
 * - Gaming thread detection and prioritization
 * - Frame-rate aware scheduling with deadline guarantees
 * - Input processing priority boosts
 * - Exclusive CPU core allocation for gaming
 * - VSync-synchronized scheduling
 * - Anti-jitter and frame pacing algorithms
 * - GPU coordination for optimal frame delivery
 * 
 * @version 1.0
 * @date 2025-08-01
 */

#include "include/scheduler.h"
#include "include/sync.h"
#include "vga.h"
#include "memory.h"
#include "string.h"
#include "timer.h"

// Gaming mode thread detection patterns
#define GAMING_THREAD_NAME_PATTERNS_MAX 16
static const char* gaming_thread_patterns[GAMING_THREAD_NAME_PATTERNS_MAX] = {
    "render",       // Rendering threads
    "game",         // Game logic threads  
    "audio",        // Audio threads
    "input",        // Input processing
    "physics",      // Physics simulation
    "network",      // Network game threads
    "streaming",    // Game streaming
    "capture",      // Game capture/recording
    "overlay",      // Gaming overlays
    "engine",       // Game engine threads
    "dx11",         // DirectX 11 threads
    "dx12",         // DirectX 12 threads
    "vulkan",       // Vulkan threads
    "opengl",       // OpenGL threads
    "unity",        // Unity engine
    "unreal"        // Unreal engine
};

// Gaming performance tracking
typedef struct gaming_perf_tracker {
    uint64_t frame_start_ns;              // Frame start timestamp
    uint64_t frame_end_ns;                // Frame end timestamp  
    uint64_t frame_duration_ns;           // Last frame duration
    uint64_t frame_target_ns;             // Target frame duration
    uint32_t frames_rendered;             // Total frames rendered
    uint32_t frames_dropped;              // Dropped/missed frames
    uint32_t frame_rate_current;          // Current FPS
    uint32_t frame_rate_target;           // Target FPS
    
    // Input latency tracking
    uint64_t last_input_timestamp_ns;     // Last input event
    uint64_t input_to_render_latency_ns;  // Input-to-render latency
    uint32_t input_events_processed;      // Input events processed
    
    // GPU synchronization
    bool vsync_enabled;                   // VSync status
    uint64_t vsync_timestamp_ns;          // Last VSync timestamp
    uint32_t gpu_frame_queue_depth;       // GPU frame queue depth
    
    // Anti-jitter control
    uint64_t frame_time_variance_ns;      // Frame time variance
    uint64_t smooth_frame_time_ns;        // Smoothed frame time
    bool frame_pacing_enabled;            // Frame pacing active
    
} gaming_perf_tracker_t;

// Per-process gaming context
typedef struct gaming_context {
    bool is_gaming_process;               // Identified as gaming process
    gaming_perf_tracker_t perf;           // Performance tracking
    
    // Priority management
    int32_t base_priority;                // Base gaming priority
    int32_t boost_priority;               // Temporary priority boost
    uint64_t boost_expiry_ns;             // Priority boost expiry time
    
    // CPU affinity for gaming
    cpu_mask_t gaming_cpu_mask;           // CPUs allocated for gaming
    bool exclusive_cpu_mode;              // Exclusive CPU allocation
    uint32_t preferred_render_cpu;        // Preferred CPU for rendering
    uint32_t preferred_input_cpu;         // Preferred CPU for input
    
    // Frame scheduling
    uint64_t next_frame_deadline_ns;      // Next frame deadline
    uint64_t frame_budget_ns;             // CPU budget per frame
    bool frame_deadline_missed;           // Last frame deadline status
    
    // Thread classification
    bool is_render_thread;                // Main rendering thread
    bool is_input_thread;                 // Input processing thread
    bool is_audio_thread;                 // Audio processing thread
    bool is_physics_thread;               // Physics simulation thread
    
} gaming_context_t;

// Global gaming scheduler state
static struct {
    bool initialized;                     // Gaming scheduler initialized
    spinlock_t lock;                      // Gaming scheduler lock
    
    // Gaming processes tracking
    process_t* gaming_processes[MAX_PROCESSES];
    uint32_t gaming_process_count;
    
    // CPU allocation for gaming
    cpu_mask_t gaming_cpu_mask;           // CPUs reserved for gaming
    cpu_mask_t system_cpu_mask;           // CPUs for system tasks
    bool exclusive_mode_active;           // Exclusive gaming mode
    
    // Frame rate management
    uint32_t global_target_fps;           // Global target frame rate
    uint64_t global_frame_period_ns;      // Global frame period
    uint64_t last_vsync_ns;               // Last VSync timestamp
    
    // Performance statistics
    uint64_t total_gaming_context_switches;
    uint64_t total_frame_deadlines_met;
    uint64_t total_frame_deadlines_missed;
    uint32_t average_input_latency_us;
    
} g_gaming_scheduler;

// Forward declarations
static bool detect_gaming_process(process_t* proc);
static void setup_gaming_context(process_t* proc);
static void optimize_gaming_cpu_affinity(process_t* proc);
static void handle_frame_deadline(process_t* proc);
static void boost_input_priority(process_t* proc);
static uint64_t calculate_frame_budget(uint32_t target_fps);
static void update_gaming_performance_stats(process_t* proc);
static void frame_pacing_algorithm(process_t* proc);

/**
 * Initialize the gaming scheduler
 */
void gaming_scheduler_init(void) {
    vga_puts("Initializing Gaming Scheduler with Ultra-Low Latency...\n");
    
    memset(&g_gaming_scheduler, 0, sizeof(g_gaming_scheduler));
    spinlock_init(&g_gaming_scheduler.lock);
    
    // Configure default gaming parameters
    g_gaming_scheduler.global_target_fps = 60;
    g_gaming_scheduler.global_frame_period_ns = calculate_frame_budget(60);
    
    // Allocate CPUs for gaming (reserve high-performance cores)
    g_gaming_scheduler.gaming_cpu_mask = 0x0F;      // First 4 CPUs for gaming
    g_gaming_scheduler.system_cpu_mask = 0xF0;      // Remaining CPUs for system
    
    g_gaming_scheduler.initialized = true;
    
    vga_puts("Gaming Scheduler initialized - Ready for ultra-low latency gaming\n");
}

/**
 * Enable gaming mode globally
 */
void gaming_mode_enable(void) {
    if (!g_gaming_scheduler.initialized) {
        gaming_scheduler_init();
    }
    
    uint64_t flags;
    spin_lock_irqsave(&g_gaming_scheduler.lock, &flags);
    
    g_gaming_config.enabled = true;
    g_gaming_scheduler.exclusive_mode_active = g_gaming_config.exclusive_cpu_mode;
    
    // Configure system for gaming
    if (g_gaming_config.disable_power_save) {
        g_power_state.enabled = false;
        
        // Set all CPUs to maximum frequency
        for (uint32_t cpu = 0; cpu < g_scheduler.active_cpus; cpu++) {
            power_scale_frequency(cpu, g_gaming_config.min_cpu_frequency_mhz);
        }
    }
    
    // Optimize timer frequency for gaming (1000 Hz for 1ms precision)
    arch_set_timer_interrupt(1000000); // 1ms intervals
    
    spin_unlock_irqrestore(&g_gaming_scheduler.lock, flags);
    
    vga_puts("Gaming Mode ENABLED - Ultra-low latency active\n");
}

/**
 * Disable gaming mode
 */
void gaming_mode_disable(void) {
    uint64_t flags;
    spin_lock_irqsave(&g_gaming_scheduler.lock, &flags);
    
    g_gaming_config.enabled = false;
    g_gaming_scheduler.exclusive_mode_active = false;
    
    // Re-enable power management
    g_power_state.enabled = true;
    
    // Reset timer to normal frequency (100 Hz)
    arch_set_timer_interrupt(10000000); // 10ms intervals
    
    spin_unlock_irqrestore(&g_gaming_scheduler.lock, flags);
    
    vga_puts("Gaming Mode DISABLED - Normal scheduling restored\n");
}

/**
 * Configure gaming mode parameters
 */
void gaming_mode_configure(const gaming_config_t* config) {
    if (!config) return;
    
    uint64_t flags;
    spin_lock_irqsave(&g_gaming_scheduler.lock, &flags);
    
    // Copy configuration
    memcpy(&g_gaming_config, config, sizeof(gaming_config_t));
    
    // Update global parameters
    g_gaming_scheduler.global_target_fps = config->frame_rate_target;
    g_gaming_scheduler.global_frame_period_ns = calculate_frame_budget(config->frame_rate_target);
    g_gaming_scheduler.gaming_cpu_mask = config->gaming_cpu_mask;
    
    spin_unlock_irqrestore(&g_gaming_scheduler.lock, flags);
    
    vga_puts("Gaming Mode configured with custom parameters\n");
}

/**
 * Boost a process for gaming performance
 */
void gaming_boost_process(process_t* proc) {
    if (!proc || !proc->sched_entity) return;
    
    sched_entity_t* se = (sched_entity_t*)proc->sched_entity;
    
    // Detect if this is a gaming process
    if (!se->gaming_mode && detect_gaming_process(proc)) {
        se->gaming_mode = true;
        setup_gaming_context(proc);
        optimize_gaming_cpu_affinity(proc);
        
        // Add to gaming process list
        uint64_t flags;
        spin_lock_irqsave(&g_gaming_scheduler.lock, &flags);
        
        if (g_gaming_scheduler.gaming_process_count < MAX_PROCESSES) {
            g_gaming_scheduler.gaming_processes[g_gaming_scheduler.gaming_process_count] = proc;
            g_gaming_scheduler.gaming_process_count++;
        }
        
        spin_unlock_irqrestore(&g_gaming_scheduler.lock, flags);
        
        vga_puts("Process boosted for gaming performance\n");
    }
    
    // Apply immediate priority boost
    se->priority = g_gaming_config.input_boost_priority;
    se->boost_count++;
    
    // Set frame deadline if specified
    if (g_gaming_config.frame_deadline_ns > 0) {
        gaming_set_frame_deadline(proc, g_gaming_config.frame_deadline_ns);
    }
}

/**
 * Set frame deadline for gaming process
 */
void gaming_set_frame_deadline(process_t* proc, uint64_t deadline_ns) {
    if (!proc || !proc->sched_entity) return;
    
    sched_entity_t* se = (sched_entity_t*)proc->sched_entity;
    
    if (!se->gaming_mode) {
        gaming_boost_process(proc);
    }
    
    se->frame_deadline_ns = deadline_ns;
    
    // Ensure process is scheduled before deadline
    uint64_t now = get_timestamp_ns();
    if (deadline_ns > now) {
        // Calculate remaining time budget
        uint64_t remaining_ns = deadline_ns - now;
        se->time_quantum_remaining_ns = remaining_ns;
        
        // Move to gaming queue for immediate scheduling
        scheduler_enqueue_task(proc, se->preferred_cpu);
    }
}

/**
 * Gaming-aware process scheduling hook
 */
process_t* gaming_schedule_next(uint32_t cpu_id) {
    if (!g_gaming_config.enabled || cpu_id >= MAX_CPUS) {
        return NULL;
    }
    
    cpu_runqueue_t* rq = &g_scheduler.cpu_runqueues[cpu_id];
    process_t* best_candidate = NULL;
    uint64_t earliest_deadline = UINT64_MAX;
    uint64_t now = get_timestamp_ns();
    
    // Check gaming queue for processes with urgent deadlines
    process_t* proc = rq->gaming_queue.head;
    while (proc) {
        sched_entity_t* se = (sched_entity_t*)proc->sched_entity;
        
        if (se->frame_deadline_ns > 0 && se->frame_deadline_ns < earliest_deadline) {
            earliest_deadline = se->frame_deadline_ns;
            best_candidate = proc;
        }
        
        proc = se->sched_next;
    }
    
    // Check if any gaming process has an urgent frame deadline
    if (best_candidate && earliest_deadline <= now + US_TO_NS(500)) { // 500µs urgency threshold
        handle_frame_deadline(best_candidate);
        return best_candidate;
    }
    
    return NULL; // No urgent gaming process found
}

/**
 * Handle gaming process frame deadline
 */
static void handle_frame_deadline(process_t* proc) {
    if (!proc || !proc->sched_entity) return;
    
    sched_entity_t* se = (sched_entity_t*)proc->sched_entity;
    gaming_context_t* gc = (gaming_context_t*)proc->gaming_context;
    
    if (!gc) return;
    
    uint64_t now = get_timestamp_ns();
    
    if (now <= se->frame_deadline_ns) {
        // Frame deadline met
        gc->perf.frames_rendered++;
        g_gaming_scheduler.total_frame_deadlines_met++;
        
        // Update frame timing statistics
        if (gc->perf.frame_start_ns > 0) {
            gc->perf.frame_duration_ns = now - gc->perf.frame_start_ns;
            update_gaming_performance_stats(proc);
        }
        
        // Apply frame pacing if enabled
        if (gc->perf.frame_pacing_enabled) {
            frame_pacing_algorithm(proc);
        }
        
        gc->frame_deadline_missed = false;
        
    } else {
        // Frame deadline missed
        gc->perf.frames_dropped++;
        g_gaming_scheduler.total_frame_deadlines_missed++;
        gc->frame_deadline_missed = true;
        
        // Apply emergency priority boost
        se->priority = -20; // Maximum priority
        boost_input_priority(proc);
        
        vga_puts("Gaming: Frame deadline MISSED - emergency boost applied\n");
    }
    
    // Set next frame deadline
    gc->perf.frame_start_ns = now;
    se->frame_deadline_ns = now + gc->perf.frame_target_ns;
}

/**
 * Boost input processing priority
 */
static void boost_input_priority(process_t* proc) {
    if (!proc || !proc->sched_entity) return;
    
    sched_entity_t* se = (sched_entity_t*)proc->sched_entity;
    gaming_context_t* gc = (gaming_context_t*)proc->gaming_context;
    
    if (!gc) return;
    
    // Apply input processing boost
    se->priority = g_gaming_config.input_boost_priority;
    se->boost_count++;
    
    // Set boost expiry
    gc->boost_expiry_ns = get_timestamp_ns() + g_gaming_config.input_boost_duration_ns;
    
    // Move to preferred input CPU
    if (gc->preferred_input_cpu != se->last_cpu) {
        migrate_process(proc, gc->preferred_input_cpu);
    }
    
    // Update input latency tracking
    if (gc->perf.last_input_timestamp_ns > 0) {
        uint64_t now = get_timestamp_ns();
        gc->perf.input_to_render_latency_ns = now - gc->perf.last_input_timestamp_ns;
    }
}

/**
 * Detect if a process is a gaming process
 */
static bool detect_gaming_process(process_t* proc) {
    if (!proc) return false;
    
    // Check process name against gaming patterns
    // This is a simplified implementation - real detection would be more sophisticated
    
    // For now, assume any process with high CPU usage and frequent context switches
    // combined with real-time characteristics is a gaming process
    sched_entity_t* se = (sched_entity_t*)proc->sched_entity;
    
    if (se->cpu_usage_percent > 30 &&                    // High CPU usage
        se->voluntary_switches > 100 &&                  // Frequent yields (frame boundaries)
        se->sched_class == SCHED_CLASS_INTERACTIVE) {    // Interactive process
        return true;
    }
    
    return false;
}

/**
 * Setup gaming context for a process
 */
static void setup_gaming_context(process_t* proc) {
    if (!proc) return;
    
    // Allocate gaming context
    gaming_context_t* gc = (gaming_context_t*)kmalloc(sizeof(gaming_context_t));
    if (!gc) return;
    
    memset(gc, 0, sizeof(gaming_context_t));
    proc->gaming_context = gc;
    
    // Initialize gaming context
    gc->is_gaming_process = true;
    gc->base_priority = -10;  // High gaming priority
    gc->gaming_cpu_mask = g_gaming_scheduler.gaming_cpu_mask;
    gc->exclusive_cpu_mode = g_gaming_config.exclusive_cpu_mode;
    
    // Performance tracking initialization
    gc->perf.frame_rate_target = g_gaming_config.frame_rate_target;
    gc->perf.frame_target_ns = calculate_frame_budget(gc->perf.frame_rate_target);
    gc->perf.frame_pacing_enabled = true;
    gc->perf.vsync_enabled = false; // Detect from system
    
    // CPU assignment for gaming threads
    gc->preferred_render_cpu = 0;  // First CPU for rendering
    gc->preferred_input_cpu = 1;   // Second CPU for input
    
    // Frame scheduling
    gc->next_frame_deadline_ns = get_timestamp_ns() + gc->perf.frame_target_ns;
    gc->frame_budget_ns = gc->perf.frame_target_ns * 80 / 100; // 80% of frame time
    
    vga_puts("Gaming context initialized for process\n");
}

/**
 * Optimize CPU affinity for gaming process
 */
static void optimize_gaming_cpu_affinity(process_t* proc) {
    if (!proc || !proc->sched_entity) return;
    
    sched_entity_t* se = (sched_entity_t*)proc->sched_entity;
    gaming_context_t* gc = (gaming_context_t*)proc->gaming_context;
    
    if (!gc) return;
    
    // Set CPU affinity to gaming CPUs only
    se->cpu_affinity = gc->gaming_cpu_mask;
    
    // Prefer high-performance cores for gaming
    se->preferred_cpu = gc->preferred_render_cpu;
    
    // Migrate to preferred CPU if necessary
    if (se->last_cpu != se->preferred_cpu && 
        CPU_ISSET(se->preferred_cpu, se->cpu_affinity)) {
        migrate_process(proc, se->preferred_cpu);
    }
}

/**
 * Calculate frame budget for given FPS
 */
static uint64_t calculate_frame_budget(uint32_t target_fps) {
    if (target_fps == 0) target_fps = 60;
    return 1000000000ULL / target_fps; // nanoseconds per frame
}

/**
 * Update gaming performance statistics
 */
static void update_gaming_performance_stats(process_t* proc) {
    if (!proc) return;
    
    gaming_context_t* gc = (gaming_context_t*)proc->gaming_context;
    if (!gc) return;
    
    // Calculate current frame rate
    if (gc->perf.frame_duration_ns > 0) {
        gc->perf.frame_rate_current = 1000000000ULL / gc->perf.frame_duration_ns;
    }
    
    // Update smoothed frame time (exponential moving average)
    if (gc->perf.smooth_frame_time_ns == 0) {
        gc->perf.smooth_frame_time_ns = gc->perf.frame_duration_ns;
    } else {
        // Alpha = 0.1 for smoothing
        gc->perf.smooth_frame_time_ns = 
            (gc->perf.smooth_frame_time_ns * 9 + gc->perf.frame_duration_ns) / 10;
    }
    
    // Calculate frame time variance for jitter detection
    if (gc->perf.smooth_frame_time_ns > 0) {
        int64_t variance = (int64_t)gc->perf.frame_duration_ns - 
                          (int64_t)gc->perf.smooth_frame_time_ns;
        gc->perf.frame_time_variance_ns = (variance < 0) ? -variance : variance;
    }
    
    // Update global gaming statistics
    g_gaming_scheduler.total_gaming_context_switches++;
}

/**
 * Frame pacing algorithm to reduce jitter
 */
static void frame_pacing_algorithm(process_t* proc) {
    if (!proc) return;
    
    gaming_context_t* gc = (gaming_context_t*)proc->gaming_context;
    if (!gc) return;
    
    uint64_t now = get_timestamp_ns();
    uint64_t frame_time = gc->perf.frame_duration_ns;
    uint64_t target_time = gc->perf.frame_target_ns;
    
    // If frame completed early, introduce controlled delay
    if (frame_time < target_time) {
        uint64_t delay_ns = target_time - frame_time;
        
        // Don't delay more than 1ms to maintain responsiveness
        if (delay_ns > 1000000) {
            delay_ns = 1000000;
        }
        
        // Set process to sleep for precise timing
        // This would typically use a high-resolution timer
        sched_entity_t* se = (sched_entity_t*)proc->sched_entity;
        se->time_quantum_remaining_ns = delay_ns;
    }
    
    // Adjust frame target based on system performance
    if (gc->perf.frame_time_variance_ns > target_time / 10) { // 10% variance threshold
        // High jitter detected - slightly increase frame budget
        gc->perf.frame_target_ns = (gc->perf.frame_target_ns * 101) / 100; // 1% increase
    } else if (gc->perf.frame_time_variance_ns < target_time / 50) { // 2% variance threshold
        // Low jitter - can tighten frame budget
        gc->perf.frame_target_ns = (gc->perf.frame_target_ns * 999) / 1000; // 0.1% decrease
    }
}

/**
 * Gaming scheduler tick - called more frequently during gaming mode
 */
void gaming_scheduler_tick(uint32_t cpu_id) {
    if (!g_gaming_config.enabled || cpu_id >= MAX_CPUS) return;
    
    cpu_runqueue_t* rq = &g_scheduler.cpu_runqueues[cpu_id];
    uint64_t now = get_timestamp_ns();
    
    // Check all gaming processes for deadline urgency
    for (uint32_t i = 0; i < g_gaming_scheduler.gaming_process_count; i++) {
        process_t* proc = g_gaming_scheduler.gaming_processes[i];
        if (!proc || !proc->sched_entity) continue;
        
        sched_entity_t* se = (sched_entity_t*)proc->sched_entity;
        gaming_context_t* gc = (gaming_context_t*)proc->gaming_context;
        
        if (!gc) continue;
        
        // Check for frame deadline urgency
        if (se->frame_deadline_ns > 0 && 
            se->frame_deadline_ns <= now + US_TO_NS(100)) { // 100µs urgency
            
            // Boost priority for urgent deadline
            se->priority = -20;
            
            // Move to gaming queue if not already there
            if (proc->state == PROCESS_STATE_READY) {
                scheduler_enqueue_task(proc, se->preferred_cpu);
            }
        }
        
        // Check for input boost expiry
        if (gc->boost_expiry_ns > 0 && now >= gc->boost_expiry_ns) {
            se->priority = gc->base_priority;
            gc->boost_expiry_ns = 0;
        }
        
        // Update performance statistics
        update_gaming_performance_stats(proc);
    }
}

/**
 * Register input event for gaming latency tracking
 */
void gaming_register_input_event(process_t* proc) {
    if (!proc) return;
    
    gaming_context_t* gc = (gaming_context_t*)proc->gaming_context;
    if (!gc) return;
    
    gc->perf.last_input_timestamp_ns = get_timestamp_ns();
    gc->perf.input_events_processed++;
    
    // Apply input boost
    boost_input_priority(proc);
}

/**
 * Register VSync event for frame synchronization
 */
void gaming_register_vsync_event(void) {
    uint64_t now = get_timestamp_ns();
    g_gaming_scheduler.last_vsync_ns = now;
    
    // Synchronize all gaming processes to VSync
    for (uint32_t i = 0; i < g_gaming_scheduler.gaming_process_count; i++) {
        process_t* proc = g_gaming_scheduler.gaming_processes[i];
        if (!proc || !proc->gaming_context) continue;
        
        gaming_context_t* gc = (gaming_context_t*)proc->gaming_context;
        gc->perf.vsync_enabled = true;
        gc->perf.vsync_timestamp_ns = now;
        
        // Align frame deadlines to VSync
        sched_entity_t* se = (sched_entity_t*)proc->sched_entity;
        se->frame_deadline_ns = now + gc->perf.frame_target_ns;
    }
}

/**
 * Get gaming performance statistics
 */
void gaming_get_performance_stats(gaming_perf_tracker_t* stats) {
    if (!stats) return;
    
    memset(stats, 0, sizeof(gaming_perf_tracker_t));
    
    // Aggregate statistics from all gaming processes
    for (uint32_t i = 0; i < g_gaming_scheduler.gaming_process_count; i++) {
        process_t* proc = g_gaming_scheduler.gaming_processes[i];
        if (!proc || !proc->gaming_context) continue;
        
        gaming_context_t* gc = (gaming_context_t*)proc->gaming_context;
        
        stats->frames_rendered += gc->perf.frames_rendered;
        stats->frames_dropped += gc->perf.frames_dropped;
        stats->input_events_processed += gc->perf.input_events_processed;
        
        // Take maximum latencies
        if (gc->perf.input_to_render_latency_ns > stats->input_to_render_latency_ns) {
            stats->input_to_render_latency_ns = gc->perf.input_to_render_latency_ns;
        }
        if (gc->perf.frame_time_variance_ns > stats->frame_time_variance_ns) {
            stats->frame_time_variance_ns = gc->perf.frame_time_variance_ns;
        }
    }
    
    // Calculate averages
    if (g_gaming_scheduler.gaming_process_count > 0) {
        stats->input_to_render_latency_ns /= g_gaming_scheduler.gaming_process_count;
        stats->frame_time_variance_ns /= g_gaming_scheduler.gaming_process_count;
    }
}

/**
 * Cleanup gaming context when process exits
 */
void gaming_cleanup_process(process_t* proc) {
    if (!proc || !proc->gaming_context) return;
    
    // Remove from gaming process list
    uint64_t flags;
    spin_lock_irqsave(&g_gaming_scheduler.lock, &flags);
    
    for (uint32_t i = 0; i < g_gaming_scheduler.gaming_process_count; i++) {
        if (g_gaming_scheduler.gaming_processes[i] == proc) {
            // Shift remaining processes down
            for (uint32_t j = i; j < g_gaming_scheduler.gaming_process_count - 1; j++) {
                g_gaming_scheduler.gaming_processes[j] = 
                    g_gaming_scheduler.gaming_processes[j + 1];
            }
            g_gaming_scheduler.gaming_process_count--;
            break;
        }
    }
    
    spin_unlock_irqrestore(&g_gaming_scheduler.lock, flags);
    
    // Free gaming context
    kfree(proc->gaming_context);
    proc->gaming_context = NULL;
    
    if (proc->sched_entity) {
        ((sched_entity_t*)proc->sched_entity)->gaming_mode = false;
    }
}