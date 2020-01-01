/*
 * Copyright 2019 Paradoxianer <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <AppDefs.h>
#include <Bitmap.h>
#include <Catalog.h>
#include <ListItem.h>
#include <Locale.h>
#include <View.h>

#include <stdio.h>

#include "ResultListView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SearchWindow"

BibleItem::BibleItem(const char* key,  const char* text)
	: BListItem()
{
	fKey=key;
	fText=text;
}

BibleItem::~BibleItem(void)
{
}

void BibleItem::DrawItem(BView *owner,
            BRect frame,
            bool complete)
{
	DrawBackground(owner, frame);
	ResultListView* rlView = dynamic_cast<ResultListView *>(owner);
	if (IsSelected())
		owner->SetHighColor(ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR));
	else
		owner->SetHighColor(ui_color(B_LIST_ITEM_TEXT_COLOR));
	font_height fh;
	owner->GetFontHeight(&fh);
	
	BString truncatedString(fText);
	owner->TruncateString(&truncatedString, B_TRUNCATE_MIDDLE,
						  frame.Width() - TEXT_OFFSET - 4.0);
						  
	float height = frame.Height();
	float textHeight = fh.ascent + fh.descent;
	BPoint keyPoint;
	BPoint versePoint;
	keyPoint.x = frame.left + TEXT_OFFSET;
	keyPoint.y = frame.top
				  + ceilf(height / 2.0 - textHeight / 2.0
				  		  + fh.ascent);	
	versePoint=keyPoint;
	keyPoint.x = keyPoint.x + owner->StringWidth(fKey)+5;
	owner->DrawString(fKey, keyPoint);
	owner->DrawString(truncatedString.String(),versePoint );
}

void BibleItem::DrawBackground(BView *owner, BRect frame)
{
	// stroke a blue frame around the item if it's focused
	/*if (flags & FLAGS_FOCUSED) {
		owner->SetLowColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
		owner->StrokeRect(frame, B_SOLID_LOW);
		frame.InsetBy(1.0, 1.0);
	}*/
	// figure out bg-color
	rgb_color color = ui_color(B_LIST_BACKGROUND_COLOR);

	if (IsSelected())
		color = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);

	owner->SetLowColor(color);
	owner->FillRect(frame, B_SOLID_LOW);
}

ResultListView::ResultListView(const char* name, list_view_type type
					, uint32 flags)
	:BOutlineListView( name, type, flags),
	fDragCommand(B_SIMPLE_DATA)
{
	Init();
}


ResultListView::~ResultListView()
{
}

void ResultListView::Init()
{
}


bool
ResultListView::InitiateDrag( BPoint point, int32 index, bool)
{
	bool success = false;
	BListItem* item = ItemAt(CurrentSelection(0));
	if (!item) {
		// workarround a timing problem
		Select(index);
		item = ItemAt(index);
	}
	if (item) {
		// create drag message
		BMessage msg( fDragCommand );
		MakeDragMessage( &msg );
		// figure out drag rect
		float width = Bounds().Width();
		BRect dragRect(0.0, 0.0, width, -1.0);
		// figure out, how many items fit into our bitmap
		int32 sIndex=0;
		bool fade = false;
		BibleItem* tmpItem =NULL;
		for (int32 i = 0; (sIndex = CurrentSelection(i)) >= 0; i++)
		{
			tmpItem = dynamic_cast<BibleItem*>(ItemAt(CurrentSelection(sIndex)));
			dragRect.bottom += ceilf( item->Height() ) + 1.0;
			if ( dragRect.Height() > MAX_DRAG_HEIGHT ) {
				fade = true;
				dragRect.bottom = MAX_DRAG_HEIGHT;
				i++;
				break;
			}
		}
		BBitmap* dragBitmap = new BBitmap( dragRect, B_RGB32, true );
		if ( dragBitmap && dragBitmap->IsValid() ) {
			if ( BView *v = new BView( dragBitmap->Bounds(), "helper", B_FOLLOW_NONE, B_WILL_DRAW ) ) {
				dragBitmap->AddChild( v );
				dragBitmap->Lock();
				BRect itemBounds( dragRect) ;
				itemBounds.bottom = 0.0;
				// let all selected items, that fit into our drag_bitmap, draw
				BibleItem* item = NULL;
				int32 sIndex=0;
				for (int32 i = 0; (sIndex = CurrentSelection(i)) >= 0; i++)
				{
					item = dynamic_cast<BibleItem*>(ItemAt(CurrentSelection(sIndex)));
					itemBounds.bottom = itemBounds.top + ceilf( item->Height() );
					if ( itemBounds.bottom > dragRect.bottom )
						itemBounds.bottom = dragRect.bottom;
					item->DrawItem(v, itemBounds);
					itemBounds.top = itemBounds.bottom + 1.0;
				}
				// make a black frame arround the edge
				v->SetHighColor( 0, 0, 0, 255 );
				v->StrokeRect( v->Bounds() );
				v->Sync();

				uint8 *bits = (uint8 *)dragBitmap->Bits();
				int32 height = (int32)dragBitmap->Bounds().Height() + 1;
				int32 width = (int32)dragBitmap->Bounds().Width() + 1;
				int32 bpr = dragBitmap->BytesPerRow();

				if (fade) {
					for ( int32 y = 0; y < height - ALPHA / 2; y++, bits += bpr ) {
						uint8 *line = bits + 3;
						for (uint8 *end = line + 4 * width; line < end; line += 4)
							*line = ALPHA;
					}
					for ( int32 y = height - ALPHA / 2; y < height; y++, bits += bpr ) {
						uint8 *line = bits + 3;
						for (uint8 *end = line + 4 * width; line < end; line += 4)
							*line = (height - y) << 1;
					}
				} else {
					for ( int32 y = 0; y < height; y++, bits += bpr ) {
						uint8 *line = bits + 3;
						for (uint8 *end = line + 4 * width; line < end; line += 4)
							*line = ALPHA;
					}
				}
				dragBitmap->Unlock();
			}
		} else {
			delete dragBitmap;
			dragBitmap = NULL;
		}
		if (dragBitmap)
			DragMessage( &msg, dragBitmap, B_OP_ALPHA, BPoint( 0.0, 0.0 ) );
		else
			DragMessage( &msg, dragRect.OffsetToCopy( point ), this );

		//_SetDragMessage(&msg);
		success = true;
	}
	return success;
}

void ResultListView::MakeDragMessage(BMessage* message)
{
	if (message)
	{
		BLanguage language;
		BLocale::Default()->GetLanguage(&language);
		int32 index;
		for (int32 i = 0; (index = CurrentSelection(i)) >= 0; i++)
		{
			BibleItem* tmpItem = dynamic_cast< BibleItem*>(FullListItemAt(index));
			if (tmpItem != NULL)
			{
				message->AddString("key", tmpItem->GetKey());
				message->AddString("text",tmpItem->GetText());
				message->AddString("locale", language.Code());
			}
		}
	}
}
