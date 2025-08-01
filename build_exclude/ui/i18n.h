#ifndef I18N_H
#define I18N_H

#include <stdint.h>

// Initialize internationalization framework
void i18n_init(void);

// Set current locale
void i18n_set_locale(const char* locale_code);

// Get translated string
const char* i18n_get_string(const char* key);

#endif // I18N_H
