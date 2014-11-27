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
#include <iostream>

#include "constants.h"
#include "LogosApp.h"
#include "LogosMainWindow.h"
#include "LogosSearchWindow.h"
#include "SwordBackend.h"
#include "Preferences.h"

/*enum
{
	FIND_BUTTON_OK = 'FBok',
	FIND_BUTTON_HELP,
	FIND_CHECK_CASE_SENSITIVE,
	FIND_SELECT_FROM,
	FIND_SELECT_TO,
	FIND_SEARCH_STR,
	FIND_RADIO1,
	FIND_RADIO2,
	FIND_RADIO3,
	FIND_LIST_CLICK,
	FIND_LIST_DCLICK,
	FIND_TMP
};*/

class VersePreview : public BTextView
{
public:
	VersePreview(BRect r, const char *name, BRect textrect, int32 resize, int32 flags);
	virtual ~VersePreview(void);
	virtual void FrameResized(float width, float height);
};

VersePreview::VersePreview(BRect r, const char *name, BRect textrect, int32 resize,
							int32 flags)
 :	BTextView(r,name,textrect,resize,flags)
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

SGSearchWindow::SGSearchWindow(BRect frame, const char *module, BMessenger *owner)
 :	BWindow(frame,"", B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, B_NOT_ZOOMABLE),
 	fMessenger(owner)
{
	float minw,minh,maxw,maxh;
	GetSizeLimits(&minw,&maxw,&minh,&maxh);
	minw = 325;
	minh = 410;
	SetSizeLimits(minw,maxw,minh,maxh);
	
	int8 myFontSize;
	bool myLineBreak;
	
	prefsLock.Lock();
	if(preferences.FindInt8("fontsize",&myFontSize)!=B_OK)
		myFontSize = 12;
	if(preferences.FindBool("linebreaks",&myLineBreak)!=B_OK)
		myLineBreak = false;
	if(preferences.FindString("module",&curModule)!=B_OK)
		curModule = "WEB";
	prefsLock.Unlock();
	
	myBible = new SwordBackend();
	curModule = module;
	books = GetBookNames();
	fCurrentModule = myBible->FindModule(curModule);
	
	if (module)
	{
		BString title("Find in ");
		title << fCurrentModule->FullName();
		SetTitle(title.String());
	}
	else
		SetTitle("Find");
	
	fSearchMode = SEARCH_WORDS;
	fSearchFlags = REG_ICASE;
	fSearchStart = 0;
	fSearchEnd = books.size()-1;

	// start the show
	BuildGUI();
	searchString->MakeFocus(true);
}

SGSearchWindow::~SGSearchWindow(void)
{
	delete myBible;
	
	fMessenger->SendMessage(FIND_QUIT);
	delete fMessenger;
}

void SGSearchWindow::BuildGUI(void)
{
	BRect r(Bounds());
	
	// for holding preferred width values
	float pwidth,pheight;
	
	// searchWindow is the main view
	BView *searchWindow = new BView(r, "search_window",  B_FOLLOW_ALL, B_WILL_DRAW); 
	searchWindow->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// The find button
	findButton = new BButton(BRect(0,0,0,0), "find_button", "Find", 
							new BMessage(FIND_BUTTON_OK), B_FOLLOW_RIGHT, B_WILL_DRAW);
	findButton->GetPreferredSize(&pwidth,&pheight);
	findButton->ResizeToPreferred();
	findButton->MoveTo(Bounds().Width() - 10 - pwidth,10);
	searchWindow->AddChild(findButton); 
	SetDefaultButton(findButton);
	
	// The search query box
	r.Set(10,10,findButton->Frame().left - 10,35);
	searchString = new BTextControl(r, "searchstring", "Find: ", "", 
									new BMessage(FIND_SEARCH_STR),
									B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
	searchString->SetDivider(searchString->StringWidth("Find: ") + 5);
	
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
	for(unsigned int i=1; i<books.size(); i++)
		bookChoice->AddItem(new BMenuItem(books[i], new BMessage(FIND_SELECT_FROM)));
	box1->AddChild(bookField);

	r.OffsetBy(0,r.Height()+5);
	
	// The last book in the scope
	BPopUpMenu *sndBookChoice = new BPopUpMenu("biblebook2");
	sndBookField = new BMenuField(r, "book_field", "End in ", sndBookChoice);
	sndBookField->SetDivider(sndBookField->StringWidth("End in ") + 5);
 	BMenuItem *lastBook = new BMenuItem(books[books.size()-1], new BMessage(FIND_SELECT_TO)); 
	lastBook->SetMarked(true);
	for (uint16 i = 0; i < books.size() - 1; i++)
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
}

void SGSearchWindow::MessageReceived(BMessage *message) 
{
	switch(message->what) 
	{
		case M_ACTIVATE_WINDOW:
		{
			Activate(true);
			break;
		}
		case FIND_SELECT_FROM:
		{
			// first book in search book
			// Prevent a negative search scope for the last book
			BMenu *menu = bookField->Menu();
			BMenu *menu2 = sndBookField->Menu();
			fSearchStart = menu->IndexOf(menu->FindMarked());
			
			for (uint8 i = 0; i < fSearchStart; i++)
			{
				BMenuItem* mi = menu2->ItemAt(i);
				mi->SetEnabled(false);
			}
			
			for (uint8 i = fSearchStart; i < books.size(); i++)
			{
				BMenuItem* mi = menu2->ItemAt(i);
				mi->SetEnabled(true);
			}
			BMenuItem *mi = menu->ItemAt(fSearchStart);
			mi->SetMarked(true);
			break;
		}
		
		case FIND_SELECT_TO:
		{
			// last book in search book
			// Prevent a negative search scope for the last book
			BMenu *menu = sndBookField->Menu();
			BMenu *menu2 = bookField->Menu();
			fSearchEnd = menu->IndexOf(menu->FindMarked());
			for (uint8 i = 0; i <= fSearchEnd; i++)
			{
				BMenuItem* mi = menu2->ItemAt(i);
				mi->SetEnabled(true);
			}
			for (uint8 i = fSearchEnd + 1; i < books.size(); i++)
			{
				BMenuItem* mi = menu2->ItemAt(i);
				mi->SetEnabled(false);
			}
			BMenuItem *mi = menu->ItemAt(fSearchEnd);
			mi->SetMarked(true);
			break;
		}
		
		case FIND_SEARCH_STR:
		{
			fSearchString = searchString->Text();
			break;
		}

		case FIND_BUTTON_OK:
		{
			fSearchString = searchString->Text();
			if(fSearchString.CountChars() > 0)
			{
				findButton->SetEnabled(false);
				searchString->SetEnabled(false);
				
				verseList = fCurrentModule->SearchModule(fSearchMode, fSearchFlags,
														fSearchString.String(),
														books[fSearchStart],
														books[fSearchEnd],
														searchStatus);
				searchResults->RemoveItems(0,searchResults->CountItems());
				for(uint32 i = 0; i < verseList.size(); i++)
       			{
					BString tmpstr(verseList[i]);
					tmpstr << "   " << fCurrentModule->GetVerse(verseList[i]);
					searchResults->AddItem(new BStringItem(tmpstr.String()));
       			}   
       			
				findButton->SetEnabled(true);
				searchString->SetEnabled(true);
			}
 			break;
		}
 
		case FIND_CHECK_CASE_SENSITIVE:
		{
			fSearchFlags = (caseSensitiveCheckBox->Value() == B_CONTROL_ON) ?
	 					fSearchFlags = 0 :
	 					REG_ICASE;
			break;
		}

		case FIND_LIST_DCLICK:
		{
			// an item in the list is double clicked. Open a new window with the selected verse
			uint32 i = searchResults->CurrentSelection();
			if (i < verseList.size())
			{
				// TODO: Spawn with a frame obtained from preferences
				BRect windowRect(50,50,599,399); 
				SGMainWindow *win = new SGMainWindow(windowRect, curModule, verseList[i]);
				win->Show();
			}
			break;
		}

		case FIND_LIST_CLICK:
		{
			// an item in the list is clicked. Show the verse in the textview
			uint32 i = searchResults->CurrentSelection();
			verseSelected->Delete(0,verseSelected->TextLength());
			if (i < verseList.size())
			{
				verseSelected->Delete(0,verseSelected->TextLength());
				verseSelected->Insert(verseList[i]);
				verseSelected->Insert("   ");
				verseSelected->Insert(fCurrentModule->GetVerse(verseList[i]));
				verseSelected->Select(0,0);
			}
			break;
		}

		case FIND_RADIO1:
		{
			fSearchMode = SEARCH_WORDS;
			break;
		}
		case FIND_RADIO2:
		{
			fSearchMode = SEARCH_PHRASE;
			break;
		}
		case FIND_RADIO3:
		{
			fSearchMode = SEARCH_REGEX;
			break;
		}
		default:
			BWindow::MessageReceived(message);
			break;
	}
}

bool SGSearchWindow::QuitRequested() 
{
	return true;
}
