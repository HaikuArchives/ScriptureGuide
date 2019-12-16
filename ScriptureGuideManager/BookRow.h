#ifndef BOOK_ROW
#define BOOK_ROW

#include <ColumnListView.h>
#include "ModUtils.h"

/*class BookRow : public BStringItem
{
public:
	BookRow(const char *text, bool marked=false, uint32 level=0, bool expanded=true);
	virtual ~BookRow(void);
	virtual void DrawItem(BView *owner,BRect frame, bool complete=false);
	void SetMarked(bool ismarked);
	bool IsMarked(void) const { return fMarked; }
private:
	
};*/

class BookRow : public BRow
{
public:
	BookRow(ConfigFile *file, bool fMarked=false);
	virtual ~BookRow(void);
	ConfigFile *File(void) const { return fFile; }
	void SetFile(ConfigFile *file) { fFile=file; }
	void SetMarked(bool ismarked);
	bool IsMarked(void) const { return fMarked; }

private:
	ConfigFile *fFile;
	bool fMarked;
};

#endif
