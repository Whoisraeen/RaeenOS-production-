#include "i18n.h"
#include "../../vga.h"
#include "../../string.h"

static const char* current_locale = "en_US"; // Default locale

// Simple key-value store for translations (placeholder)
typedef struct {
    const char* key;
    const char* en_US_value;
    // Add more languages here
} translation_entry_t;

static translation_entry_t translations[] = {
    {"hello", "Hello"},
    {"goodbye", "Goodbye"},
    // Add more translations
    {NULL, NULL} // Sentinel
};

void i18n_init(void) {
    vga_puts("Internationalization framework initialized (placeholder).\n");
}

void i18n_set_locale(const char* locale_code) {
    current_locale = locale_code;
    vga_puts("Locale set to: ");
    vga_puts(current_locale);
    vga_puts("\n");
}

const char* i18n_get_string(const char* key) {
    for (int i = 0; translations[i].key != NULL; i++) {
        if (strcmp(translations[i].key, key) == 0) {
            // For now, always return English. Real implementation would check current_locale.
            return translations[i].en_US_value;
        }
    }
    return key; // Return key if not found
}
