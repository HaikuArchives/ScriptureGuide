#include <Application.h>
#include <Message.h>
#include <Messenger.h>
#include <Alert.h>
#include <Roster.h>
#include <Window.h>
#include <Directory.h>
#include <Entry.h>
#include <Font.h>
#include <Path.h>

#include <string.h>
#include <String.h>
#include <stdlib.h>
#include <stdio.h>

#include "constants.h"
#include "LogosApp.h"
#include "LogosMainWindow.h"
#include "Preferences.h"

// Global containing the startup path. Accessed via GetAppPath()
BString gAppPath;
bool gDocsAvailable;

BRect windowRect(50,50,749,449);

SGApp::SGApp()
  : BApplication("application/x-vnd.Scripture-Guide")
{
	if (StartupCheck() == B_OK)
	{
		BString module, verseKey;
		
		prefsLock.Lock();
		
		if (preferences.FindRect("windowframe",&windowRect) != B_OK)
			windowRect.Set(50,50,749,449);
		
		if (preferences.FindString("module",&module) != B_OK)
			module = "WEB";
		
		if (preferences.FindString("key",&verseKey) != B_OK)
			verseKey="Gen 1:1";
		
		prefsLock.Unlock();
		
		// opens the main window with the current options
		SGMainWindow *win = new SGMainWindow(windowRect, module.String(), verseKey.String());
		win->Show();
	}
	else
	{
		// If we don't have B_OK, it means that StartupCheck found some problems
		// but couldn't fix them and told the user about it
		PostMessage(B_QUIT_REQUESTED);
	}
}

SGApp::~SGApp(void)
{
	SavePreferences(PREFERENCES_FILE);
}

void SGApp::MessageReceived(BMessage *message) 
{
	switch(message->what) 
	{
		case M_WINDOW_CLOSED:
		{
			if (CountWindows() <= 1)
				PostMessage(B_QUIT_REQUESTED);
			break;
		}
		default:
			BApplication::MessageReceived(message);
			break;
	}
}

/*
	This function makes sure that quite a few of the conditions needed for proper operation
	of the app are met. Certain problems are fixed, such as missing preferences. Fatal
	errors are reported to the user and B_ERROR is returned so that the app will quit without
	crashing.
*/
status_t SGApp::StartupCheck(void)
{
	// Check to see if the preferences file and/or folder exists. If they don't,
	// we will create the preferences folder
	BEntry prefsFolder(PREFERENCES_PATH),prefsFile(PREFERENCES_FILE);
	bool useDefaultPrefs=false;
	
	if (!prefsFolder.Exists())
	{
		useDefaultPrefs=true;
		create_directory(PREFERENCES_PATH,0777);
	}
	else
	if (!prefsFile.Exists())
		useDefaultPrefs=true;
	else 
	if (LoadPreferences(PREFERENCES_FILE)!=B_OK)
		useDefaultPrefs=true;

	if (useDefaultPrefs)
	{
		prefsLock.Lock();
		
		preferences.MakeEmpty();
		
		preferences.AddBool("linebreaks",false);
		preferences.AddInt8("fontsize",12);
		preferences.AddRect("windowframe",BRect(50,50,749,449));
		preferences.AddInt16("module",0);
		preferences.AddString("key","Gen 1:1");
		SavePreferences(PREFERENCES_FILE);
		
		prefsLock.Unlock();
	}
	
	// release the file handles kept by these two variables
	prefsFolder.Unset();
	prefsFile.Unset();
	
	BEntry entry;

	// Set the startup path variable
	app_info ai;
	be_app->GetAppInfo(&ai);
			
	entry.SetTo(&ai.ref);
	if (entry.InitCheck()==B_OK)
	{
		BEntry parent;
				
		entry.GetParent(&parent);
		BPath path;
		parent.GetPath(&path);
		
		gAppPath=path.Path();
		gAppPath+="/";
	}
		
	// Check for the existence of the modules, documentation, and notes
	
	// The original location for everything was home/.sword/ for everything. As a result,
	// documentation were at /boot/home/.sword/docs/help/,
	// module folders were /boot/home/.sword/modules and /boot/home/.sword/mods.d,
	// and notes were kept in /boot/home/.sword/notes/,
	
	// We're going to check for this. If this ever happens, it'll take a little longer to 
	// get to the point where the user can do something, but it'll ultimately be a good thing
	
	// Check for the existence of the new modules directory
	BDirectory dir;
	entry.SetTo(MODULES_PATH);
	bool modfail=false;
	BAlert *alert=NULL;
	
	if (!entry.Exists())
		create_directory(MODULES_PATH,0777);

	entry.SetTo("/boot/home/config/non-packaged/data/sword/mods.d");
	if (!entry.Exists())
	{
		// This is either an upgrade or the modules don't exist. 
		entry.SetTo("/boot/home/.sword/mods.d");
		if (!entry.Exists())
		{
			// This is an upgrade from 0.7, so we're going to get into the moving business --
			// and to think that I was just a codemonkey. :P
			
			dir.SetTo(MODULES_PATH);
			entry.MoveTo(&dir,NULL,true);
		}
		else
			modfail=true;
	}
	
	entry.SetTo("/boot/home/config/non-packaged/data/sword/modules");
	if (!entry.Exists())
	{
		entry.SetTo("/boot/home/.sword/modules");
		if (!entry.Exists())
		{
			dir.SetTo(MODULES_PATH);
			entry.MoveTo(&dir,NULL,true);
		}
		else
			modfail=true;
	}
	
	if (modfail)
	{
		alert=new BAlert("Scripture Guide","Scripture Guide could not find any books -- no Bibles,"
			" commentaries, or anything else. You need to have at least one book from the"
			" SWORD Project in order to run Scripture Guide. Would you like to go to their website"
			" now, look at Scripture Guide's installation notes, or just quit?\n","Website",
			"Install Notes","Quit");
		int32 value=alert->Go();
		
		if (value==0) {
			const char* url = "http://www.crosswire.org/sword/";
			be_roster->Launch("text/html", 1, (char**)&url);
		} else if (value==1) {
			BString string(GetAppPath());
			string+="INSTALL.htm";
			const char* file = string.String();
			be_roster->Launch("text/html", 1, (char**)&file);
		}
		return B_ERROR;
	}
	
	// Check for existence of documentation
	gDocsAvailable=true;
	
	BString docspath(gAppPath);
	docspath+="docs/help";
	entry.SetTo(docspath.String());
	if (entry.Exists())
	{
		// The folder exists, so check to see if the index file is available
		docspath+="/index.html";
		entry.SetTo(docspath.String());
		if (!entry.Exists())
			gDocsAvailable=false;
	}
	else
		gDocsAvailable=false;
	
	// Ensure that the study notes exist
	BString notespath(gAppPath);
	notespath+="notes";
	entry.SetTo(notespath.String());
	if (!entry.Exists())
	{
		// Notes folder has probably been blown away, so create a new folder
		// and a new default notes file.
		create_directory(notespath.String(),0777);
		
		notespath+="/Notes.txt";
		BFile file(notespath.String(), B_READ_WRITE | B_CREATE_FILE);
		const char notes[]="Scripture Guide Study Notes\n-------------------------------\n";
		file.Write(notes,strlen(notes)+1);
		file.Unset();
	}
	else
	{
		// folder exists, but does the notefile exist?
		notespath+="/Notes.txt";
		entry.SetTo(notespath.String());
		if (!entry.Exists())
		{
			BFile file(notespath.String(), B_READ_WRITE | B_CREATE_FILE);
			const char notes[]="Scripture Guide Study Notes\n-------------------------------\n";
			file.Write(notes,strlen(notes));
			file.Unset();
		}
	}
	
	return B_OK;
}

// An easy function to access the path that the file run from
const char *GetAppPath(void)
{
	return gAppPath.String();
}

// Global read-only access function to know if the help files exist
bool HelpAvailable(void)
{
	return gDocsAvailable;
}

int main(void) 
{
	SGApp theApp;
	theApp.Run();
	return 0;
}
