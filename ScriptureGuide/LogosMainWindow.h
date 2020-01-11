#ifndef SGMAINWINDOW_H
#define SGMAINWINDOW_H


#include <Font.h>
#include <Menu.h>
#include <MenuItem.h>
#include <MenuField.h>
#include <MessageFilter.h>
#include <Messenger.h>
#include <ScrollView.h>
#include <TextControl.h>
#include <TextView.h>
#include <String.h>
#include <Window.h>

#include <vector>

#include "SwordBackend.h"

class FontPanel;
class SGModule;

#define M_WINDOW_CLOSED 'wcls'
using namespace std;


class EndKeyFilter : public BMessageFilter
{
public:
	EndKeyFilter(void);
	~EndKeyFilter(void);
	virtual filter_result Filter(BMessage* msg, BHandler **target);
};

class SGMainWindow : public BWindow 
{
public:
	SGMainWindow(BRect frame, const char* module, const char* key,
					uint16 selectVers = 1, uint16 selectVersEnd = 0);
	~SGMainWindow();
	virtual bool QuitRequested();
	virtual void MessageReceived(BMessage* message);
	virtual void FrameResized(float width, float height);

private:
	void BuildGUI(void);
	void InsertVerseNumber(int verse);
	void InsertChapter(void);
	void LoadPrefsForModule(void);
	void SavePrefsForModule(void);
	bool NeedsLineBreaks(void);
	
	void SetModule(const TextType &module, const int32 &index);
	void SetModuleFromString(const char* name);
	void SetBook(const char* name);	
	void SetChapter(const int16 &chapter);
	void SetVerse(const int16 &verse);

	BMenuBar		*fMenuBar;
	
	BMenuField		*fModuleField;
	
	BMenuItem		*fShowParallelItem,
					*fShowVerseNumItem,
					*fShowParallelContextItem;

	BMenu			*fBookMenu,
					*fBibleMenu,
					*fCommentaryMenu,
					*fLexiconMenu,
					*fGeneralMenu;
	
	BTextControl	*fChapterBox;
	BTextControl	*fVerseBox;

	BButton			*fNoteButton;
	
	BTextView		*fVerseView;
	BScrollView		*fScrollView;
	
	FontPanel		*fFontPanel;
	
	
	SwordBackend	*fModManager;
	SGModule		*fCurrentModule;
	uint16			fCurrentChapter;
	uint16			fCurrentVerse;
	uint16			fCurrentVerseEnd;	
	
	int16			fFontSize;
	BFont			*fCurrentFont;
	BFont			fDisplayFont,
					fGreekFont,
					fHebrewFont,
					fRomanFont;
	
	bool			fIsLineBreak,
					fShowVerseNumbers;
	
	BMessenger		*fFindMessenger;
};

#endif
