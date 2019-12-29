/*
 * Copyright 2019 Paradoxianer <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef RESULT_LIST_VIEW_H
#define RESULT_LIST_VIEW_H

#include <ColumnListView.h>
#include <SupportDefs.h>

#define MAX_DRAG_HEIGHT		200.0
#define ALPHA				170
#define TEXT_OFFSET			5.0

class ResultRow : public BRow
{
public:
	ResultRow(char* key, char* text);
	virtual ~ResultRow(void);
	char* GetKey(void) const { return fKey; }
	char* GetText(void) const { return fText; }
	void SetKey(char* key) { fKey = key; }
	void SetText(char* text) { fText = text; }

private:
	char *fKey;
	char *fText;
};

class ResultListView : public BColumnListView{
public:
	ResultListView(BRect rect, const char* name);
	ResultListView(const char* name);
	virtual			~ResultListView();
	virtual	bool	InitiateDrag(BPoint point, int32 index, bool wasSelected);
	virtual void	MakeDragMessage(BMessage* message) const = 0;
				
protected:
	uint32			fDragCommand;

private:
	void			Init(void);

};


#endif // _H
