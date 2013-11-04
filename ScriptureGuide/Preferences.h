#ifndef PREFERENCES_H_
#define PREFERENCES_H_

#include <Locker.h>
#include <Message.h>
#include <File.h>

extern BLocker prefsLock;
extern BLocker modPrefsLock;
extern BMessage preferences;

status_t SavePreferences(const char *path);
status_t LoadPreferences(const char *path);

status_t SaveModulePreferences(const char *module, BMessage *msg);
status_t LoadModulePreferences(const char *module, BMessage *msg);


#endif
