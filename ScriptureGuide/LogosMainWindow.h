#ifndef __LMAINWINDOW_H__
#define __LMAINWINDOW_H__

/* Scripture Guide - LogosApp.h
 *
 * Published under the GNU General Public License
 * see LICENSE for details
 *
 */

#include <Font.h>
#include <Menu.h>
#include <MenuItem.h>
#include <MenuField.h>
#include <ScrollView.h>
#include <TextControl.h>
#include <TextView.h>
#include <String.h>
#include <MessageFilter.h>
#include <vector.h>
#include "SwordBackend.h"
#include "FontPanel.h"


class EndKeyFilter : public BMessageFilter
{
public:
	EndKeyFilter(void);
	~EndKeyFilter(void);
	virtual filter_result Filter(BMessage *msg, BHandler **target);
};

class LogosMainWindow : public BWindow 
{
	public:
		LogosMainWindow(BRect frame, const char *module, const char* key, bool mark);
		~LogosMainWindow();
		virtual bool QuitRequested();
		virtual void MessageReceived(BMessage *message);
		virtual void FrameResized(float width, float height);

	private:
		void _InitWindow(void);
		void Register(bool need_id);
		void Unregister(void);
		void insertVerseNumber(int verse);
		void parseText(void);
		void LoadPrefs(void);
		void SavePrefs(void);
		bool NeedsLineBreaks(void);

		BMenuBar *menubar;
		BTextView *textview;
		BView *toolbar;
		BScrollView *scrollview;
		BMenuField *moduleField;
		BMenuField *bookField;
		BTextControl *chapterChoice;
		BTextControl *verseChoice;
		BButton *noteButton;
		BMenuItem *manualItem;
		BMenuItem *showVerseNumItem;

		BFont greekFont;
		BFont romanFont;
		BFont hebrewFont;
		BFont *curFont;
		
		BFont displayFont;
		int16 curFontSize;
		FontPanel *fontPanel;

		int32 window_id;

		SwordBackend *myBible;
		vector<const char*> modules;
		vector<const char*> books;    
		BString curModule;
		BString curBook;
		int16 curChapter;
		int16 curVerse;
		bool setMark;

		bool isLineBreak;
		bool showVerseNumbers;
		int versePos[177];
};

#endif
