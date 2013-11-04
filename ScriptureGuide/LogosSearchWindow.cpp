/* Scripture Guide - LogosSearchWindow.cpp
 *
 * Published under the GNU General Public License
 * see LICENSE for details
 *
 * LogosSearchWindiw displays the search gui and shows search results
 */

#include <Application.h>
#include <Alert.h>
#include <Box.h>
#include <Entry.h>
#include <MenuBar.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Message.h>
#include <Messenger.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <RadioButton.h>
#include <Roster.h>
#include <ScrollView.h>
#include <StatusBar.h>
#include <String.h>
#include <TextControl.h>
#include <View.h>
#include <Window.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>

#include "constants.h"
#include "LogosApp.h"
#include "LogosMainWindow.h"
#include "LogosSearchWindow.h"
#include "SwordBackend.h"
#include "Preferences.h"

class VersePreview : public BTextView
{
public:
	VersePreview(BRect r, const char *name, BRect textrect, int32 resize, int32 flags);
	virtual ~VersePreview(void);
	virtual void FrameResized(float width, float height);
};

VersePreview::VersePreview(BRect r, const char *name, BRect textrect, int32 resize, int32 flags)
 : BTextView(r,name,textrect,resize,flags)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetStylable(true);
	SetDoesUndo(false);
	MakeEditable(false);
	SetWordWrap(true);
}

VersePreview::~VersePreview(void)
{
}

void VersePreview::FrameResized(float width, float height)
{
	BTextView::FrameResized(width, height);
	
	SetTextRect(Bounds().InsetByCopy(5,5));
}

// constructor; gets the module and the options to pass on for subwindows
LogosSearchWindow::LogosSearchWindow(BRect frame, const char* module)
	: BWindow(frame, "Find ", B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, B_NOT_ZOOMABLE)
{
	float minw,minh,maxw,maxh;
	GetSizeLimits(&minw,&maxw,&minh,&maxh);
	minw=325;
	minh=410;
	SetSizeLimits(minw,maxw,minh,maxh);
	
	int8 myFontSize=12;
	bool myLineBreak=false;
	
	prefsLock.Lock();
	if(preferences.FindInt8("fontsize",&myFontSize)!=B_OK)
		myFontSize=12;
	if(preferences.FindBool("linebreaks",&myLineBreak)!=B_OK)
		myLineBreak=false;
	if(preferences.FindString("module",&curModule)!=B_OK)
		curModule="Webster";
	prefsLock.Unlock();
	
	myBible = new SwordBackend();
	curModule = module;
	books = myBible->getBookNames();
	myBible->setModule(curModule);
	
	romanFont.SetSize(12);
	curFont = &romanFont;

	// loads the greek font 
	// do we need this?
	greekFont.SetSize(12);

	int32 numFamilies = count_font_families(); 
	for ( int32 i = 0; i < numFamilies; i++ ) 
	{ 
		font_family family; 
		uint32 flags; 
		if ( get_font_family(i, &family, &flags) == B_OK ) 
		{ 
			if(!strcmp(family,GREEK))
				greekFont.SetFamilyAndStyle(family, NULL);
		}
	}
	
	mySearchStyle = -2;
	mySearchFlags = REG_ICASE;
	myFrom = 0;
	myTo = books.size()-1;
	myTxt = "";

	isLineBreak = myLineBreak;
	curFontSize = myFontSize;

	// start the show
	_initWindow();
	searchString->MakeFocus(true);
	Show();
}

// destructor
LogosSearchWindow::~LogosSearchWindow()
{
	delete myBible;
	Unregister();
}

// builds the gui
void LogosSearchWindow::_initWindow()
{
	BRect r(Bounds());
	BRect buttonframe;
	
	// for holding preferred width values
	float pwidth,pheight;
	
	// searchWindow is the main view
	BView *searchWindow = new BView(r, "search_window",  B_FOLLOW_ALL, B_WILL_DRAW); 
	searchWindow->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// The find button
	findButton=new BButton(BRect(0,0,0,0), "find_button", "Find", new BMessage(FIND_BUTTON_OK),
		B_FOLLOW_RIGHT, B_WILL_DRAW);
	findButton->GetPreferredSize(&pwidth,&pheight);
	findButton->ResizeToPreferred();
	findButton->MoveTo(Bounds().Width()-10-pwidth,10);
	searchWindow->AddChild(findButton); 
	SetDefaultButton(findButton);
	
	// The search query box
	r.Set(10,10,findButton->Frame().left-10,35);
	searchString = new BTextControl(r, "searchstring", "Find: ", "", 
			new BMessage(FIND_SEARCH_STR), B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
	searchString->SetDivider(searchString->StringWidth("Find: ")+5);
	
	// We do this craziness because a BTextControl has a fixed height. Given two of them with
	// the same width, one with a height of 100 and one with a height of 40, both will end up
	// looking exactly the same size.
	searchString->GetPreferredSize(&pwidth,&pheight);
	
	// we reset the width because ResizeToPreferred takes the current text into account when
	// calculating its width.
	r.bottom=r.top+pheight;
	searchString->ResizeTo(r.Width(),r.Height());
	
	searchWindow->AddChild(searchString);
	
	// The Options box
	r.top=searchString->Frame().bottom+5;
	if(findButton->Frame().bottom+5>r.top)
		r.top=findButton->Frame().bottom+5;
	r.right=searchWindow->Bounds().Width()-10;
	r.bottom=r.top+120;
	
	BBox *box1 = new BBox(r,"search_scope_box",B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	box1->SetLabel("Options");
	
	
	// First, we will set up the two menu fields for the search range
	
	// The first book in the scope
	BPopUpMenu *bookChoice = new BPopUpMenu("biblebook");
	r.Set(5,15,(box1->Bounds().Width()/2)-5,40);
	bookField = new BMenuField(r, "book_field", "Start in ", bookChoice);
	bookField->SetDivider(bookField->StringWidth("Start at ")+5);
 	BMenuItem* firstBook = new BMenuItem(books[0], new BMessage(FIND_SELECT_FROM)); 
	firstBook->SetMarked(true);
	bookChoice->AddItem(firstBook);
	for(int i=1; i<books.size(); i++)
		bookChoice->AddItem(new BMenuItem(books[i], new BMessage(FIND_SELECT_FROM)));
	box1->AddChild(bookField);

	r.OffsetBy(0,r.Height()+5);
	
	// The last book in the scope
	BPopUpMenu *sndBookChoice = new BPopUpMenu("biblebook2");
	sndBookField = new BMenuField(r, "book_field", "End at ", sndBookChoice);
	sndBookField->SetDivider(sndBookField->StringWidth("End at ")+5);
 	BMenuItem* lastBook = new BMenuItem(books[books.size()-1], new BMessage(FIND_SELECT_TO)); 
	lastBook->SetMarked(true);
	for(int i=0; i<books.size()-1; i++)
		sndBookChoice->AddItem(new BMenuItem(books[i], new BMessage(FIND_SELECT_TO)));
	sndBookChoice->AddItem(lastBook);
	box1->AddChild(sndBookField);

	searchWindow->AddChild(box1);
	
	// Now we will add the radio buttons for the search method
	
	r.left=box1->Bounds().Width()-box1->StringWidth("Regular Expression")-20;
	r.top=15;
	r.right=box1->Bounds().Width()-5;
	r.bottom=25;
	
	// The radio buttons
	BRadioButton *wordsRadio = new BRadioButton(r, "exactwords", "Find Words", 
			new BMessage(FIND_RADIO1),B_FOLLOW_RIGHT);
	box1->AddChild(wordsRadio);
	
	r.OffsetBy(0,wordsRadio->Bounds().Height());
	BRadioButton *phraseRadio = new BRadioButton(r, "phrase", "Find Phrase", 
			new BMessage(FIND_RADIO2),B_FOLLOW_RIGHT);
	box1->AddChild(phraseRadio);

	r.OffsetBy(0,phraseRadio->Bounds().Height());
	BRadioButton *regexRadio = new BRadioButton(r, "regex", "Regular Expression", 
			new BMessage(FIND_RADIO3),B_FOLLOW_RIGHT);
	box1->AddChild(regexRadio);
	
	wordsRadio->SetValue(B_CONTROL_ON);

 	// The case sensitivity checkbox
 	r.OffsetBy(0,regexRadio->Bounds().Height()+5);
 	caseSensitiveCheckBox = new BCheckBox(r, "case_sensitive", "Match Case",  
 			new BMessage(FIND_CHECK_CASE_SENSITIVE), B_FOLLOW_RIGHT | B_FOLLOW_TOP, 
 			B_WILL_DRAW | B_NAVIGABLE);
 	
	box1->AddChild(caseSensitiveCheckBox);
	
	
	// Now that we have the options taken care of, we can add the progress bar
	
	// This should become the status bar in future.
	r.Set(10,box1->Frame().bottom+5,searchWindow->Bounds().Width()-10,box1->Frame().bottom+25);
	
	searchStatus = new BStatusBar(r, "statusbar", "Search Progress:",NULL);
	searchStatus->SetResizingMode(B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	searchWindow->AddChild(searchStatus);
	
	r.Set(10,searchStatus->Frame().bottom+20,
			searchWindow->Bounds().Width()-10,
			searchStatus->Frame().bottom+40);
	
	BStringView *resultsLabel=new BStringView(r,"resultslabel","Search Results:", B_FOLLOW_LEFT | B_FOLLOW_TOP);
	searchWindow->AddChild(resultsLabel);
	
	// The listview for the results
	r.Set(10,resultsLabel->Frame().bottom+5,
			searchWindow->Bounds().Width()-B_V_SCROLL_BAR_WIDTH-10,
			resultsLabel->Frame().bottom+105);
	
	searchResults = new BListView(r, "searchresults", B_SINGLE_SELECTION_LIST,B_FOLLOW_ALL);
	searchResults->SetInvocationMessage(new BMessage(FIND_LIST_DCLICK));
	searchResults->SetSelectionMessage(new BMessage(FIND_LIST_CLICK));
	
	BScrollView *scrollView=new BScrollView("scroll_sresults", searchResults, B_FOLLOW_ALL,
			0, false, true);
	searchWindow->AddChild(scrollView); 
	
	// The unamed box to display the selected verse
	r.Set(10,scrollView->Frame().bottom+3,searchWindow->Bounds().Width()-5,
			searchWindow->Bounds().bottom-5);
	
	BBox *box5 = new BBox(r,"previewbox",B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM);

	// The textview for the selected verse
	r.Set(5,5,box5->Bounds().Width()-5,box5->Bounds().Height()-5);
	BRect textrect = r;
	textrect.OffsetTo(B_ORIGIN);
	verseSelected = new VersePreview(r, "verse", textrect,  B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM,
			 B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE); 
	box5->AddChild(verseSelected);

	searchWindow->AddChild(box5);
	AddChild(searchWindow);
	
	Register(true);
	Minimize(true);		// So Show() doesn't really make it visible
}

// Parse incoming messages
void LogosSearchWindow::MessageReceived(BMessage *message) 
{
	switch(message->what) 
	{
		// set window title
		case WINDOW_REGISTRY_ADDED:
			{
				char s[100] = "Find in ";
				if (message->FindInt32("new_window_number", &window_id) == B_OK) 
				{
					strcat(s, curModule);
					SetTitle(s);
				}
				Minimize(false);
			}
			break;
		
		// open the search help
		case FIND_BUTTON_HELP:
			{
				BString helppath(GetAppPath());
				helppath+="docs/help/index.html";
				
				BEntry entry;
				if(entry.SetTo(helppath.String())!=B_OK)
				{
					BAlert *alert=new BAlert("Scripture Guide","The help files are either missing or "
						"in an unusable condition. This shouldn't ever happen unless someone has "
						"deleted them from your machine. Sorry. You can restore them by reinstalling "
						" Scripture Guide.", "Bummer");
					alert->Go();
					be_app->PostMessage(DOCS_UNAVAILABLE);
					break;
				}
				
				entry_ref ref;
				entry.GetRef(&ref);
				be_roster->Launch(&ref);
			}
			break;
		case DOCS_UNAVAILABLE:
			{
				findButton->SetEnabled(false);
			}
			break;
			
		// first book in search book
		// do not allow negative search scope for the last book
		case FIND_SELECT_FROM:
			{
				BMenu* menu = bookField->Menu();
				BMenu* menu2 = sndBookField->Menu();
				myFrom = menu->IndexOf(menu->FindMarked());
				for(int i=0; i<myFrom; i++)
				{
					BMenuItem* mi = menu2->ItemAt(i);
					mi->SetEnabled(false);
				}
				for(int i=myFrom; i<books.size(); i++)
				{
					BMenuItem* mi = menu2->ItemAt(i);
					mi->SetEnabled(true);
				}
				BMenuItem* mi = menu->ItemAt(myFrom);
				mi->SetMarked(true);
			}
			break;
		
		// last book in search book
		// do not allow negative search scope for the first book
		case FIND_SELECT_TO:
			{
				BMenu* menu = sndBookField->Menu();
				BMenu* menu2 = bookField->Menu();
				myTo = menu->IndexOf(menu->FindMarked());
				for(int i=0; i<=myTo; i++)
				{
					BMenuItem* mi = menu2->ItemAt(i);
					mi->SetEnabled(true);
				}
				for(int i=myTo+1; i<books.size(); i++)
				{
					BMenuItem* mi = menu2->ItemAt(i);
					mi->SetEnabled(false);
				}
				BMenuItem* mi = menu->ItemAt(myTo);
				mi->SetMarked(true);
			}
			break;
		
		// a entered search query
		case FIND_SEARCH_STR:
			{
				myTxt = (char*) searchString->Text();
			}
			break;

		// do the search
		case FIND_BUTTON_OK:
			{
				myTxt = (char*) searchString->Text();
				if(strlen(myTxt)>0)
				{
					findButton->SetEnabled(false);
					searchString->SetEnabled(false);
					
					verseList = myBible->searchModule(mySearchStyle, mySearchFlags, myTxt, books[myFrom], books[myTo], searchStatus);
					searchResults->RemoveItems(0,searchResults->CountItems());
 					for(int i=0; i<verseList.size(); i++)
        			{
						BString tmpstr(verseList[i]);
						tmpstr += "   ";
						tmpstr += myBible->getVerse(verseList[i]);
						searchResults->AddItem(new BStringItem(tmpstr.String()));
        			}   
        			
					findButton->SetEnabled(true);
					searchString->SetEnabled(true);
				}
 			}
 			break;
 
		case FIND_CHECK_CASE_SENSITIVE:
 			{
 				if (caseSensitiveCheckBox->Value() == B_CONTROL_ON) mySearchFlags = 0;  // && REG_ICASE instead?
 				else mySearchFlags = REG_ICASE; // || REG_ICASE instead?   // ignore case
			}
			break;

		// an item in the list is double clicked
		// open a new window with the selected verse
		case FIND_LIST_DCLICK:
			{
				int i = searchResults->CurrentSelection();
				if(i<verseList.size())
				{
					BRect windowRect(50,50,599,399); 
					new LogosMainWindow(windowRect, curModule, verseList[i], true);
				}
			}
			break;

		// an item in the list is clicked 
		// show the verse in the textview
		case FIND_LIST_CLICK:
			{
				int i = searchResults->CurrentSelection();
				verseSelected->Delete(0,verseSelected->TextLength());
				if(i<verseList.size())
				{
					verseSelected->Delete(0,verseSelected->TextLength());
					verseSelected->Insert(verseList[i]);
					verseSelected->Insert("   ");
					verseSelected->Insert(myBible->getVerse(verseList[i]));
					verseSelected->Select(0,0);
				}
			}
			break;

		// the search method is selected 
		case FIND_RADIO1:
			{
				mySearchStyle = -2;
			}
			break;
		case FIND_RADIO2:
			{
				mySearchStyle = -1;
			}
			break;
		case FIND_RADIO3:
			{
				mySearchStyle = 0;
			}
			break;

		// otherwise
		default:
			BWindow::MessageReceived(message);
			break;
	}
}

void LogosSearchWindow::Register(bool need_id) 
{
	BMessage message(WINDOW_REGISTRY_ADD);
	
	message.AddBool("need_id", need_id);
	be_app_messenger.SendMessage(&message, this);
}

void LogosSearchWindow::Unregister(void) 
{
	be_app_messenger.SendMessage(new BMessage(WINDOW_REGISTRY_SUB));
}

bool LogosSearchWindow::QuitRequested() 
{
	return true;
}
