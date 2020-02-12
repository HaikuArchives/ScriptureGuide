#include <Application.h>
#include <Alert.h>
#include <Box.h>
#include <Catalog.h>
#include <Clipboard.h>
#include <Entry.h>
#include <Locale.h>
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

#include <LayoutBuilder.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <versekey.h>

#include "constants.h"
#include "LogosApp.h"
#include "LogosMainWindow.h"
#include "LogosSearchWindow.h"
#include "SwordBackend.h"
#include "Preferences.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SearchWindow"

class VersePreview : public BTextView
{
public:
	VersePreview(const char* name, int32 flags);
	virtual ~VersePreview(void);
	virtual void FrameResized(float width, float height);
};

VersePreview::VersePreview(const char* name, int32 flags)
 :	BTextView(name, flags)
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
	
	SetTextRect(Bounds().InsetByCopy(5, 5));
}


SGSearchWindow::SGSearchWindow(BRect frame, const char* module,
					BMessenger* owner)
 :	BWindow(frame, "", B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
			B_NOT_ZOOMABLE | B_CLOSE_ON_ESCAPE),
 	fMessenger(owner)
{
	float minw, minh, maxw, maxh;
	GetSizeLimits(&minw, &maxw, &minh, &maxh);
	minw = 500;
	minh = 480;
	SetSizeLimits(minw, maxw, minh, maxh);
	
	prefsLock.Lock();
	if (preferences.FindString("module", &curModule) != B_OK)
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
	} else
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
	// The find button
	findButton = new BButton("find_button", B_TRANSLATE("Find"), 
							new BMessage(FIND_BUTTON_OK), B_WILL_DRAW);
	SetDefaultButton(findButton);
	
	// The search query box
	searchString = new BTextControl("searchstring", B_TRANSLATE("Find: "), "", 
									new BMessage(FIND_SEARCH_STR),
									B_WILL_DRAW | B_NAVIGABLE);
	searchString->SetDivider(searchString->StringWidth("Find: ") + 5);
	
	// First, we will set up the two menu fields for the search range
	
	// The first book in the scope
	BPopUpMenu* bookChoice = new BPopUpMenu("biblebook");
	bookField = new BMenuField("book_field", B_TRANSLATE("Start in "), bookChoice);
	bookField->SetDivider(bookField->StringWidth( B_TRANSLATE("Start in "))+5);
 	BMenuItem* firstBook = new BMenuItem(books[0],
								new BMessage(FIND_SELECT_FROM)); 
	firstBook->SetMarked(true);
	bookChoice->AddItem(firstBook);
	for (unsigned int i = 1; i<books.size(); i++)
		bookChoice->AddItem(new BMenuItem(books[i], new BMessage(FIND_SELECT_FROM)));
	
	// The last book in the scope
	BPopUpMenu* sndBookChoice = new BPopUpMenu("biblebook2");
	sndBookField = new BMenuField("book_field", B_TRANSLATE("End in "),
						sndBookChoice);
	sndBookField->SetDivider(sndBookField->StringWidth(
						B_TRANSLATE("End in ")) + 5);
 	BMenuItem* lastBook = new BMenuItem(books[books.size()-1],
								new BMessage(FIND_SELECT_TO)); 
	lastBook->SetMarked(true);
	for (uint16 i = 0; i < books.size() - 1; i++)
		sndBookChoice->AddItem(new BMenuItem(books[i], new BMessage(FIND_SELECT_TO)));
	sndBookChoice->AddItem(lastBook);
	
	// The radio buttons
	BRadioButton* wordsRadio = new BRadioButton("exactwords", 
						B_TRANSLATE("Find Words"), new BMessage(FIND_RADIO1));
	
	BRadioButton* phraseRadio = new BRadioButton("phrase",
						B_TRANSLATE("Find Phrase"),	new BMessage(FIND_RADIO2));

	BRadioButton* regexRadio = new BRadioButton("regex",
						B_TRANSLATE("Regular Expression"), new BMessage(FIND_RADIO3));
	
	wordsRadio->SetValue(B_CONTROL_ON);

 	// The case sensitivity checkbox
 	caseSensitiveCheckBox = new BCheckBox("case_sensitive",  B_TRANSLATE("Match Case"),
 			new BMessage(FIND_CHECK_CASE_SENSITIVE), B_WILL_DRAW | B_NAVIGABLE);
	
	searchStatus = new BStatusBar("statusbar",  B_TRANSLATE("Search Progress:"),NULL);
	
	BStringView* resultsLabel = new BStringView("resultslabel", B_TRANSLATE("Search Results:"));
	
	// The listview for the results
	searchResults = new ResultListView("searchresults", B_MULTIPLE_SELECTION_LIST);
	searchResults->SetInvocationMessage(new BMessage(FIND_LIST_DCLICK));
	searchResults->SetSelectionMessage(new BMessage(FIND_LIST_CLICK));
	
	BScrollView* scrollView = new BScrollView("scroll_sresults", searchResults,
			0, false, true);
	
	// The textview for the selected verse
	verseSelected = new VersePreview("verse", B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	
	BScrollView* scrollVerse = new BScrollView("scroll_verse", verseSelected,
			0, false, true);
	
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_SMALL_INSETS)
		.AddGroup(B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
			.Add(searchString)
			.Add(findButton)
		.End()
		.AddGroup(B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
			.AddGroup(B_VERTICAL, B_USE_HALF_ITEM_SPACING)
				.Add(wordsRadio)
				.Add(phraseRadio)
				.Add(regexRadio)
				//.AddGlue()
			.End()
			.AddGroup(B_VERTICAL, B_USE_HALF_ITEM_SPACING)
				.Add(bookField)
				.Add(sndBookField)
				.Add(caseSensitiveCheckBox)
				//.AddGlue()
			.End()
		.End()
		.Add(searchStatus)
		.Add(resultsLabel)
		.Add(scrollView)
		.Add(scrollVerse)
	.End();
}


void SGSearchWindow::MessageReceived(BMessage* message) 
{
	switch (message->what) 
	{
		case M_ACTIVATE_WINDOW:
		{
			Activate(true);
			if (IsHidden())
				Show();
			break;
		}
		case FIND_SELECT_FROM:
		{
			// first book in search book
			// Prevent a negative search scope for the last book
			BMenu* menu = bookField->Menu();
			BMenu* menu2 = sndBookField->Menu();
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
			BMenuItem* mi = menu->ItemAt(fSearchStart);
			mi->SetMarked(true);
			break;
		}
		
		case FIND_SELECT_TO:
		{
			// last book in search book
			// Prevent a negative search scope for the last book
			BMenu* menu = sndBookField->Menu();
			BMenu* menu2 = bookField->Menu();
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
			BMenuItem* mi = menu->ItemAt(fSearchEnd);
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
			if (fSearchString.CountChars() > 0)
			{
				findButton->SetEnabled(false);
				searchString->SetEnabled(false);
				
				verseList = fCurrentModule->SearchModule(fSearchMode, fSearchFlags,
														fSearchString.String(),
														books[fSearchStart],
														books[fSearchEnd],
														searchStatus);
				searchResults->MakeEmpty();
				BLanguage language;
				BLocale::Default()->GetLanguage(&language);
				for (uint32 i = 0; i < verseList.size(); i++)
       			{
					sword::VerseKey myKey = sword::VerseKey(verseList[i]);
					myKey.setLocale(language.Code());
					BString tmpstr(fCurrentModule->GetVerse(verseList[i]));
					searchResults->AddItem(new BibleItem(myKey.getText(), tmpstr.String(), fSearchString.String()));
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
			BibleItem *item = dynamic_cast<BibleItem*>(searchResults->FullListItemAt(searchResults->FullListCurrentSelection()));
			if (item)
			{
				// TODO: Spawn with a frame obtained from preferences
				BRect windowRect(50, 50, 599, 399);
				BLanguage language;
				BLocale::Default()->GetLanguage(&language);
				sword::VerseKey myKey = sword::VerseKey();
				myKey.setLocale(language.Code());
				myKey.setText(item->GetKey());
				SGMainWindow* win = new SGMainWindow(windowRect, curModule,
										myKey, VerseFromKey(myKey), VerseFromKey(myKey));
				win->Show();
			}
			break;
		}

		case FIND_LIST_CLICK:
		{
			// an item in the list is clicked. Show the verse in the textview
			BibleItem *item = dynamic_cast<BibleItem*>(searchResults->FullListItemAt(searchResults->FullListCurrentSelection()));
			if (item)
			{
				verseSelected->Delete(0, verseSelected->TextLength());
				verseSelected->Insert(fCurrentModule->GetParagraph(item->GetKey()));
				verseSelected->Select(0, 0);
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
		case B_COPY:
		{
			BString		clipBoardString= BString();
			int32 i = 0;
			int32 selected =searchResults->FullListCurrentSelection(i);
			BibleItem *item = NULL;
			while ( (item =dynamic_cast<BibleItem*>(searchResults->FullListItemAt(selected))) != 0 )
			{
				i++;
				if (i < verseList.size())
				{
					clipBoardString << item->GetKey();
					clipBoardString << "   " << item->GetText();
				}
			}
			BMessage* clip = NULL;
			if (be_clipboard->Lock())
			{
				be_clipboard->Clear();
    			if (clip = be_clipboard->Data())
				{
					clip->AddData("text/plain", B_MIME_TYPE,
						clipBoardString.String(), clipBoardString.Length());
					be_clipboard->Commit();
				} else
					printf("ERROR couldnt get clipboard Data");
				be_clipboard->Unlock();
 			} else
 				printf("ERROR couldnt lock clipboard\n");
		}
		default:
			BWindow::MessageReceived(message);
			break;
	}
}


bool SGSearchWindow::QuitRequested() 
{
	Hide();
	return false;
}
