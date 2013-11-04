#ifndef __LSEARCHWINDOW_H__
#define __LSEARCHWINDOW_H__

/* Scripture Guide - LogosSearchWindow.h
 *
 * Published under the GNU General Public License
 * see LICENSE for details
 *
 */

#include <CheckBox.h>
#include <StringView.h>
#include <ListView.h>
#include <vector.h>
#include "SwordBackend.h"
#include <Button.h>

class VersePreview;

class LogosSearchWindow : public BWindow
{
public:
	LogosSearchWindow(BRect frame, const char* module); 
	~LogosSearchWindow();
    	virtual bool QuitRequested();
    	virtual void MessageReceived(BMessage *message);

private:
	void _initWindow();
    	void Register(bool need_id);
    	void Unregister(void);

	vector<const char*> books;
	SwordBackend *myBible;
	const char* curModule;
    
    	BMenuField *bookField;
    	BMenuField *sndBookField;
	BTextControl *searchString;
	BListView *searchResults;
	VersePreview *verseSelected;
	BCheckBox *caseSensitiveCheckBox;
	BStatusBar *searchStatus;
	BButton* findButton;
	
	BFont romanFont;
	BFont greekFont;
	BFont *curFont;

	int mySearchStyle;
	int mySearchFlags;
	int myFrom;
	int myTo;
	char* myTxt;
	char oldtxt[100];
	vector<const char*> verseList;
	int curFontSize;
	bool isLineBreak;
    	
	int32 window_id;
};

#endif
