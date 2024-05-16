#ifndef PLATFORM_PREFS_H
#define PLATFORM_PREFS_H

const char *pref_data_dir(void);

void pref_save_data_dir(const char *data_dir);

const char *pref_user_dir(void);

void pref_save_user_dir(const char *user_dir);

#endif // PLATFORM_PREFS_H
