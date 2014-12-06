#include "BitmapView.h"
#include <ClassInfo.h>

BitmapView::BitmapView(BRect rect,
					const char* name,
					uint32 resizing_mode,
					BBitmap *bitmap,
					rgb_color bgcolor)
	:BView(rect,name,resizing_mode,B_WILL_DRAW)
{
	fBitmap = bitmap;

	SetViewColor(bgcolor);
}

BitmapView::~BitmapView()
{
	delete fBitmap;
}

void
BitmapView::Draw(BRect updateRect)
{
	drawing_mode mode = DrawingMode();
	SetDrawingMode(B_OP_ALPHA);
	if(fBitmap != NULL)
		DrawBitmap(fBitmap,BPoint(0,0));
	SetDrawingMode(mode);
}

void
BitmapView::SetBitmap(BBitmap *bitmap)
{
	delete fBitmap;
	fBitmap = bitmap;
}
