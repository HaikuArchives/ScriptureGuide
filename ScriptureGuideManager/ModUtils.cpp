#include "ModUtils.h"
#include <File.h>
#include <Entry.h>
#include <Directory.h>
#include <Entry.h>

ConfigFile::ConfigFile(void)
{
}

ConfigFile::ConfigFile(BMessage *archive)
 : BArchivable(archive)
{
	BMessage msg;
	
	archive->FindString("about",&fAbout);
	archive->FindString("description", &fDescription);
	archive->FindString("license",&fLicense);
	archive->FindString("datapath",&fDataPath);
	archive->FindString("language",&fLanguage);
	archive->FindString("filename",&fFileName);
	archive->FindString("zipfilename",&fZipFileName);
	archive->FindFloat("filesize",&fFileSize);
}

ConfigFile::~ConfigFile(void)
{
}

BArchivable *ConfigFile::Instantiate(BMessage *archive)
{
	if(validate_instantiation(archive,"ConfigFile"))
		return new ConfigFile(archive);
	return NULL;
}

status_t ConfigFile::Archive(BMessage *archive, bool deep) const
{
	status_t status=BArchivable::Archive(archive,deep);
	if(status!=B_OK)
		return status;
	
	archive->AddString("class","ConfigFile");
	archive->AddString("about",fAbout);
	archive->AddString("description",fDescription);
	archive->AddString("license",fLicense);
	archive->AddString("datapath",fDataPath);
	archive->AddString("language",fLanguage);
	archive->AddString("filename",fFileName);
	archive->AddString("zipfilename",fZipFileName);
	archive->AddFloat("filesize",fFileSize);
	return B_OK;
}

bool IsInstalled(const char *name)
{
	// Expects the name of the config file
	if(!name)
		return false;
	
	BString path(name);
	path.Prepend(SG_MODULEBASE_PATH "mods.d/");
	
	BEntry entry(path.String());
	return entry.Exists();
}

status_t InstallModule(const char *name)
{
	return B_ERROR;
}

status_t UninstallModule(const char *name)
{
	return B_ERROR;
}

status_t InstallFromDisk(const char *path)
{
	return B_ERROR;
}

status_t ReadConfigFile(const char *name, ConfigFile &cfile)
{
	if(!name)
		return B_ERROR;
		
	BEntry entry(name);
	status_t status;
	
	status=entry.InitCheck();
	if(status!=B_OK)
		return status;
	
	entry_ref ref;
	entry.GetRef(&ref);
	cfile.fFileName=ref.name;
	
	BFile file(&entry,B_READ_ONLY);
	status=file.InitCheck();
	if(status!=B_OK)
		return status;
	
	off_t filesize;
	file.GetSize(&filesize);
	if(filesize<1)
	{
		status=B_BAD_VALUE;
		return status;
	}
	
	BString filecontents;
	
	char *filedata=new char[filesize];
	file.Read(filedata,filesize);
	
	filecontents=filedata;
	
	delete [] filedata;
	
	int32 offset=0,lineend=0;
	
	filecontents.RemoveAll("\r");
	
	// This is to make sure that the file ends in a newline. If a .conf
	// file doesn't and one of these entries is at the end, it will crash. :(
	filecontents+="\n";
	
	offset=filecontents.FindFirst("Description=",0);
	if(offset!=B_ERROR)
	{
		offset+=12;
		lineend=filecontents.FindFirst("\n",offset);
		filecontents.CopyInto(cfile.fDescription,offset,lineend-offset);
	}
	
	offset=filecontents.FindFirst("About=",0);
	if(offset!=B_ERROR)
	{
		offset+=6;
		lineend=filecontents.FindFirst("\n",offset);
		filecontents.CopyInto(cfile.fAbout,offset,lineend-offset);
		cfile.fAbout.ReplaceAll("\\par ","\n");
		cfile.fAbout.ReplaceAll("\\par","\n");
		
		// TODO: See what else needs to be replaced or stripped out in
		// descriptions
	}
	
	offset=filecontents.FindFirst("DataPath=",0);
	if(offset!=B_ERROR)
	{
		offset+=9;
		lineend=filecontents.FindFirst("\n",offset);
		filecontents.CopyInto(cfile.fDataPath,offset,lineend-offset);
		
		if(cfile.fDataPath.ByteAt(0)=='.')
			cfile.fDataPath.RemoveFirst(".");
		if(cfile.fDataPath.ByteAt(0)=='/')
			cfile.fDataPath.RemoveFirst("/");
	}
	
	offset=filecontents.FindFirst("Lang=",0);
	if(offset!=B_ERROR)
	{
		offset+=5;
		lineend=filecontents.FindFirst("\n",offset);
		filecontents.CopyInto(cfile.fLanguage,offset,lineend-offset);
	}
	
	offset=filecontents.FindFirst("DistributionLicense=",0);
	if(offset!=B_ERROR)
	{
		offset+=20;
		lineend=filecontents.FindFirst("\n",offset);
		filecontents.CopyInto(cfile.fLicense,offset,lineend-offset);
	}
	
	return B_OK;
}

status_t FilterConfigFile(const ConfigFile &cfile)
{
	if(cfile.fLanguage.Compare("zh")==0)
		return B_ERROR;
	
	return B_OK;
}

/*
const char *TranslateLanguageName(const BString &string)
{
	char first, second;
	
	first=string.ByteAt(0);
	second=string.ByteAt(1);
	
	switch(first)
	{
		case
	}
}
*/
