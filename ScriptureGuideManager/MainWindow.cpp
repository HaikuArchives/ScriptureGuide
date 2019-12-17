#include <Application.h>
#include <LayoutBuilder.h>

#include <stdlib.h>
#include <ColumnTypes.h>

#include "MainWindow.h"
#include "ModUtils.h"
#include "BookRow.h"
#include "DownloadLocations.h"

extern BList gFileNameList;
extern BList fConfFileList;

enum
{
	M_SELECT_MODULE='slmd',
	M_MARK_MODULE,
	M_SET_PACKAGES
};

MainWindow::MainWindow(BRect frame)
	: BWindow(frame, "ScriptureGuide Book Manager",B_DOCUMENT_WINDOW_LOOK,
 		B_NORMAL_WINDOW_FEEL, 0)
{
	fApplyThread=-1;
	
	// Set up menu
	BMenuBar *mbar=new BMenuBar("menu_bar");
	
	BMenu *menu=new BMenu("Program");
	menu->AddItem(new BMenuItem("Quit",new BMessage(B_QUIT_REQUESTED),'Q',0));
	mbar->AddItem(menu);
	
	// Set up the module list
	fBookListView = new BColumnListView("booklist",0);
	BStringColumn *installed_column = new BStringColumn("Installed",25,50,50,0);
	BStringColumn *book_column = new BStringColumn("Book",200,50,1000,0);
	BStringColumn *language_column = new BStringColumn("Language",75,50,1000,0);
	fBookListView->AddColumn(installed_column,0);
	fBookListView->AddColumn(book_column,1);
	fBookListView->AddColumn(language_column,2);
	fBookListView->SetSelectionMessage(new BMessage(M_SELECT_MODULE));
	fBookListView->SetInvocationMessage(new BMessage(M_MARK_MODULE));	
	for(int32 i=0; i<fConfFileList.CountItems(); i++)
	{
		ConfigFile *cfile=(ConfigFile*)fConfFileList.ItemAt(i);
		if(cfile)
		{
			BookRow *row = new BookRow(cfile);
			BStringField *install_field = NULL;
			if(IsInstalled(cfile->fFileName.String()))
				 install_field = new BStringField("*");
			else
				install_field = new BStringField(" ");
			BStringField *book_field = new BStringField(cfile->fDescription.String());
			BStringField *lang_field = new BStringField(cfile->fLanguage.String());
			
				
			row->SetField(install_field,0);
			row->SetField(book_field,1);
			row->SetField(lang_field,2);
			fBookListView->AddRow(row);
			
			//	row->SetMarked(true);
			/*BookRow *BookRow=new BookRow(cfile->fDescription.String(),cfile);
			fListView->AddItem(BookRow);*/
			
		}
	}
	
	// Add the box we use for descriptions
	fTextView=new BTextView("descriptionview");
	fTextView->MakeEditable(false);
	fTextScrollView=new BScrollView("textscrollview",fTextView,0,false,true);
	fTextScrollView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	fApplyButton=new BButton("apply button", "Apply",
			new BMessage(M_SET_PACKAGES));
	fApplyButton->SetEnabled(false);
	
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(mbar)
		.AddSplit(B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
			.Add(fBookListView, 1)
			.AddGroup(B_VERTICAL, B_USE_HALF_ITEM_SPACING, 1)
				.Add(fTextScrollView)
				.AddGroup(B_HORIZONTAL)
					.AddGlue()
					.Add(fApplyButton)
					.AddGlue()
				.End()
			.End()
		.End()
	.End();
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
			
			BookRow *item=(BookRow*)fBookListView->CurrentSelection();
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
			
			BookRow *row=(BookRow*)fBookListView->CurrentSelection();
			if(row)
			{	
				// Installed: 	uncheck=uninstall
				//				check=remove from uninstall list
				
				// Not installed: 	uncheck=remove from install list
				// 					check=add to isntall list
				BStringField *install_field = dynamic_cast<BStringField*>(row->GetField(0));
				if(IsInstalled(row->File()->fFileName.String()))
				{
					if(strchr(install_field->String(),'*')!=NULL)
					{
						// uncheck item: uninstall
						install_field->SetString("X");
						fUninstallList.Add(row->File()->fZipFileName);
					}
					else
					{
						// check item: remove from uninstall list - make no changes
						install_field->SetString("*");
						fUninstallList.Remove(row->File()->fZipFileName);
					}
				}
				else
				{
					if(strchr(install_field->String(),'i')!=NULL)
					{
						// uncheck item: make no changes to system
						install_field->SetString(" ");
						fInstallList.Remove(row->File()->fZipFileName);
					}
					else
					{
						// check item: install module
						install_field->SetString("i");
						fInstallList.Add(row->File()->fZipFileName);
					}
				}
				row->Invalidate();
			}
			if(fInstallList.CountStrings()==0 && fUninstallList.CountStrings()==0)
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
	
	installcount=win->fInstallList.CountStrings();
	removecount=win->fUninstallList.CountStrings();
	
	win->Unlock();
	
	for(i=0; i<installcount; i++)
	{
		win->Lock();
		
		zipfilename = win->fInstallList.StringAt(i);
		configpath=zipfilename;
		configpath.ToLower();
		configpath+=".conf";
		configpath.Prepend(SG_PKGINFO_PATH "configfiles/");
		ReadConfigFile(configpath.String(),cfile);
		
		displaystring="Downloading ";
		displaystring << cfile.fDescription << "\n";
				
		win->fTextView->Insert(displaystring.String());
		win->Unlock();
		
		syscmd="wget -P " SG_PKGCACHE_PATH " " SG_DOWNLOAD_PKGS;
		syscmd+=zipfilename;
		syscmd+=".zip";
		
		system(syscmd.String());
		
		win->Lock();
		displaystring="Installing ";
		displaystring << cfile.fDescription << "\n";
		win->fTextView->Insert(displaystring.String());
		win->Unlock();
		
		syscmd="unzip -o ";
		syscmd << SG_PKGCACHE_PATH << zipfilename << ".zip -d " << SG_MODULEBASE_PATH;
		
		system(syscmd.String());
		
		win->Lock();
		win->fTextView->Insert("Done\n");
		win->Unlock();
	}
	// ToDo make thiw waaaay more secure.. one error and all modules are gone.. like it happens now...
	for(i=0; i<removecount; i++)
	{
		printf("removing %s\n ",win->fUninstallList.StringAt(i).String());
		win->Lock();
		
		zipfilename=win->fUninstallList.StringAt(i);
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
		printf("%s\n",syscmd.String());
		system(syscmd.String());

		syscmd="rm -r ";
		syscmd << SG_MODULEBASE_PATH << cfile.fDataPath << "*";
		printf("%s\n",syscmd.String());
		system(syscmd.String());
		
		syscmd="rmdir ";
		syscmd << SG_MODULEBASE_PATH << cfile.fDataPath;
		printf("%s\n",syscmd.String());
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
