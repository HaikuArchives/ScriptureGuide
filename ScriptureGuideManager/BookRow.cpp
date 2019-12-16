#include <Font.h>
#include <View.h>
#include "BookRow.h"

/*MarkableItem::MarkableItem(const char *text, bool marked, uint32 level, bool expanded)
 :BStringItem(text,level,expanded)
{
	fMarked=marked;
}

MarkableItem::~MarkableItem(void)
{
}

void MarkableItem::DrawItem(BView *owner,BRect frame, bool complete)
{
	if(fMarked)
		owner->SetFont(be_bold_font);
	else
		owner->SetFont(be_plain_font);
	BStringItem::DrawItem(owner,frame,complete);
}*/

BookRow::BookRow(ConfigFile *file, bool marked)
	: BRow()
{
	
	SetFile(file);
}

BookRow::~BookRow(void)
{
}


void BookRow::SetMarked(bool value)
{
	fMarked=value;
}
