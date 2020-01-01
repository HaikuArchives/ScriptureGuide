/*
 * Copyright 2019 Paradoxianer <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef RESULT_LIST_VIEW_H
#define RESULT_LIST_VIEW_H

#include <OutlineListView.h>
#include <SupportDefs.h>

#define MAX_DRAG_HEIGHT		200.0
#define ALPHA				170
#define TEXT_OFFSET			5.0

static const float kRowDragSensitivity = 5.0;


class BibleItem : public BListItem
{
public:
	BibleItem(const char* key,const  char* text);
	virtual ~BibleItem(void);
	const char* GetKey(void){ return fKey; }
	const char* GetText(void) const { return fText; }
	void SetKey(const char* key) { fKey = key; }
	void SetText(const char* text) { fText = text; }
	virtual void DrawItem(BView *owner,
            BRect frame,
            bool complete = false);
	virtual	void DrawBackground(BView* owner, BRect frame);

private:
	const char *fKey;
	const char *fText;
};

class ResultListView : public BOutlineListView{
public:
	ResultListView(const char* name, list_view_type type
					= B_SINGLE_SELECTION_LIST, uint32 flags
					= B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);

	virtual			~ResultListView();
	
	virtual bool	InitiateDrag(BPoint point, int32 index, bool);
	
protected:
	void			MakeDragMessage(BMessage* message);


private:
	void			Init(void);
	uint32			fDragCommand;

};


#endif // _H
