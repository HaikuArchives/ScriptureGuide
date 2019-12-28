#include "Preferences.h"
#include "constants.h"
#include <String.h>

BLocker modPrefsLock;
BLocker prefsLock;
BMessage preferences;

status_t SavePreferences(const char* path)
{
	if (!path)
		return B_ERROR;
	
	prefsLock.Lock();
	
	BFile file(path, B_READ_WRITE | B_ERASE_FILE | B_CREATE_FILE);
	
	status_t status = file.InitCheck();
	if (status!=B_OK)
		return status;
	
	status = preferences.Flatten(&file);
		
	prefsLock.Unlock();
	return status;
}


status_t LoadPreferences(const char* path)
{
	if (!path)
		return B_ERROR;
	
	prefsLock.Lock();
	
	BFile file(path, B_READ_ONLY);

	BMessage msg;
	
	status_t status = file.InitCheck();
	if (status!=B_OK)
		return status;
	
	status = msg.Unflatten(&file);
	if (status==B_OK)
		preferences = msg;
		
	prefsLock.Unlock();
	return status;
	
}


status_t SaveModulePreferences(const char* module, BMessage* msg)
{
	if (!module || !msg)
		return B_ERROR;
	
	modPrefsLock.Lock();
	
	BString path(PREFERENCES_PATH);
	path+=module;

	BFile file(path.String(), B_READ_WRITE | B_ERASE_FILE | B_CREATE_FILE);
	
	status_t status=file.InitCheck();
	if (status!=B_OK)
	{
		modPrefsLock.Unlock();
		return status;
	}
	
	status = msg->Flatten(&file);
	modPrefsLock.Unlock();
	return status;
}


status_t LoadModulePreferences(const char* module, BMessage* msg)
{
	if (!module || !msg)
		return B_ERROR;
	
	modPrefsLock.Lock();

	BString path(PREFERENCES_PATH);
	path+=module;
	
	BFile file(path.String(),B_READ_ONLY);

	BMessage temp;
	
	status_t status = file.InitCheck();
	if (status != B_OK)
	{
		modPrefsLock.Unlock();
		return status;
	}
	
	status=temp.Unflatten(&file);
	if (status == B_OK)
		*msg = temp;

	modPrefsLock.Unlock();
	
	return status;
}
