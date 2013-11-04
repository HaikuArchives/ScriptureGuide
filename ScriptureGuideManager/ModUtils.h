#ifndef MODUTILS_H
#define MODUTILS_H

#include <stdio.h>
#include <String.h>
#include <Archivable.h>
#include <Message.h>

#define SG_SETTINGS_PATH "/boot/home/config/settings/scripture-guide/"
#define SG_PKGCACHE_PATH "/boot/home/config/settings/scripture-guide/packages/"
#define SG_PKGINFO_PATH "/boot/home/config/settings/scripture-guide/package-info/"
#define SG_MODULEBASE_PATH "/boot/home/config/add-ons/scripture-guide/"
#define SG_DOWNLOAD_PATH "http://www.crosswire.org/ftpmirror/pub/sword/packages/rawzip/"
class ConfigFile : public BArchivable
{
public:
	ConfigFile(void);
	~ConfigFile(void);
	ConfigFile(BMessage *archive);
	status_t Archive(BMessage *archive, bool deep=true) const;
	static BArchivable *Instantiate(BMessage *archive);
	
	// There are a great many more attributes kept in a .conf file, but
	// these are the ones we are most concerned with.
	BString fFileName;
	BString fZipFileName;
	BString fAbout;
	BString fDataPath;
	BString fDescription;
	BString fLanguage;
	BString fLicense;
	float fFileSize;
};

status_t FilterConfigFile(const ConfigFile &cfile);

bool IsInstalled(const char *name);

status_t InstallModule(const char *name);
status_t UninstallModule(const char *name);
status_t InstallFromDisk(const char *path);

status_t ReadConfigFile(const char *name, ConfigFile &cfile);

//const char *TranslateLanguageName(const BString &string);

#endif
