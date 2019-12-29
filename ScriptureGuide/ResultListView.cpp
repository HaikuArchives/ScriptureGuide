/*
 * Copyright 2019 Paradoxianer <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <AppDefs.h>
#include <Catalog.h>
#include <ColumnTypes.h>
#include <Locale.h>

#include "ResultListView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SearchWindow"

ResultRow::ResultRow(char* key, char* text)
	: BRow()
{
	BStringField *key_field = new BStringField(key);
	BStringField *text_field = new BStringField(text);
	SetField(key_field,0);
	SetField(text_field,1);
}

ResultRow::~ResultRow(void)
{
}


ResultListView::ResultListView(BRect rect, const char* name)
	: BColumnListView(rect, name, 0, 0, B_NO_BORDER, true),
		fDragCommand(B_SIMPLE_DATA)
{
	Init();
}



ResultListView::ResultListView(const char*name)
	:BColumnListView(name, 0, B_NO_BORDER, true),
		fDragCommand(B_SIMPLE_DATA)
{
	Init();
}


ResultListView::~ResultListView()
{
}

void ResultListView::Init()
{
	BStringColumn *verse_key = new BStringColumn(B_TRANSLATE("Bibleverse"),25,50,50,0);
	BStringColumn *preview = new BStringColumn(B_TRANSLATE("Preview"),200,50,1000,0);
	AddColumn(verse_key,0);
	AddColumn(preview,1);
}

bool
ResultListView::InitiateDrag( BPoint point, int32 index, bool )
{
	bool success = false;
	BRow* row = CurrentSelection( NULL );
	if ( row ) {
		// create drag message
		BMessage msg( fDragCommand );
		MakeDragMessage( &msg );
		// figure out drag rect
		float width = Bounds().Width();
		BRect dragRect(0.0, 0.0, width, -1.0);
		// figure out, how many items fit into our bitmap
		int32 numItems;
		bool fade = false;
		for (numItems = 0; BRow* item = CurrentSelection( item ) ; numItems++) {
			dragRect.bottom += ceilf( item->Height() ) + 1.0;
			if ( dragRect.Height() > MAX_DRAG_HEIGHT ) {
				fade = true;
				dragRect.bottom = MAX_DRAG_HEIGHT;
				numItems++;
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
				BRow* item = NULL;	
				for ( int32 i = 0; i < numItems; i++ ) {			
					item = CurrentSelection( item );
					itemBounds.bottom = itemBounds.top + ceilf( item->Height() );
					if ( itemBounds.bottom > dragRect.bottom )
						itemBounds.bottom = dragRect.bottom;
					for (int32 q = 0; q < CountColumns(); q++)
					{
						BColumn* column = ColumnAt(q);
						column->DrawField(item->GetField(0), itemBounds, v);
						column->DrawField(item->GetField(1), itemBounds, v);
					}
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


void
ResultListView::MakeDragMessage(BMessage* message) const
{
	BLanguage language;
	BLocale::Default()->GetLanguage(&language);
	for (int32 i = 0; i < CountRows(); i++)
	{
		const ResultRow* tmpRow = dynamic_cast<const ResultRow*>(RowAt(i));
		if (tmpRow != NULL)
		{
			if (tmpRow->HasLatch() != true)
			{
				const BStringField* tmpField = dynamic_cast<const BStringField*>(tmpRow->GetField(0));
				if (tmpField != NULL)
					message->AddString("key", tmpField->String());
				tmpField = dynamic_cast<const BStringField*>(tmpRow->GetField(1));
				if (tmpField != NULL)
					message->AddString("text",tmpField->String());
				message->AddString("locale", language.Code());
			}
		}
	}
}
