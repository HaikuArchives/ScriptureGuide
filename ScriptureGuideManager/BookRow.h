#ifndef BOOK_ROW
#define BOOK_ROW

#include <ColumnListView.h>
#include "ModUtils.h"


class BookRow : public BRow
{
public:
	BookRow(ConfigFile *file, bool fMarked=false);
	virtual ~BookRow(void);
	ConfigFile *File(void) const { return fFile; }
	void SetFile(ConfigFile *file) { fFile=file; }

private:
	ConfigFile *fFile;
};

#endif
