#include "i18n.h"
#include "../vga.h"
#include "../string.h"

static const char* current_locale = "en_US"; // Default locale

// Simple key-value store for translations (placeholder)
typedef struct {
    const char* key;
    const char* en_US_value;
    const char* es_ES_value;
    const char* fr_FR_value;
} translation_entry_t;

static translation_entry_t translations[] = {
    {"hello", "Hello", "Hola", "Bonjour"},
    {"goodbye", "Goodbye", "Adiós", "Au revoir"},
    {"welcome", "Welcome to RaeenOS", "Bienvenido a RaeenOS", "Bienvenue sur RaeenOS"},
    {"file_not_found", "File not found", "Archivo no encontrado", "Fichier non trouvé"},
    {"error", "Error", "Error", "Erreur"},
    {NULL, NULL, NULL, NULL} // Sentinel
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
            if (strcmp(current_locale, "es_ES") == 0) {
                return translations[i].es_ES_value;
            } else if (strcmp(current_locale, "fr_FR") == 0) {
                return translations[i].fr_FR_value;
            } else {
                // Default to en_US if locale not found or not implemented
                return translations[i].en_US_value;
            }
        }
    }
    return key; // Return key if not found
}
