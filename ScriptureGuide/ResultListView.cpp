/*
 * Copyright 2019 Paradoxianer <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <AppDefs.h>
#include <Bitmap.h>
#include <Catalog.h>
#include <ControlLook.h>
#include <InterfaceDefs.h>
#include <ListItem.h>
#include <Locale.h>
#include <View.h>

#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "ResultListView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SearchWindow"

BibleItem::BibleItem(const char* key,  const char* text, const char* highlight)
	: BListItem(),
	fKey(NULL),
	fText(NULL),
	fHighlight(NULL),
	fBaselineOffset(0)
{
	SetKey(key);
	SetText(text);
	SetHighlight(highlight);
}

BibleItem::~BibleItem(void)
{
}

void BibleItem::DrawItem(BView *owner,
            BRect frame,
            bool complete)
{
	if (fText == NULL && fKey == NULL)
		return;
	BFont viewFont;
	BFont boldFont;
	rgb_color lowColor = owner->LowColor();
	owner -> GetFont(&viewFont);
	boldFont=viewFont;
	boldFont.SetFace(B_BOLD_FACE);
	if (IsSelected() || complete) {
		rgb_color color;
		if (IsSelected())
			color = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
		else
			color = owner->ViewColor();

		owner->SetLowColor(color);
		owner->FillRect(frame, B_SOLID_LOW);
	} else
		owner->SetLowColor(owner->ViewColor());
	owner->MovePenTo(frame.left + be_control_look->DefaultLabelSpacing(),
		frame.top + fBaselineOffset);
	owner->SetFont(&boldFont);
	owner->DrawString(fKey);
	owner->MovePenBy(be_control_look->DefaultLabelSpacing(),0);
	owner->SetFont(&viewFont);	
	if (fHighlight)
	{
		BString textString(fText);
		BString highlightString(fHighlight);
		int32 offset = 0;
		int32 lastOffset = 0;
		offset=textString.IFindFirst(fHighlight, lastOffset);
		while (offset>=0)
		{
			BString tmpText;
			textString.CopyInto(tmpText, lastOffset,offset - lastOffset);			
			owner->DrawString(tmpText.String());
			lastOffset = offset + highlightString.CountChars();
			owner->SetFont(&boldFont);			
			owner->DrawString(fHighlight);
			owner->SetFont(&viewFont);
			offset=textString.IFindFirst(fHighlight, lastOffset);
		}
		BString tmpText;
		textString.CopyInto(tmpText, lastOffset,textString.CountChars() - lastOffset);
		owner->DrawString(tmpText);
	} else
	{
		owner->DrawString(fText);
	}
	owner->SetLowColor(lowColor);
}


void BibleItem::SetKey(const char* key)
{
	free(fKey);
	fKey = NULL;

	if (key)
		fKey = strdup(key);
}


void BibleItem::SetText(const char* text)
{
	free(fText);
	fText = NULL;

	if (text)
		fText = strdup(text);
}

void BibleItem::SetHighlight(const char* highlight)
{
	free(fHighlight);
	fHighlight = NULL;

	if (highlight)
		fHighlight = strdup(highlight);
}


void BibleItem::Update(BView* owner, const BFont* font)
{
	float widt = 0.0;
	if (fKey != NULL)
	{
		widt += font->StringWidth(fKey)
				+ be_control_look->DefaultLabelSpacing();
	}
	if (fText != NULL)
	{
		widt += be_control_look->DefaultLabelSpacing()
				+ font->StringWidth(fText)
				+ be_control_look->DefaultLabelSpacing();
	}

	font_height fheight;
	font->GetHeight(&fheight);

	fBaselineOffset = 2 + ceilf(fheight.ascent + fheight.leading / 2);

	SetHeight(ceilf(fheight.ascent) + ceilf(fheight.descent)
		+ ceilf(fheight.leading) + 4);
}



ResultListView::ResultListView(const char* name, list_view_type type
					, uint32 flags)
	:BOutlineListView( name, type, flags),
	fDragCommand(SG_BIBLE)
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
		int32 sIndex=CurrentSelection(0);
		bool fade = false;
		BibleItem* tmpItem = NULL;
		int32 i = 0;
		while(sIndex >= 0)
		{
			tmpItem = dynamic_cast<BibleItem*>(ItemAt(CurrentSelection(i)));
			if (tmpItem)
			{
				dragRect.bottom += ceilf( item->Height() ) + 1.0;
				if ( dragRect.Height() > MAX_DRAG_HEIGHT ) {
					fade = true;
					dragRect.bottom = MAX_DRAG_HEIGHT;
					i++;
					break;
				}
			}
			sIndex=CurrentSelection(i);
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
				int32 i = 0;				
				int32 sIndex=CurrentSelection(0);
				while (sIndex>=0)
				{
					item = dynamic_cast<BibleItem*>(ItemAt(CurrentSelection(i)));
					if (item)
					{
						itemBounds.bottom = itemBounds.top + ceilf( item->Height() );
						if ( itemBounds.bottom > dragRect.bottom )
							itemBounds.bottom = dragRect.bottom;
						item->DrawItem(v, itemBounds);
						itemBounds.top = itemBounds.bottom + 1.0;
					}
					i++;
					sIndex=CurrentSelection(i);
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
		BString allVerses;
		BString allKeys;
		message->AddPointer("be:originator", this);
		

		for (int32 i = 0; (index = CurrentSelection(i)) >= 0; i++)
		{
			BibleItem* tmpItem = dynamic_cast< BibleItem*>(FullListItemAt(index));
			if (tmpItem != NULL)
			{
				allVerses << tmpItem->GetKey() << " ";
				allVerses << tmpItem->GetText() << "\n";
				if (allKeys.Length()>0)
					allKeys << ", ";
				allKeys << tmpItem->GetKey();
				message->AddString("key", tmpItem->GetKey());
				message->AddString("text",tmpItem->GetText());
				message->AddString("locale", language.Code());
			}
		}
		message->AddData("text/plain", B_MIME_TYPE, allVerses.String(), allVerses.Length());
		message->AddString("be:clip_name", allKeys);
	}
}
