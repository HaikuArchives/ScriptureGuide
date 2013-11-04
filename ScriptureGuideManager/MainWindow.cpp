#include <Application.h>
#include "MainWindow.h"
#include "ModUtils.h"
#include "MarkableItem.h"
#include <stdlib.h>

extern BList gFileNameList;
extern BList fConfFileList;

enum
{
	M_SELECT_MODULE='slmd',
	M_MARK_MODULE,
	M_SET_PACKAGES
};

MainWindow::MainWindow(BRect frame)
 :BWindow(frame,"Scripture-Guide Book Manager",B_DOCUMENT_WINDOW_LOOK,
  B_NORMAL_WINDOW_FEEL,0,0)
{
	BRect r(Bounds());
	
	fApplyThread=-1;
	
	BView *bgview=new BView(r,"background",B_FOLLOW_ALL,B_WILL_DRAW);
	bgview->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(bgview);
	
	r.bottom=20;
	
	// Set up menu
	
	BMenuBar *mbar=new BMenuBar(r,NULL);
	bgview->AddChild(mbar);
	
	BMenu *menu=new BMenu("Program");
	menu->AddItem(new BMenuItem("Quit",new BMessage(B_QUIT_REQUESTED),'Q',0));
	mbar->AddItem(menu);
	
	r.top=mbar->Frame().bottom+1;
	r.bottom=Bounds().bottom;
	r.right=200;
	
	r.bottom-=B_H_SCROLL_BAR_HEIGHT+2;
	
	// Set up the module list
	
	fListView=new BListView(r,"modlist",B_SINGLE_SELECTION_LIST,B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM);
	fListScrollView=new BScrollView("listscrollview",fListView,
		B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM,0,true,true);
	fListScrollView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	bgview->AddChild(fListScrollView);
	fListView->SetSelectionMessage(new BMessage(M_SELECT_MODULE));
	fListView->SetInvocationMessage(new BMessage(M_MARK_MODULE));
	
	for(int32 i=0; i<fConfFileList.CountItems(); i++)
	{
		ConfigFile *cfile=(ConfigFile*)fConfFileList.ItemAt(i);
		if(cfile)
		{
			BookItem *bookitem=new BookItem(cfile->fDescription.String(),cfile);
			fListView->AddItem(bookitem);
			if(IsInstalled(cfile->fFileName.String()))
				bookitem->SetMarked(true);
		}
	}
	
	// Add the box we use for descriptions
	r.left=fListScrollView->Frame().right+1;
	r.right=Bounds().right-B_V_SCROLL_BAR_WIDTH;
	r.top=mbar->Frame().bottom+1;
	r.bottom=Bounds().bottom/2;
	
	fTextView=new BTextView(r,"descriptionview",r.OffsetToCopy(0,0).InsetByCopy(5,5),
		B_FOLLOW_ALL);
	fTextView->MakeEditable(false);
	fTextScrollView=new BScrollView("textscrollview",fTextView,B_FOLLOW_LEFT|B_FOLLOW_TOP,
		0,false,true);
	fTextScrollView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	bgview->AddChild(fTextScrollView);
	
	fApplyButton=new BButton(BRect(0,0,0,0),"apply button", "Apply", 
		new BMessage(M_SET_PACKAGES), B_FOLLOW_NONE);
	fApplyButton->ResizeToPreferred();
	r=fApplyButton->Frame();
	
	float panelwidth=Bounds().right-fListScrollView->Frame().right;
	float panelheight=Bounds().bottom-fTextScrollView->Frame().bottom;
	fApplyButton->MoveTo(fListScrollView->Frame().right+ ((panelwidth-r.Width())*.8),
		fTextScrollView->Frame().bottom+ ((panelheight-r.Height())*2)/3 );
	fApplyButton->SetEnabled(false);
	
	bgview->AddChild(fApplyButton);
}

bool MainWindow::QuitRequested(void)
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

void MainWindow::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
		case M_SELECT_MODULE:
		{
			if(fApplyThread!=-1)
				break;
			
			BookItem *item=(BookItem*)fListView->ItemAt(fListView->CurrentSelection());
			if(item && item->File())
			{
				BString str(item->File()->fAbout);
				str << "\n\n" << "Archive Size: " << item->File()->fFileSize << " K";
				fTextView->SetText(str.String());
			}
			break;
		}
		case M_MARK_MODULE:
		{
			if(fApplyThread!=-1)
				break;
			
			BookItem *item=(BookItem*)fListView->ItemAt(fListView->CurrentSelection());
			if(item)
			{	
				// Installed: 	uncheck=uninstall
				//				check=remove from uninstall list
				
				// Not installed: 	uncheck=remove from install list
				// 					check=add to isntall list
				
				if(IsInstalled(item->File()->fFileName.String()))
				{
					if(item->IsMarked())
					{
						// uncheck item: uninstall
						item->SetMarked(false);
						fUninstallList.AddItem(item->File()->fZipFileName);
					}
					else
					{
						// check item: remove from uninstall list - make no changes
						item->SetMarked(true);
						fUninstallList.RemoveItem(item->File()->fZipFileName);
					}
				}
				else
				{
					if(item->IsMarked())
					{
						// uncheck item: make no changes to system
						item->SetMarked(false);
						fInstallList.RemoveItem(item->File()->fZipFileName);
					}
					else
					{
						// check item: install module
						item->SetMarked(true);
						fInstallList.AddItem(item->File()->fZipFileName);
					}
				}
				fListView->InvalidateItem(fListView->CurrentSelection());
			}
			if(fInstallList.CountItems()==0 && fUninstallList.CountItems()==0)
			{
				if(fApplyButton->IsEnabled())
					fApplyButton->SetEnabled(false);
			}
			else
			{
				if(!fApplyButton->IsEnabled())
					fApplyButton->SetEnabled(true);
			}
			break;
		}
		case M_SET_PACKAGES:
		{
			if(fApplyThread!=-1)
				break;
			
			fApplyThread=spawn_thread(ApplyThread,"applythread",B_NORMAL_PRIORITY,this);
			resume_thread(fApplyThread);
			
			break;
		}
		default:
			BWindow::MessageReceived(msg);
	}
}

int32 MainWindow::ApplyThread(void *data)
{
	MainWindow *win=(MainWindow*)data;
	
	int32 i, installcount,removecount;
	
	BString zipfilename, configpath, syscmd, displaystring;
	ConfigFile cfile;
	
	win->Lock();
	win->fApplyButton->SetEnabled(false);
	win->fTextView->SetText("");
	
	installcount=win->fInstallList.CountItems();
	removecount=win->fUninstallList.CountItems();
	
	win->Unlock();
	
	for(i=0; i<installcount; i++)
	{
		win->Lock();
		
		zipfilename=*win->fInstallList.ItemAt(i);
		configpath=zipfilename;
		configpath.ToLower();
		configpath+=".conf";
		configpath.Prepend(SG_PKGINFO_PATH "configfiles/");
		ReadConfigFile(configpath.String(),cfile);
		
		displaystring="Downloading ";
		displaystring << cfile.fDescription << "\n";
				
		win->fTextView->Insert(displaystring.String());
		win->Unlock();
		
		syscmd="wget --directory-prefix=" SG_PKGCACHE_PATH " " SG_DOWNLOAD_PATH;
		syscmd+=zipfilename;
		syscmd+=".zip";
		
		system(syscmd.String());
		
		win->Lock();
		displaystring="Installing ";
		displaystring << cfile.fDescription << "\n";
		win->fTextView->Insert(displaystring.String());
		win->Unlock();
		
		syscmd="unzip ";
		syscmd << SG_PKGCACHE_PATH << zipfilename << ".zip -d " << SG_MODULEBASE_PATH;
		
		system(syscmd.String());
		
		win->Lock();
		win->fTextView->Insert("Done\n");
		win->Unlock();
	}
	
	for(i=0; i<removecount; i++)
	{
		win->Lock();
		
		zipfilename=*win->fUninstallList.ItemAt(i);
		configpath=zipfilename;
		configpath.ToLower();
		configpath+=".conf";
		configpath.Prepend(SG_PKGINFO_PATH "configfiles/");
		ReadConfigFile(configpath.String(),cfile);
		
		displaystring="Removing ";
		displaystring << cfile.fDescription << "\n";
				
		win->fTextView->Insert(displaystring.String());
		win->Unlock();
		
		// The two things needed to remove a module:
		// delete mods.d/modulename.conf
		// delete datapath/*
		// remove datapath
		
		configpath=zipfilename;
		configpath.ToLower();
		
		syscmd="rm ";
		syscmd << SG_MODULEBASE_PATH << "mods.d/" << configpath << ".conf";
//		printf("%s\n",syscmd.String());
		system(syscmd.String());

		syscmd="rm -r ";
		syscmd << SG_MODULEBASE_PATH << cfile.fDataPath << "*";
//		printf("%s\n",syscmd.String());
		system(syscmd.String());
		
		syscmd="rmdir ";
		syscmd << SG_MODULEBASE_PATH << cfile.fDataPath;
//		printf("%s\n",syscmd.String());
		system(syscmd.String());
		
		win->Lock();
		win->fTextView->Insert("Done\n");
		win->Unlock();
	}
	
	
	win->Lock();
	win->fApplyButton->SetEnabled(true);
	win->fApplyThread=-1;
	win->fInstallList.MakeEmpty();
	win->fUninstallList.MakeEmpty();
	win->Unlock();

	exit_thread(B_OK);
	return 0;
}
