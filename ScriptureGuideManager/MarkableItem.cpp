#include <Font.h>
#include <View.h>
#include "MarkableItem.h"

MarkableItem::MarkableItem(const char *text, bool marked, uint32 level=0, bool expanded=true)
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
}

void MarkableItem::SetMarked(bool value)
{
	fMarked=value;
}

BookItem::BookItem(const char *text, ConfigFile *file, bool marked, 
	uint32 level, bool expanded)
	: MarkableItem(text,marked,level,expanded)
{
	SetFile(file);
}

BookItem::~BookItem(void)
{
}

void BookItem::DrawItem(BView *owner,BRect frame, bool complete)
{
	if(IsMarked())
	{
		owner->SetHighColor(0,0,200);
		owner->SetFont(be_bold_font);
	}
	else
	{
		owner->SetHighColor(0,0,0);
		owner->SetFont(be_plain_font);
	}
	BStringItem::DrawItem(owner,frame,complete);
}
