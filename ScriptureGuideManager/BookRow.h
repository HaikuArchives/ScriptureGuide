#ifndef MARKABLE_ITEM
#define MARKABLE_ITEM

#include <ListItem.h>
#include "ModUtils.h"

class MarkableItem : public BStringItem
{
public:
	MarkableItem(const char *text, bool marked=false, uint32 level=0, bool expanded=true);
	virtual ~MarkableItem(void);
	virtual void DrawItem(BView *owner,BRect frame, bool complete=false);
	void SetMarked(bool ismarked);
	bool IsMarked(void) const { return fMarked; }
private:
	bool fMarked;
};

class BookItem : public MarkableItem
{
public:
	BookItem(const char *text, ConfigFile *file, bool marked=false, uint32 level=0, bool expanded=true);
	virtual ~BookItem(void);
	ConfigFile *File(void) const { return fFile; }
	void SetFile(ConfigFile *file) { fFile=file; }
	virtual void DrawItem(BView *owner,BRect frame, bool complete=false);
private:
	ConfigFile *fFile;
};

#endif
