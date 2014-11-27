#include "LogosMainWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <Entry.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Message.h>
#include <Messenger.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <Roster.h>
#include <String.h>
#include <File.h>
#include <Directory.h>
#include <Box.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "HAboutWindow.h"
#include "constants.h"
#include "LogosApp.h"
#include "LogosSearchWindow.h"
#include "FontPanel.h"
#include "Preferences.h"

/**enum
{
	MENU_FILE_NEW = 'MFnw',
	MENU_FILE_QUIT,
	MENU_EDIT_NOTE,
	MENU_EDIT_FIND,
	MENU_OPTIONS_PARALLEL,
	MENU_OPTIONS_PARALLEL_CONTEXT,
	MENU_OPTIONS_LINE,
	MENU_OPTIONS_FONT,
	MENU_OPTIONS_VERSENUMBERS,
	MENU_HELP_LOGOS,
	MENU_HELP_HOWTO,
	MENU_HELP_ABOUT,
	
	SELECT_BIBLE,
	SELECT_COMMENTARY,
	SELECT_LEXICON,
	SELECT_GENERAL,
	SELECT_BOOK,
	SELECT_CHAPTER,
	SELECT_VERSE,
	SELECT_FONT,
	NEXT_BOOK,
	PREV_BOOK,
	NEXT_CHAPTER,
	PREV_CHAPTER
};*/

SGMainWindow::SGMainWindow(BRect frame, const char *module, const char *key)
 :	BWindow(frame, "Scripture Guide", B_DOCUMENT_WINDOW, 0),
 	fFontPanel(NULL),
 	fModManager(NULL),
 	fCurrentModule(NULL),
 	fCurrentChapter(1),
 	fCurrentFont(NULL),
 	fFindMessenger(NULL)
{
	float minw,minh,maxw,maxh;
	GetSizeLimits(&minw,&maxw,&minh,&maxh);
	minw = 580;
	minh = 420;
	SetSizeLimits(minw,maxw,minh,maxh);
	
	if (key)
		fCurrentChapter = ChapterFromKey(key);
		
	fModManager = new SwordBackend();
	BuildGUI();
	
	BMenuItem *item = fBookMenu->FindItem(BookFromKey(key));
	if (item)
		item->SetMarked(true);
	
	SetModuleFromString(module);
	fCurrentModule->SetKey(key);

	// More voodoo hackerdom to work around a bug. :)
	AddCommonFilter(new EndKeyFilter);
	
	if (fModManager->CountModules()==0)
	{
		BAlert *alert = new BAlert("Scripture Guide","Scripture Guide could not find any books -- no Bibles,"
			" commentaries, or anything else. You need to have at least one book from the"
			" SWORD Project in order to run Scripture Guide. Would you like to go to their website"
			" now, look at Scripture Guide's installation notes, or just quit?\n","Website",
			"Install Notes","Quit");
		int32 value = alert->Go();
		
		if (value == 0)
			system("/boot/beos/apps/NetPositive http://www.crosswire.org/sword/index.jsp &");
		else
		if (value == 1)
		{
			BString string("/boot/beos/apps/StyledEdit ");
			string << GetAppPath() << "INSTALL";
			system(string.String());
		}
		
		be_app->PostMessage(B_QUIT_REQUESTED);
		PostMessage(B_QUIT_REQUESTED);
		return;
	}
	
	fCurrentModule = fModManager->FindModule(fCurrentModule->Name());
	if (!fCurrentModule)
	{
		// It's possible for this call to fail, so we'll handle it as best we can. Seeing how we
		// managed to get this far, there has to be *some* kind of module available. As a result,
		// we'll just select the first module available.
		if (fModManager->CountBibles() > 0)
		{
			SGModule *mod = fModManager->BibleAt(0);
			if (mod)
			{
				fModManager->SetModule(mod);
				fCurrentModule = mod;
			}
		}
		else
		if (fModManager->CountCommentaries() > 0)
		{
			SGModule *mod = fModManager->CommentaryAt(0);
			if (mod)
			{
				fModManager->SetModule(mod);
				fCurrentModule = mod;
			}
		}
		else
		if (fModManager->CountLexicons() > 0)
		{
			SGModule *mod = fModManager->LexiconAt(0);
			if (mod)
			{
				fModManager->SetModule(mod);
				fCurrentModule = mod;
			}
		}
		else
		if (fModManager->CountGeneralTexts() > 0)
		{
			SGModule *mod = fModManager->GeneralTextAt(0);
			if (mod)
			{
				fModManager->SetModule(mod);
				fCurrentModule = mod;
			}
		}
		else
		{
			// we should never be here, but just in case we do....
			
			BAlert *alert = new BAlert("Scripture Guide","Scripture Guide could not find any books -- no Bibles,"
				" commentaries, or anything else. You need to have at least one book from the"
				" SWORD Project in order to run Scripture Guide. Would you like to go to their website"
				" now, look at Scripture Guide's installation notes, or just quit?\n","Website",
				"Install Notes","Quit");
			int32 value = alert->Go();
			
			if (value == 0)
				system("/boot/beos/apps/NetPositive http://www.crosswire.org/sword/index.jsp &");
			else
			if (value == 1)
			{
				BString string("/boot/beos/apps/StyledEdit ");
				string << GetAppPath() << "INSTALL";
				system(string.String());
			}
			
			be_app->PostMessage(B_QUIT_REQUESTED);
			PostMessage(B_QUIT_REQUESTED);
			return;
		}
	}
	
	// Load the preferences for the individual module
	LoadPrefsForModule();
	
	fCurrentFont = &fDisplayFont;
	
	fRomanFont.SetSize(fFontSize);
	fGreekFont.SetSize(fFontSize);
	fHebrewFont.SetSize(fFontSize);
	
	// Attempt to set the supported Greek and Hebrew fonts. In this case, the BeBook is wrong --
	// SetFamilyandFace is declared in Font.h as returning a status_t.
	
	// Ideally, there should be a better way of choosing non-Latin fonts, but we'll live with
	// this for now. :)
	font_family fam;
	sprintf((char *)fam,"%s",GREEK);
	if (fGreekFont.SetFamilyAndFace(fam, B_REGULAR_FACE)!=B_OK)
		fGreekFont=*be_plain_font;
	
	sprintf((char *)fam,"%s",HEBREW);
	if (fHebrewFont.SetFamilyAndFace(fam, B_REGULAR_FACE)!=B_OK)
		fHebrewFont=*be_plain_font;
}


SGMainWindow::~SGMainWindow() 
{
	delete fModManager;
	delete fFontPanel;
}


void SGMainWindow::BuildGUI(void) 
{
	BRect r;
	BMenu *menu;
	
	r = Bounds();
	fMenuBar = new BMenuBar(r, "menubar");
	AddChild(fMenuBar);
	
	menu = new BMenu("Program");
	menu->AddItem(new BMenuItem("About Scripture Guide…", new BMessage(MENU_HELP_ABOUT)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Duplicate This Window…", new BMessage(MENU_FILE_NEW), 'D'));
	menu->AddItem(new BMenuItem("Close This Window", new BMessage(B_QUIT_REQUESTED), 'W'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Quit", new BMessage(MENU_FILE_QUIT), 'Q'));
	fMenuBar->AddItem(menu);


	menu = new BMenu("Navigation");
	menu->AddItem(new BMenuItem("Find Verse…", new BMessage(MENU_EDIT_FIND), 'F'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Next Book", new BMessage(NEXT_BOOK), B_RIGHT_ARROW, B_OPTION_KEY));
	menu->AddItem(new BMenuItem("Previous Book", new BMessage(PREV_BOOK), B_LEFT_ARROW, B_OPTION_KEY));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Next Chapter", new BMessage(NEXT_CHAPTER), B_RIGHT_ARROW));
	menu->AddItem(new BMenuItem("Previous Chapter", new BMessage(PREV_CHAPTER), B_LEFT_ARROW));
	menu->AddSeparatorItem();
	
	BMenuItem *copyitem = new BMenuItem("Copy Verses", new BMessage(B_COPY), 'C');
	menu->AddItem(copyitem);
	
	BMenuItem *selectallitem = new BMenuItem("Select All", new BMessage(B_SELECT_ALL), 'A');
	menu->AddItem(selectallitem);
	fMenuBar->AddItem(menu);
	
	// disabled this for now -- it doesn't make much sense to have *both* a menu item and a button,
	// especially when we don't really have a menu to put it in where it makes much organizational sense
//	menu->AddItem(new BMenuItem("Notes...", new BMessage(MENU_EDIT_NOTE), 'T'));
//	menu->AddSeparatorItem();
	
	menu = new BMenu("Options");
	
	menu->AddItem(new BMenuItem("Choose Font...", new BMessage(SELECT_FONT)));
	menu->AddSeparatorItem();
	
	fShowVerseNumItem = new BMenuItem("Show Verse Numbers", new BMessage(MENU_OPTIONS_VERSENUMBERS));
	if (fShowVerseNumbers)
		fShowVerseNumItem->SetMarked(true);
	menu->AddItem(fShowVerseNumItem);
	
	fMenuBar->AddItem(menu);
	
	// We need to populate the module menu because the size of the field depends on the average
	// module name width. We average because the KJV + Strongs + Morphology has a horribly long
	// name while the rest are reasonable.
	BMenu *modulemenu = new BMenu("text");
	int32 count = fModManager->CountBibles();
	
	float avgwidth = 0.0;
	
	// Add Bibles
	fBibleMenu = new BMenu("Bibles");
	for(int32 i = 0; i < count; i++)
	{
		BString modname = fModManager->BibleAt(i)->FullName();
		avgwidth += be_plain_font->StringWidth(modname.String());
		
		BMessage *biblemsg = new BMessage(SELECT_BIBLE);
		biblemsg->AddInt32("index",i);
		fBibleMenu->AddItem(new BMenuItem(modname.String(),biblemsg));
	}
	modulemenu->AddItem(fBibleMenu);

	count = fModManager->CountCommentaries();
	
	// Add Commentaries
	fCommentaryMenu = new BMenu("Commentaries");
	for(int32 i = 0; i < count; i++)
	{
		BString modname = fModManager->CommentaryAt(i)->FullName();
		avgwidth += be_plain_font->StringWidth(modname.String());
		
		BMessage *commmsg = new BMessage(SELECT_COMMENTARY);
		commmsg->AddInt32("index",i);
		fCommentaryMenu->AddItem(new BMenuItem(modname.String(),commmsg));
	}
	modulemenu->AddItem(fCommentaryMenu);
	
	avgwidth /= fModManager->CountBibles() + fModManager->CountCommentaries();
	
	
	// Add the toolbar view
	BRect toolframe = Bounds();
	toolframe.top = fMenuBar->Bounds().bottom + 1.0;
	toolframe.bottom = toolframe.top + 28.0;
	
	BBox *toolbar = new BBox(toolframe, "toolbar_view",  B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
	//toolbar->SetViewColor(fMenuBar->ViewColor());
	AddChild(toolbar);
	
	BRect fieldframe(toolbar->Bounds());
	fieldframe.left = 5;
	fieldframe.right = 5 + avgwidth + 30 + be_plain_font->StringWidth("Text:") + 5;
	fModuleField = new BMenuField(fieldframe, "modulefield", "Text:", modulemenu);
	fModuleField->SetDivider(be_plain_font->StringWidth("Text:") + 5);
	fModuleField->ResizeToPreferred();
	fModuleField->ResizeTo(fieldframe.Width(), fModuleField->Bounds().Height());
	fModuleField->MoveTo(5, (toolbar->Bounds().Height() - fModuleField->Bounds().Height())/2);
	fieldframe = fModuleField->Frame();
	toolbar->AddChild(fModuleField);
	
	// Add the book menu
	fBookMenu = new BMenu("book");
	fieldframe.left = fieldframe.right + 5;
	fieldframe.right = fieldframe.left + be_plain_font->StringWidth("Revelation of John") +
						be_plain_font->StringWidth("Book:") + 30;
	BMenuField *bookfield = new BMenuField(fieldframe, "bookfield", "Book:", fBookMenu);
	bookfield->SetDivider(be_plain_font->StringWidth("Book:") + 5);
	toolbar->AddChild(bookfield);
	fBookMenu->SetLabelFromMarked(true);
	
	vector<const char *> booknames = GetBookNames();
	for(uint32 i = 0; i < booknames.size(); i++)
		fBookMenu->AddItem(new BMenuItem(booknames[i], new BMessage(SELECT_BOOK)));
	fBookMenu->ItemAt(0)->SetMarked(true);
	
	// The toolbar controls go from general to specific (for the most part) when going from left to right.
	// The textboxes and buttons are right justified, so the easiest way to dynamically calculate
	// layout is from right to left based on the value of the right edge of the toolbar view. Thus, we
	// will build all the controls from right to left, but in order to keep keyboard navigation in the
	// proper order, we will wait until all controls have been built and add them in order of left to
	// right.
	
	// Add the notes button
	fieldframe = toolbar->Bounds();
	BButton *fNoteButton = new BButton(fieldframe, "note_button", "Notes…", new BMessage(MENU_EDIT_NOTE),
										B_FOLLOW_RIGHT | B_FOLLOW_TOP , B_WILL_DRAW | B_NAVIGABLE);
	fNoteButton->ResizeToPreferred();
	fNoteButton->MoveTo( toolbar->Bounds().right - 10 - fNoteButton->Bounds().Width(),1);
	
	if (fieldframe.Height() < fNoteButton->Bounds().Height() + 5)
	{
		toolbar->ResizeTo(toolbar->Bounds().Width(), fNoteButton->Frame().bottom + 5);
		fModuleField->MoveTo(5, (toolbar->Bounds().Height() - fModuleField->Bounds().Height())/2);
		bookfield->MoveTo(fModuleField->Frame().right + 10, (toolbar->Bounds().Height() - fModuleField->Bounds().Height())/2);
		fNoteButton->MoveBy(0,2);
	}

	BString alphaChars("qwertyuiop[]\\asdfghjkl;'zxcvbnm,./QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>?`~!@#$%^&*()-_=+");
	
	// even though it seems like we just got the ideal size, ResizeToPreferred makes for a much wider
	// size than what we want at this point. At the same time, the control has a fixed height, so if
	// we ask for anything taller than this, it looks just plain dumb, so what we have done is actually
	// obtained the height we want and dictated the width we want. :)
	fieldframe = toolbar->Bounds();
	fChapterBox = new BTextControl(fieldframe, "chapter_choice", "Chapter", NULL,
									new BMessage(SELECT_CHAPTER), B_FOLLOW_RIGHT | B_FOLLOW_TOP,
									B_WILL_DRAW  | B_NAVIGABLE);
	
	BTextView *chapterView = fChapterBox->TextView();
	for(int32 i=0; i < alphaChars.CountChars(); i++)
	{
		char c = alphaChars.ByteAt(i);
		chapterView->DisallowChar(c);
	}
	
	toolbar->AddChild(fChapterBox);
	toolbar->AddChild(fNoteButton);
	
	// We need to do the resizing code for the BTextControl because of an R5 bug -- the ResizeToPreferred
	// code does nothing unless attached to a window
	fChapterBox->ResizeToPreferred();
	fieldframe = fChapterBox->Frame();
	fChapterBox->SetDivider(be_plain_font->StringWidth("Chapter") + 5);
	
	fieldframe.right = fieldframe.left + 80;
	fChapterBox->ResizeTo(fChapterBox->Divider() + be_plain_font->StringWidth("0000"),fieldframe.Height());
	fChapterBox->MoveTo(fNoteButton->Frame().left - 10 - fChapterBox->Bounds().Width(),
						(toolbar->Bounds().Height() - fChapterBox->Bounds().Height())/2);

	// Add the text view
	BRect textframe = Bounds();
	textframe.top = toolbar->Frame().bottom + 1;
	textframe.right -= B_V_SCROLL_BAR_WIDTH;
	BRect textrect = textframe;
	textrect.OffsetTo(B_ORIGIN);
	r.InsetBy(3.0,3.0);
	
	fVerseView = new BTextView(textframe, "text_view", textrect,
				B_FOLLOW_ALL_SIDES, B_WILL_DRAW|B_PULSE_NEEDED);
	
	fScrollView = new BScrollView("scroll_view", fVerseView, B_FOLLOW_ALL_SIDES, 0, false, true, B_NO_BORDER);
	AddChild(fScrollView);
	
	fVerseView->SetDoesUndo(false);
	fVerseView->MakeFocus(true);
	fVerseView->MakeEditable(false);
	fVerseView->SetStylable(true);
	fVerseView->SetWordWrap(true);
        
	copyitem->SetTarget(fVerseView);
	selectallitem->SetTarget(fVerseView);
}

void SGMainWindow::InsertVerseNumber(int verse)
{
	BString string;
	string << " " << verse << " ";
	
	BFont boldfont(be_bold_font);
	boldfont.SetSize(fFontSize);
	fVerseView->SetFontAndColor(&boldfont,B_FONT_ALL,&BLUE);
	fVerseView->Insert(string.String());
	fVerseView->SetFontAndColor(fCurrentFont,B_FONT_ALL,&BLACK);
}

void SGMainWindow::InsertChapter(void)
{
	BString oldtxt("1"), newtxt("2");
	BString currentbook(fBookMenu->FindMarked()->Label());
	
	uint16 versecount = VersesInChapter(currentbook.String(),fCurrentChapter);
	if (fCurrentModule->Type() == TEXT_BIBLE) 
	{
		BString text(fCurrentModule->GetVerse(currentbook.String(),fCurrentChapter,1));
		
		if (text.CountChars()<1)
		{
			// this condition will only happen if the module is only one particular
			// testament.
			fVerseView->Insert("This module does not have this section.");
			return;
		}
		
		for(uint16 currentverse = 1; currentverse <= versecount; currentverse++)
		{
			// Get the verse for processing
			text.SetTo(fCurrentModule->GetVerse(currentbook.String(),fCurrentChapter,currentverse));
			
			if (text.CountChars() < 1)
				continue;
			
			
			// Remove <P> tags and 0xc2 0xb6 sequences to carriage returns. The crazy hex sequence
			// is actually the UTF-8 encoding for the paragraph symbol. If we convert them to \n's,
			// output looks funky
			text.RemoveAll("\xc2\xb6 ");
			text.RemoveAll("<P> ");
			
			if (fIsLineBreak)
				text += "\n";
			
			if (fShowVerseNumbers)
				InsertVerseNumber(currentverse);
			fVerseView->Insert(text.String());
		}
	}
	else
	{
		for(uint16 currentverse = 1; currentverse <= versecount; currentverse++)
		{
			// for commentaries, avoid doubled output
			oldtxt.SetTo(newtxt);
			newtxt.SetTo(fCurrentModule->GetVerse(currentbook.String(),fCurrentChapter,currentverse));
			if (oldtxt != newtxt && newtxt.CountChars() > 0)
			{
				if (fShowVerseNumbers)
					InsertVerseNumber(currentverse);
				
				// Remove <P> tags and 0xc2 0xb6 sequences to carriage returns. The crazy hex sequence
				// is actually the UTF-8 encoding for the paragraph symbol. If we convert them to \n's,
				// output looks funky
				newtxt.RemoveAll("\xc2\xb6 ");
				newtxt.RemoveAll("<P> ");
				
				fVerseView->Insert(newtxt.String());
				
				// add an extra line break after each verse to make better readability
				fVerseView->Insert("\n");

				if (fIsLineBreak)
					fVerseView->Insert("\n");
			}
		}
	}
}

void SGMainWindow::LoadPrefsForModule(void)
{
	modPrefsLock.Lock();
	BMessage msg;
	
	BFont font;
	status_t status;
	font_family fam;
	font_style sty;
	BString modname;
	
	status = LoadModulePreferences(fCurrentModule->Name(),&msg);
	
	if (status != B_OK)
	{
		// We couldn't load the module-specific preferences, so generate them,
		// save them to disk, and return
		
		// We default to whatever font the system has for be_plain_font and detect the
		// need for linebreaks for the particular module
		
		msg.MakeEmpty();
		
		fFontSize = (int16)font.Size();
		
		// The fDisplayFont object initializes to be_plain_font, which is what we
		// want, but we'll need to save the family and style to preferences
		font.GetFamilyAndStyle(&fam,&sty);
		
		// Detect need for linebreak insertion
		fIsLineBreak = NeedsLineBreaks();
		
		// Normally show verse numbers
		fShowVerseNumbers = true;
		
		msg.AddInt16("fontsize",fFontSize);
		msg.AddString("family",fam);
		msg.AddString("style",sty);
		msg.AddBool("linebreaks",fIsLineBreak);
		msg.AddBool("versenumbers",fShowVerseNumbers);
		
		SaveModulePreferences(fCurrentModule->Name(),&msg);
	}
	else
	{
		// It's possible that something was left out, so detect individually
		// and compensate even though the preferences are likely to be an all-or-nothing thing
		
		bool saveprefs = false;
		
		if (msg.FindInt16("fontsize",&fFontSize)!=B_OK)
		{
			fFontSize = (int16)be_plain_font->Size();
			msg.AddInt16("fontsize",fFontSize);
			saveprefs = true;
		}
		
		if (msg.FindBool("linebreaks",&fIsLineBreak)!=B_OK)
		{
			fIsLineBreak = NeedsLineBreaks();
			msg.AddBool("linebreaks",fIsLineBreak);
			saveprefs=true;
		}
		
		if (msg.FindBool("versenumbers",&fShowVerseNumbers)!=B_OK)
		{
			fShowVerseNumbers = true;
			msg.AddBool("versenumbers",fShowVerseNumbers);
			saveprefs = true;
		}
		
		fDisplayFont = font;
		
		BString sfam, ssty;
		if ( msg.FindString("family",&sfam) != B_OK || msg.FindString("style",&ssty) != B_OK )
		{
			font.GetFamilyAndStyle(&fam,&sty);
			msg.AddString("family",fam);
			msg.AddString("style",sty);
			saveprefs = true;
		}
		else
		{
			fDisplayFont.SetFamilyAndStyle(sfam.String(),ssty.String());
			fDisplayFont.SetSize(fFontSize);
		}
		
		if (saveprefs)
			SaveModulePreferences(fCurrentModule->Name(),&msg);
	}
	modPrefsLock.Unlock();
}

void SGMainWindow::SavePrefsForModule(void)
{
	if (!fCurrentModule)
		return;
	
	BMessage msg;

	font_family fam;
	font_style sty;
	fDisplayFont.GetFamilyAndStyle(&fam,&sty);
	
	msg.AddBool("linebreaks",fIsLineBreak);
	msg.AddInt16("fontsize",fDisplayFont.Size());
	msg.AddString("family",fam);
	msg.AddString("style",sty);
	msg.AddBool("versenumbers",fShowVerseNumbers);
	SaveModulePreferences(fCurrentModule->Name(),&msg);
	
	// We also need to write to the application's main preferences so that the last
	// module used is the one to come up on the app's next execution
	
	// prevent any stupidity by the module in use
//	fCurrentModule->SetVerse(curBook.String(),curChapter,currentverse);
	
	prefsLock.Lock();
	preferences.RemoveData("windowframe");
	preferences.AddRect("windowframe",Frame());
	preferences.RemoveData("module");
	preferences.AddString("module",fCurrentModule->Name());
	preferences.RemoveData("key");
	preferences.AddString("key",fCurrentModule->GetKey());
	prefsLock.Unlock();
}

bool SGMainWindow::NeedsLineBreaks(void)
{
	// This function detects the need to manually insert newlines after each verse by getting
	// the entire current chapter. While not perfect, it is unlikely that an entire chapter should
	// go by without a new paragraph.
	
	// Get the verse for processing
	BString text;
	
	BString currentbook = fBookMenu->FindMarked()->Label();
	uint16 chaptercount = VersesInChapter(currentbook.String(),fCurrentChapter);
	for(uint16 currentverse = 1; currentverse <= chaptercount; currentverse++)
		text += fCurrentModule->GetVerse(currentbook.String(),fCurrentChapter,currentverse);
	
	text.RemoveAll("\xc2\xb6 ");
	text.RemoveAll("<P> ");
	
	if (text.FindFirst("\n") == B_ERROR)
		return true;
	
	return false;
}

// the window is resized, ajust the fVerseView and the toolbar
void SGMainWindow::FrameResized(float width, float height) 
{
	BRect textrect = fVerseView->TextRect();
	textrect.right = textrect.left + (width - B_V_SCROLL_BAR_WIDTH - 3.0);
	fVerseView->SetTextRect(textrect);
}


void SGMainWindow::MessageReceived(BMessage *msg) 
{
	switch(msg->what) 
	{
		case SELECT_BIBLE:
		{
			int32 index = 0;
			if (msg->FindInt32("index",&index) != B_OK)
				break;
			SetModule(TEXT_BIBLE,index);
			break;
		}
		case SELECT_COMMENTARY:
		{
			int32 index = 0;
			if (msg->FindInt32("index",&index) != B_OK)
				break;
			SetModule(TEXT_COMMENTARY,index);
			break;
		}
		
		case NEXT_BOOK:
		{
			// We'll figure this out by using the books menu -- figuring it all out using
			// the STL vector class is a pain, not to mention slower.
			
			int32 index = fBookMenu->IndexOf(fBookMenu->FindMarked());
			BMenuItem *currentItem = fBookMenu->ItemAt(index);
			
			if (index < fBookMenu->CountItems() - 1)
			{
				currentItem->SetMarked(false);
				
				BMenuItem *newItem = fBookMenu->ItemAt(++index);
				newItem->SetMarked(true);
				
				fCurrentChapter = 1;

				fVerseView->Delete(0,fVerseView->TextLength());
				InsertChapter();
				
				fChapterBox->SetText("1");
				fVerseView->Select(0,0);
				fVerseView->MakeFocus();
			}
			break;
		}
		case PREV_BOOK:
		{
			// We'll figure this out by using the books menu. Figuring it all out using
			// the STL vector class is a pain.
			
			int32 index = fBookMenu->IndexOf(fBookMenu->FindMarked());
			BMenuItem *currentItem = fBookMenu->ItemAt(index);
			
			if (index > 0)
			{
				currentItem->SetMarked(false);
				
				BMenuItem *newItem=fBookMenu->ItemAt(--index);
				newItem->SetMarked(true);
				
				fCurrentChapter = 1;

				fVerseView->Delete(0,fVerseView->TextLength());
				InsertChapter();
				
				fChapterBox->SetText("1");
				fVerseView->Select(0,0);
				fVerseView->MakeFocus();
			}
			break;
		}
		case SELECT_BOOK:
		{
			fCurrentChapter = 1;
			
			fVerseView->Delete(0,fVerseView->TextLength());
			InsertChapter();
			fChapterBox->SetText("1");
			fVerseView->Select(0,0);
			fVerseView->MakeFocus();
			break;
		}
		case NEXT_CHAPTER:
		{
			SetChapter(++fCurrentChapter);
			break;
		}
		case PREV_CHAPTER:
		{
			SetChapter(--fCurrentChapter);
			break;
		}
		case SELECT_CHAPTER:
		{
			int num = atoi(fChapterBox->Text());
			SetChapter(num);
			break;
		}
		
		case MENU_FILE_NEW:
		{
			SGMainWindow *win = new SGMainWindow(Frame().OffsetByCopy(20,20),
												fCurrentModule->Name(), fCurrentModule->GetKey());
			win->Show();
			break;
		}
		case MENU_FILE_QUIT:
		{
			be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case MENU_OPTIONS_VERSENUMBERS:
		{
			fShowVerseNumbers = (fShowVerseNumbers) ? false : true;
			fShowVerseNumItem->SetMarked(fShowVerseNumbers);
			fVerseView->Delete(0,fVerseView->TextLength());
			fVerseView->SetFontAndColor(fCurrentFont,B_FONT_ALL,&BLACK);
			InsertChapter();
			break;
		}
		case MENU_HELP_ABOUT:
		{
			HAboutWindow *hwin = new HAboutWindow("Scripture Guide","March 24, 2007",
				"Credits to \n\tBrian\n\tDarkWyrm\n\tjan__64\n\tKevin\n",
				"http://www.scripture-guide.org/",
				"webmaster@scripture-guide.org");
			hwin->SetLook(B_TITLED_WINDOW_LOOK);
			hwin->Show();
			break;
		}
		case FIND_QUIT:
		{
			// This message is received whenever the child find window quits
			delete fFindMessenger;
			fFindMessenger = NULL;
			break;
		}
		case MENU_EDIT_FIND:
		{
			if (fFindMessenger)
			{
				fFindMessenger->SendMessage(M_ACTIVATE_WINDOW);
				break;
			}
			
			BRect r(Frame().OffsetByCopy(5,23));
			r.right = r.left + 325;
			r.bottom = r.top + 410;
			SGSearchWindow *win = new SGSearchWindow(r, fCurrentModule->Name(),new BMessenger(this));
			fFindMessenger = new BMessenger(win);
			win->Show();
			break;
		}
		case MENU_EDIT_NOTE:
		{
			BString notespath(GetAppPath());
			notespath += "notes/Notes.txt";
			
			BEntry entry;
			if (entry.SetTo(notespath.String())!=B_OK)
			{
				// Apparently the notes were blown away while the app was running, so 
				// re-create them
				create_directory(notespath.String(),0777);
				
				notespath += "/Notes.txt";
				BFile file(notespath.String(), B_READ_WRITE | B_CREATE_FILE);
				const char notes[]="Scripture Guide Study Notes\n-------------------------------\n";
				file.Write(notes,strlen(notes));
				file.Unset();
				
				break;
			}
			
			entry_ref ref;
			entry.GetRef(&ref);
			be_roster->Launch(&ref);
			break;
		}

		// Called when the user selects 'Choose Font...' from menu
		case SELECT_FONT:
		{
			if (fFontPanel)
				delete fFontPanel;
			
			BMessenger msgr(this);				
			fFontPanel = new FontPanel(&msgr,NULL,fFontSize);
			fFontPanel->SelectFont(fDisplayFont);
			fFontPanel->Show();
			break;
		}
		
		// called when the user selects a font from the panel
		case M_FONT_SELECTED:
		{
			BString family;
			BString style;
			float size;
			
			if (msg->FindString("family",&family) != B_OK ||
				msg->FindString("style",&style) != B_OK ||
				msg->FindFloat("size",&size) != B_OK)
				break;
			
			fFontSize = (int)size;
			fCurrentFont->SetSize(size);
		//	fCurrentFont->SetFamilyAndStyle((font_family)(family.String()),(font_family)(style.String()));
			
			fDisplayFont.SetSize(size);
		//	fDisplayFont.SetFamilyAndStyle((font_family)(family.String()),(font_family)(style.String()));
			
			fVerseView->Delete(0,fVerseView->TextLength());
			fVerseView->SetFontAndColor(fCurrentFont,B_FONT_ALL,&BLACK);
			InsertChapter();
			fVerseView->Select(0,0);
			fVerseView->MakeFocus();
			break;
		}
		
		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}
}


void SGMainWindow::SetModuleFromString(const char *name)
{
	if (!name)
		return;
	
	SGModule *current = NULL;
	for (int32 i = 0; i < fModManager->CountBibles(); i++)
	{
		current = fModManager->BibleAt(i);
		if ( strcmp(name,current->Name()) == 0 || strcmp(name,current->FullName()) == 0 )
		{
			SetModule(TEXT_BIBLE, i);
			return;
		}
	}
	
	for (int32 i = 0; i < fModManager->CountCommentaries(); i++)
	{
		current = fModManager->CommentaryAt(i);
		if ( strcmp(name,current->Name()) == 0 || strcmp(name,current->FullName()) == 0 )
		{
			SetModule(TEXT_COMMENTARY, i);
			return;
		}
	}
	
	// If we got here, something is wrong
	SetModule(TEXT_BIBLE,0);
}


void SGMainWindow::SetModule(const TextType &module, const int32 &index)
{
	SavePrefsForModule();
	
	SGModule *sgmod;
	if (module == TEXT_BIBLE)
		sgmod = fModManager->BibleAt(index);
	else
	if (module == TEXT_COMMENTARY)
		sgmod = fModManager->CommentaryAt(index);
	else
	{
		// Currently-unsupported module. Should *never* happen.
		return;
	}
	
	if (!sgmod)
		return;
	
	fModManager->SetModule(sgmod);
	fCurrentModule = sgmod;
	BMenuItem *menu = (BMenuItem*) fModuleField->Menu()->Superitem();
	menu->SetLabel(sgmod->FullName());
	
	// make sure only the books available can be selected
	BMenuItem *currentbook;
	
	bool onvalue = sgmod->HasOT();
	for (int32 i = 0; i < 39; i++)
	{
		currentbook = fBookMenu->ItemAt(i);
		if (!currentbook)
			break;
		currentbook->SetEnabled(onvalue);
	}
	
	onvalue = sgmod->HasNT();
	for(int32 i = 39; i < 66; i++)
	{
		currentbook = fBookMenu->ItemAt(i);
		if (!currentbook)
			break;
		currentbook->SetEnabled(onvalue);
	}
	
	BString title("Scripture Guide: ");
	title << fCurrentModule->FullName();
	SetTitle(title.String());
	
	fVerseView->Delete(0,fVerseView->TextLength());
	
	LoadPrefsForModule();
	
	if (sgmod->IsGreek())
		fCurrentFont = &fGreekFont;
	else if (sgmod->IsHebrew())
		fCurrentFont = &fHebrewFont;
	else
		fCurrentFont = &fRomanFont;
	
	fCurrentFont->SetSize(fFontSize);
	fVerseView->SetFontAndColor(fCurrentFont,B_FONT_ALL,&BLACK);
	
	BString chapterstring;
	chapterstring << fCurrentChapter;
	fChapterBox->SetText(chapterstring.String());
	
	InsertChapter();
	fVerseView->Select(0,0);
	fVerseView->MakeFocus();
}

void SGMainWindow::SetChapter(const int16 &chapter)
{
	BString currentbook;
	
	int16 maxchapters = ChaptersInBook(fBookMenu->FindMarked()->Label());
	if (chapter > maxchapters)
	{
		int16 index = fBookMenu->IndexOf(fBookMenu->FindMarked());
		if (index < fBookMenu->CountItems()-1)
		{
			BMenuItem *item = fBookMenu->ItemAt(index);
			item->SetMarked(false);
			index++;
			item = fBookMenu->ItemAt(index);
			item->SetMarked(true);
			
			currentbook = fBookMenu->ItemAt(index)->Label();
			fCurrentChapter = 1;
		}
		else
		{
			fCurrentChapter = maxchapters;
			return;
		}
	}
	else
	if (chapter < 1)
	{
		// we are at the first chapter of the book.
		// go to the first verse of the last chapter of the previous book
		// unless there isn't another one
		
		int16 index = fBookMenu->IndexOf(fBookMenu->FindMarked());
		if (index > 0)
		{
			BMenuItem *item = fBookMenu->ItemAt(index);
			item->SetMarked(false);
			index--;
			item=fBookMenu->ItemAt(index);
			item->SetMarked(true);
			
			currentbook = fBookMenu->ItemAt(index)->Label();
			fCurrentChapter = ChaptersInBook(fBookMenu->FindMarked()->Label());
		}
		else
		{
			fCurrentChapter = 1;
			return;
		}
	}
	else
		fCurrentChapter = chapter;
	
	fVerseView->Delete(0,fVerseView->TextLength());
	InsertChapter();
	
	BString text;
	text << fCurrentChapter;
	fChapterBox->SetText(text.String());
	fVerseView->Select(0,0);
	fVerseView->MakeFocus();
}

bool SGMainWindow::QuitRequested() 
{
	if (fFindMessenger)
	{
		fFindMessenger->SendMessage(B_QUIT_REQUESTED);
		delete fFindMessenger;
		fFindMessenger = NULL;
	}
	
	SavePrefsForModule();
	be_app_messenger.SendMessage(new BMessage(M_WINDOW_CLOSED));
	
	return true;
}

EndKeyFilter::EndKeyFilter(void)
 : BMessageFilter(B_PROGRAMMED_DELIVERY, B_ANY_SOURCE,B_KEY_DOWN)
{
}

EndKeyFilter::~EndKeyFilter(void)
{
}

filter_result EndKeyFilter::Filter(BMessage *msg, BHandler **target)
{
	int32 c;
	msg->FindInt32("raw_char",&c);
	if (c == B_END || c == B_HOME)
	{
		BTextView *text = dynamic_cast<BTextView*>(*target);
		if (text && text->IsFocus())
		{
			BScrollBar *sb = text->ScrollBar(B_VERTICAL);
			
			// We have to include this check, because each
			// BTextControl has a BTextView inside it, but
			// those ones don't have scrollbars
			if (!sb)
				return B_DISPATCH_MESSAGE;
			float min, max;
			sb->GetRange(&min,&max);
			
			if (c == B_HOME)
				sb->SetValue(min);
			else
				sb->SetValue(max);
			return B_SKIP_MESSAGE;
		}
	}
	return B_DISPATCH_MESSAGE;
}

