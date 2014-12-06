#include <stdio.h>
#include <stdlib.h>
#include <Entry.h>
#include <File.h>
#include <Message.h>
#include <String.h>
#include <Directory.h>
#include <List.h>
#include <Application.h>

#include "ModUtils.h"
#include "MainWindow.h"
#include "DownloadLocations.h"

BList gFileNameList;
BList gFileSizeList;
BList fConfFileList;

class SGMApp : public BApplication
{
public:
	SGMApp(void);
	~SGMApp(void);

	status_t TokenizeWords(const char *source, BList *stringarray, const char *tokenstr);
	void SetupPackageList(void);
};

SGMApp::SGMApp(void)
 : BApplication("application/x-vnd.wgp.ScriptureGuideManager")
{
	SetupPackageList();
	MainWindow *win=new MainWindow(BRect(300,200,900,600));
	win->Show();
}

SGMApp::~SGMApp(void)
{
}

void SGMApp::SetupPackageList(void)
{
	// The package info is kept in a subfolder of the regular scripture guide settings
	// The information for each package is kept in a flattened BMessage which has
	// the same name as the zip file kept on the Crosswire FTP site, complete with
	// case. Inside this message is kept the necessary information for the user
	// to be able to decide whether or not the module should be installed.
	
	BEntry entry(SG_SETTINGS_PATH);
	BDirectory dir(SG_PKGINFO_PATH);
	BFile file;
	
	if(entry.InitCheck()!=B_OK)
	{
		printf("Couldn't read package file\n");
		return;
	}
	
	if(!entry.Exists())
		create_directory(SG_SETTINGS_PATH,0777);
	
	entry.SetTo(SG_PKGINFO_PATH);
	if(!entry.Exists())
		create_directory(SG_PKGINFO_PATH,0777);
	
	// Check for the index file
	entry.SetTo("./index.html");
	if(!entry.Exists())
		system("wget " SG_DOWNLOAD_PKGS);
		
	entry.SetTo("./packagelist.txt");
	if(!entry.Exists())
	{
		// Now we get the package data from the directory index file
		system("grep -o -E rawzip/.*\\.zip\\\" index.html > packagelist.txt");
		system("grep -o -E [0-9]+\\.[0-9]\\ kb index.html > packagesizes.txt");
	}
	
	entry.SetTo("mods.d.tar.gz");
	if(!entry.Exists())
		system("wget " SG_DOWNLOAD_MODS);
	
	entry.SetTo(SG_PKGINFO_PATH "configfiles");
	if(!entry.Exists())
	{
		// Get and unpack the compressed list of config files. There should be more config
		// files than there are modules or at least just as many.
		system("tar -xvpzf mods.d.tar.gz -C " SG_PKGINFO_PATH);
		
		entry.SetTo(SG_PKGINFO_PATH "mods.d");
		entry.Rename("configfiles");
	}
	
	dir.SetTo(SG_PKGINFO_PATH);
	if(dir.CountEntries()==1)
	{
		BFile file("packagelist.txt",B_READ_ONLY);
		off_t filesize;
		BString filedata;
		
		file.GetSize(&filesize);
		
		if(filesize<=0)
		{
			printf("Package list file size is 0\n");
			return;
		}
		
		char *data=new char[filesize+1];
		
		file.Seek(0,SEEK_SET);
		file.Read(data,filesize);
		file.Unset();
		
		filedata.SetTo(data);
		filedata.RemoveAll("\"");
		filedata.RemoveAll("rawzip/");
		TokenizeWords(filedata.String(),&gFileNameList,"\n");
		delete [] data;
		
		file.SetTo("packagesizes.txt",B_READ_ONLY);
		file.GetSize(&filesize);
		
		data=new char[filesize+1];
		file.Seek(0,SEEK_SET);
		file.Read(data,filesize);
		file.Unset();
		
		filedata.SetTo(data);
		filedata.RemoveAll(" kb");
		TokenizeWords(filedata.String(),&gFileSizeList,"\n");
		delete [] data;
		
		// Now that we have the list of filenames, we iterate through the list
		// of filenames and derive the name of the config file by removing the .zip
		// and making it all lowercase. Then, we read the config file, parse it, 
		// save it to the global config list and also save the info to package info
		// file, as well.
		
		for(int32 i=0; i<gFileNameList.CountItems(); i++)
		{
			BString *currentfile=(BString*) gFileNameList.ItemAt(i);
			if(!currentfile)
				continue;
			
			BString *filesizeentry=(BString*) gFileSizeList.ItemAt(i);
			if(!filesizeentry)
				continue;
			
			if(filesizeentry->Compare("0.0")==0)
			{
				printf("%s is an empty file\n", currentfile->String());
				continue;
			}
			
			BString filename=*currentfile;
			filename.RemoveAll(".zip");
			filename.ToLower();
			
			BString conffilename(filename);
			conffilename+=".conf";
			conffilename.Prepend(SG_PKGINFO_PATH "configfiles/");
			
			entry.SetTo(conffilename.String());
			if(entry.InitCheck()!=B_OK)
				continue;
			
			if(!entry.Exists())
			{
				printf("Entry %s doesn't exist\n",conffilename.String());
				continue;
			}
			
			ConfigFile configfile;
			if(ReadConfigFile(conffilename.String(),configfile)!=B_OK)
				continue;
			
			configfile.fZipFileName=currentfile->String();
			configfile.fZipFileName.RemoveAll(".zip");
			sscanf(filesizeentry->String(),"%f",&(configfile.fFileSize));
			
			if(FilterConfigFile(configfile)!=B_OK)
				continue;
			
			BMessage message;
			configfile.Archive(&message);

			conffilename=SG_PKGINFO_PATH;
			conffilename+=filename;
			
			file.SetTo(conffilename.String(),B_READ_WRITE|B_CREATE_FILE|B_ERASE_FILE);
			message.Flatten(&file);
		}
	}
	
	dir.SetTo(SG_PKGINFO_PATH);
	entry_ref ref;
			
	int32 entrycount=dir.CountEntries();
	for(int32 i=0; i<entrycount; i++)
	{
		if(dir.GetNextEntry(&entry)==B_OK && entry.InitCheck()==B_OK)
		{
			entry.GetRef(&ref);
			entry.Unset();
			
			BFile file(&ref,B_READ_ONLY);
			BMessage msg;
			
			if(msg.Unflatten(&file)==B_OK)
			{
				ConfigFile *conffile=(ConfigFile*)ConfigFile::Instantiate(&msg);
				if(conffile)
					fConfFileList.AddItem(conffile);
			}
		}
	}

}

status_t SGMApp::TokenizeWords(const char *source, BList *stringarray, const char *tokenstr)
{
	if(!source || !stringarray || !tokenstr || !stringarray->IsEmpty())
		return B_BAD_VALUE;
	
	// convert all tabs to spaces and eliminate consecutive spaces so that we can 
	// easily use strtok() 
	BString bstring(source);
	bstring.ReplaceAll('\t',' ');
	bstring.ReplaceAll("  "," ");

	char *workstr=new char[strlen(source)+1];
	strcpy(workstr,bstring.String());
	strtok(workstr,tokenstr);
	
	char *token=strtok(NULL,tokenstr),*lasttoken=workstr;
	
	if(!token)
	{
		delete workstr;
		stringarray->AddItem(new BString(bstring));
		return B_OK;
	}
	
	int32 length;
	BString *newword;
	
	while(token)
	{
		length=token-lasttoken;
		
		newword=new BString(lasttoken,length+1);
		lasttoken=token;
		stringarray->AddItem(newword);

		token=strtok(NULL,tokenstr);
	}
	
	length=strlen(lasttoken);
	newword=new BString(lasttoken,length+1);
	lasttoken=token;
	stringarray->AddItem(newword);
	
	delete [] workstr;

	return B_OK;
}

int main(void)
{
	SGMApp app;
	app.Run();
	return 0;
}

