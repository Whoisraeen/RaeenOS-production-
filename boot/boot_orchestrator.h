/**
 * @file boot_orchestrator.h
 * @brief RaeenOS Boot Sequence Orchestrator Header
 * 
 * Defines the interface for the boot orchestrator that coordinates
 * the complete boot process from kernel to desktop.
 */

#ifndef BOOT_ORCHESTRATOR_H
#define BOOT_ORCHESTRATOR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Main boot sequence entry point
 * Called after kernel initialization is complete
 */
void boot_orchestrator_main(void);

/**
 * Check if the system requires first-time setup (OOBE)
 * @return true if OOBE is required, false otherwise
 */
bool boot_check_oobe_required(void);

/**
 * Handle boot errors gracefully
 * @param stage The boot stage where the error occurred
 * @param error Error description
 */
void boot_handle_error(const char* stage, const char* error);

/**
 * Enter recovery mode
 */
void boot_enter_recovery_mode(void);

/**
 * Start OOBE (Out-of-Box Experience)
 */
void boot_start_oobe(void);

/**
 * Start desktop environment
 */
void boot_start_desktop(void);

#ifdef __cplusplus
}
#endif

#endif // BOOT_ORCHESTRATOR_H