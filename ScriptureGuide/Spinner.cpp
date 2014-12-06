/*
	Spinner.cpp: A number spinner control.
	Written by DarkWyrm <bpmagic@columbus.rr.com>, Copyright 2004
	Released under the MIT license.
	
	Original BScrollBarButton class courtesy Haiku project
*/
#include "Spinner.h"

#include <stdio.h>

#include <String.h>
#include <ScrollBar.h>
#include <Window.h>
#include <Font.h>
#include <Box.h>
#include <MessageFilter.h>
#include <LayoutBuilder.h>

Spinner::Spinner(const char *name, const char *label, BMessage *msg,
		uint32 flags)
 : BControl(name,NULL,msg,flags)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	fTextControl=new BTextControl("textcontrol",label,"0",new BMessage(M_TEXT_CHANGED),
			B_WILL_DRAW|B_NAVIGABLE);
	BTextView *tview=fTextControl->TextView();
	tview->SetAlignment(B_ALIGN_LEFT);
	tview->SetWordWrap(false);
	
	BString string("QWERTYUIOP[]\\ASDFGHJKL;'ZXCVBNM,/qwertyuiop{}| "
		"asdfghjkl:\"zxcvbnm<>?!@#$%^&*()-_=+`~\r");
	
	for(int32 i=0; i<string.CountChars(); i++)
	{
		char c=string.ByteAt(i);
		tview->DisallowChar(c);
	}
	
	fUpButton=new BButton("Up", "+", new BMessage(M_UP));
	BSize size = fUpButton->MinSize();
	size.width = fUpButton->StringWidth("+") + 16;
	fUpButton->SetExplicitMinSize(size);
	fUpButton->SetExplicitMaxSize(size);
	
	fDownButton=new BButton("Down", "-", new BMessage(M_DOWN));
	fDownButton->SetExplicitMinSize(size);
	fDownButton->SetExplicitMaxSize(size);
	
	BLayoutBuilder::Group<>(this, B_HORIZONTAL, 0)
		.Add(fTextControl)
		.Add(fUpButton)
		.Add(fDownButton)
	.End();
	
	fStep=1;
	fMin=0;
	fMax=100;
	
	SetValue(0);
}

Spinner::~Spinner(void)
{
}

void Spinner::AttachedToWindow()
{
	fTextControl->SetTarget(this);
	fDownButton->SetTarget(this);
	fUpButton->SetTarget(this);
}

void Spinner::SetValue(int32 value)
{
	if(value<GetMin())
		return SetValue(GetMin());
	
	if(value>GetMax())
		return SetValue(GetMax());
	
	char string[50];
	sprintf(string,"%ld",value);
	fTextControl->SetText(string);
	
	BControl::SetValue(value);
	ValueChanged(Value());
}

void Spinner::SetLabel(const char *text)
{
	fTextControl->SetLabel(text);
}

void Spinner::ValueChanged(int32 value)
{
	Invoke();
}

void Spinner::MessageReceived(BMessage *msg)
{
	if(msg->what==M_TEXT_CHANGED)
	{
		BString string(fTextControl->Text());
		int32 newvalue=0;
		
		sscanf(string.String(),"%ld",&newvalue);
		SetValue(newvalue);
	} else if (msg->what == M_UP) {
		SetValue(Value()+fStep);
	} else if (msg->what == M_DOWN) {
		SetValue(Value()-fStep);
	} else
		BControl::MessageReceived(msg);
}

void Spinner::SetSteps(int32 stepsize)
{
	fStep=stepsize;
}

void Spinner::SetRange(int32 min, int32 max)
{
	SetMin(min);
	SetMax(max);
}

void Spinner::GetRange(int32 *min, int32 *max)
{
	*min=fMin;
	*max=fMax;
}

void Spinner::SetMax(int32 max)
{
	fMax=max;
	if(Value()>fMax)
		SetValue(fMax);
}

void Spinner::SetMin(int32 min)
{
	fMin=min;
	if(Value()<fMin)
		SetValue(fMin);
}

void Spinner::SetEnabled(bool value)
{
	if(IsEnabled()==value)
		return;
	
	BControl::SetEnabled(value);
	fTextControl->SetEnabled(value);
	fUpButton->SetEnabled(value);
	fDownButton->SetEnabled(value);
}

void Spinner::MakeFocus(bool value)
{
	fTextControl->MakeFocus(value);
}
