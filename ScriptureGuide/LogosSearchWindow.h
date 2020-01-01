#ifndef SEARCH_WINDOW_H
#define SEARCH_WINDOW_H

#include <CheckBox.h>
#include <StringView.h>
#include <ListView.h>
#include <vector>
#include <Button.h>
#include <Messenger.h>
#include <StatusBar.h>

#include "ResultListView.h"

class VersePreview;
class SGModule;

#define FIND_QUIT			'FTqu'
#define M_ACTIVATE_WINDOW	'ACwn'

enum
{
	SEARCH_WORDS = -2,
	SEARCH_PHRASE = -1,
	SEARCH_REGEX = 0
};

class SGSearchWindow : public BWindow
{
public:
					SGSearchWindow(BRect frame, const char *module,
									BMessenger* owner);
					~SGSearchWindow();
	virtual bool	QuitRequested();
	virtual void	MessageReceived(BMessage* message);

private:
	void			BuildGUI(void);
	
	vector<const char*>	books;
	SwordBackend		*myBible;
	const char			*curModule;
	
	BMenuField			*bookField;
	BMenuField			*sndBookField;
	BTextControl		*searchString;
	ResultListView		*searchResults;
	VersePreview		*verseSelected;
	BCheckBox			*caseSensitiveCheckBox;
	BStatusBar			*searchStatus;
	BButton				*findButton;
	
	SGModule			*fCurrentModule;
	
	int					fSearchMode;
	int					fSearchFlags;
	int					fSearchStart;
	int					fSearchEnd;
	BString				fSearchString;
	vector<const char*>	verseList;
	
	BMessenger			*fMessenger;
};

#endif
