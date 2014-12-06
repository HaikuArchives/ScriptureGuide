/*
	FontPanel.cpp: A general-purpose font selection class
	Written by DarkWyrm <bpmagic@columbus.rr.com>, Copyright 2004
	Released under the MIT license.
*/

#include <Application.h>
#include "FontPanel.h"
#include <ColumnListView.h>
#include <ColumnTypes.h>
//#include "ColorTools.h"
#include "Spinner.h"
#include <Invoker.h>
#include <String.h>
#include <stdio.h>
#include <ScrollView.h>
#include <ScrollBar.h>

// TODO: Add Escape key as a shortcut for cancelling

enum
{
	M_OK='m_ok',
	M_CANCEL,
	M_SIZE_CHANGE,
	M_SELECTION_CHANGE
};

class FontField : public BField
{
public:
	FontField(BFont font) { SetFont(font); }
	FontField(font_family family, font_style style);
	void SetFont(BFont font);
	BFont Font(void) const { return fFont; }
	const char *FullName(void) { return fFullName; }
private:
	BFont fFont;
	char fFullName[255];
};

FontField::FontField(font_family family, font_style style)
{
	fFont.SetFamilyAndStyle(family, style);
	sprintf(fFullName,"%s %s",family,style);
}

void FontField::SetFont(BFont font)
{
	fFont=font;

	font_family fam;
	font_style sty;
	font.GetFamilyAndStyle(&fam, &sty);
	sprintf(fFullName,"%s %s",fam,sty);
}

class FontColumn : public BTitledColumn
{
public:
	FontColumn(const char *title, float width,float minWidth,float maxWidth,
			uint32 truncate, alignment align = B_ALIGN_LEFT);
	virtual void DrawField (BField* field, BRect rect, BView* parent);
	virtual int	CompareFields (BField* field1, BField* field2);
	virtual	bool AcceptsField(const BField* field) const;
	const char *Text(void) { return fText.String(); }
	void SetText(const char *text);
	void SetFontSize(float size);
	float FontSize(void) const { return fFontSize; }
	void DrawString(const char*string, BView *parent,BRect rect);
private:
	BString fText,fClippedText;
	float fWidth;
	float fFontSize;
};

FontColumn::FontColumn(const char *title, float width,float minWidth,float maxWidth,
		uint32 truncate, alignment align)
 :BTitledColumn(title, width, minWidth, maxWidth, align)
{
	fText="AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz1234567890";
	fFontSize=32;
}

void FontColumn::SetFontSize(float size)
{
	fFontSize=size;
	font_height fh;
	BFont font;
	font.SetSize(size);
	font.GetHeight(&fh);
}

void FontColumn::DrawField (BField* field, BRect rect, BView* parent)
{
	float width = rect.Width() - 16;
	FontField *ffield = static_cast<FontField*>(field);

	if (width != fWidth)
	{
		BString	out_string(fText);

		parent->TruncateString(&out_string, B_TRUNCATE_END, width + 2);
		fClippedText=out_string.String();
		fWidth=width;
	}
	BFont font=ffield->Font();
	font.SetSize(fFontSize);
	parent->SetFont(&font);
	DrawString(fClippedText.String(), parent, rect);
}

// We implement this over the one in BTitledColumn because the text layout calculations
// suck for our purposes. 
void FontColumn::DrawString(const char*string, BView *parent,BRect rect)
{
	float		width = rect.Width() - 16;
	float		y;
	BFont		font;
	font_height	finfo;

	parent->GetFont(&font);
	font.GetHeight(&finfo);
	y = (rect.bottom - (finfo.ascent + finfo.descent + finfo.leading) / 4);

	switch (Alignment())
	{
		case B_ALIGN_LEFT:
			parent->MovePenTo(rect.left + 8, y);
			break;

		case B_ALIGN_CENTER:
			parent->MovePenTo(rect.left + 8 + ((width - font.StringWidth(string)) / 2), y);
			break;

		case B_ALIGN_RIGHT:
			parent->MovePenTo(rect.right - 8 - font.StringWidth(string), y);
			break;
	}
	parent->DrawString(string);
}

int	FontColumn::CompareFields(BField* field1, BField* field2)
{
	return ICompare(((FontField*)field1)->FullName(),
		((FontField*)field2)->FullName() );
}

bool FontColumn::AcceptsField(const BField* field) const
{
	return static_cast<bool>(dynamic_cast<const FontField*>(field));
}

class FontView : public BView
{
public:
	
	FontView(const BRect &frame, float size);
	~FontView(void);
	void AttachedToWindow(void);
	void MessageReceived(BMessage *msg);
	
	void SetHideWhenDone(bool value);
	bool HideWhenDone(void) const;
	void SetTarget(BMessenger msgr);
	void SetMessage(BMessage *msg);
	void SetFontSize(uint16 size);
	void SelectFont(const BFont &font);
	
private:
	
	BMessenger *fMessenger;
	BMessage fMessage;
	bool fHideWhenDone;
	
	BColumnListView *fFontList;
	BButton *fOK, *fCancel;
	Spinner *fSpinner;
	BScrollBar *fScrollBar;
};

FontView::FontView(const BRect &frame, float size)
 : BView(frame, "fontview", B_FOLLOW_ALL, B_WILL_DRAW)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	fMessenger=new BMessenger(be_app_messenger);
	fMessage.what=M_FONT_SELECTED;
	fHideWhenDone=true;
	
	BRect r(Bounds());
	r.InsetBy(10,10);
	r.bottom-=50;

	
	fFontList=new BColumnListView(r,"fontlist",B_FOLLOW_ALL,B_WILL_DRAW|B_NAVIGABLE);
	AddChild(fFontList);
	
	BStringColumn *col=new BStringColumn("Font Name",r.Width()/3,StringWidth("Font Name"),
			r.Width(),B_TRUNCATE_END);
	fFontList->ScrollView()->SetResizingMode(B_FOLLOW_ALL);
	fFontList->AddColumn(col,0);
	fFontList->SetSortColumn(col,true,true);
	fFontList->SetSelectionMode(B_SINGLE_SELECTION_LIST);
	
	// tweak the display colors so they seem a little more BeOS-like
	fFontList->SetColor(B_COLOR_BACKGROUND,make_color(255,255,255,255));
	fFontList->SetColor(B_COLOR_SELECTION,make_color(230,230,230,255));
	fFontList->SetInvocationMessage(new BMessage(M_OK));

	FontColumn *fcol=new FontColumn("Preview",r.Width()*2,StringWidth("Preview"),r.Width()*2,
			B_TRUNCATE_END);
	fcol->SetFontSize(size);
	fFontList->AddColumn(fcol,1);
	fFontList->SetColumnFlags(B_ALLOW_COLUMN_RESIZE);
	
	
	r=Bounds().InsetByCopy(10,10);
	r.top=r.bottom-27;
	r.right=r.left+100;
	r.OffsetBy(25,0);
	fSpinner=new Spinner(r,"spinner","Font Size: ", new BMessage(M_SIZE_CHANGE),
			B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
	AddChild(fSpinner);
	BTextControl *tcontrol=fSpinner->TextControl();
	tcontrol->SetDivider(StringWidth("Font Size: ")+5);
	fSpinner->SetRange(6,999);
	fSpinner->SetValue(32);
	
	r.Set(0,0,50,50);
	fCancel=new BButton(r,"Cancel","Cancel",new BMessage(M_CANCEL),B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	fCancel->ResizeToPreferred();
	
	r=fCancel->Frame();
	r.OffsetTo(Bounds().right-((r.Width()+10)*2)-10,Bounds().bottom-r.Height()-10);
	fCancel->MoveTo(r.left,r.top);
	AddChild(fCancel);
	
	r.OffsetBy(r.Width()+10,0);
	fOK=new BButton(r,"OK","OK",new BMessage(M_OK),B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	AddChild(fOK);
	
	//Available fonts
	font_family plain_family;
	font_style plain_style;
	be_plain_font->GetFamilyAndStyle(&plain_family,&plain_style);
	int32 rowsize= int32(4*size)/3;
	BRow *selectionrow;
	
	int32 numFamilies = count_font_families();
	for ( int32 i = 0; i < numFamilies; i++ )
	{
		font_family localfamily;
		if ( get_font_family ( i, &localfamily ) == B_OK ) 
		{
			int32 numStyles=count_font_styles(localfamily);
			for(int32 j = 0;j<numStyles;j++)
			{
				font_style style;
				uint32 flags;
				if( get_font_style(localfamily,j,&style,&flags)==B_OK)
				{
					BRow *row=new BRow(rowsize);
					fFontList->AddRow(row);
					if(i==0)
						selectionrow=row;
					
					BString string(localfamily);
					string+=" ";
					string+=style;
					
					BStringField *field=new BStringField(string.String());
					row->SetField(field,0);
					FontField *ffield=new FontField(localfamily,style);
					row->SetField(ffield,1);
				}
			}
		}
	}
	fFontList->SetFocusRow(selectionrow,true);
}

FontView::~FontView(void)
{
	delete fMessenger;
}

void FontView::AttachedToWindow(void)
{
	Window()->SetDefaultButton(fOK);
	fOK->SetTarget(this);
	fCancel->SetTarget(this);
	fFontList->SetTarget(this);
	fSpinner->SetTarget(this);
	
	// Do some "magic" because the ColumnListView's scrollview isn't one, so
	// we find the scrollbars themselves. Having the source is nice sometimes. :P
	fScrollBar=fFontList->ScrollBar(B_VERTICAL);
	fScrollBar->SetSteps(20,130);
	
	fFontList->MakeFocus(true);
}

void FontView::SetHideWhenDone(bool value)
{
	fHideWhenDone=value;
}

bool FontView::HideWhenDone(void) const
{
	return fHideWhenDone;
}

void FontView::SetTarget(BMessenger msgr)
{
	delete fMessenger;
	fMessenger=new BMessenger(msgr);
}

void FontView::SetMessage(BMessage *msg)
{
	fMessage=(msg)? *msg : BMessage(M_FONT_SELECTED);
}

void FontView::SetFontSize(uint16 size)
{
	if(Window())
		Window()->DisableUpdates();
	
	fSpinner->SetValue(size);
	
	FontColumn *col=(FontColumn*)fFontList->ColumnAt(1);
	col->SetFontSize(fSpinner->Value());

	int32 newheight=(4*fSpinner->Value())/3;
	for(int32 i=0; i<fFontList->CountRows(); i++)
	{
		BRow *row=fFontList->RowAt(i);
//		row->SetHeight(newheight); // TODO
		fFontList->UpdateRow(row);
	}
	
	float newrange=newheight * fFontList->CountRows();
	
	fScrollBar->SetRange(0,newrange);
	fScrollBar->ValueChanged(fScrollBar->Value());
	
	fFontList->ScrollTo(fFontList->FocusRow());
	
	if(Window())
		Window()->EnableUpdates();
}

void FontView::SelectFont(const BFont &font)
{
font_family fam;
font_style sty;
font.GetFamilyAndStyle(&fam,&sty);
	
	int32 rowcount=fFontList->CountRows();
	int32 fontID=font.FamilyAndStyle();
	
	for(int32 i=0; i<rowcount; i++)
	{
		BRow *row=fFontList->RowAt(i);
		
		FontField *ffield=(FontField*) row->GetField(1);
		
		// We actually do not want to just use the == operator because
		// it also compares size. We only care about family and style, ignoring
		// size, so we compare the 32-bit identifiers that each BFont object
		// uses internally to identify itself from other family/style combinations
		if(ffield->Font().FamilyAndStyle()==fontID)
		{
			fFontList->SetFocusRow(row,true);
			fFontList->ScrollTo(row);
			return;
		}
	}
}

void FontView::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
		case M_SIZE_CHANGE:
		{
			SetFontSize(fSpinner->Value());
			break;
		}
		case M_CANCEL:
		{
			BMessage *cancel=new BMessage(B_CANCEL);
			cancel->AddPointer("source",this);
			fMessenger->SendMessage(cancel);
			
			if(fHideWhenDone)
				Window()->Hide();
			
			break;
		}
		case M_OK:
		{
			BMessage *ok=new BMessage;
			*ok=fMessage;
			
			BRow *row=fFontList->FocusRow();
			FontField *field=(FontField*)row->GetField(1);
			BFont font=field->Font();
			
			font_family family;
			font_style style;
			
			font.GetFamilyAndStyle(&family, &style);
			
			ok->AddString("family",family);
			ok->AddString("style",style);
			ok->AddFloat("size",fSpinner->Value());
			
			fMessenger->SendMessage(ok);
			
			if(fHideWhenDone)
				Window()->Hide();
				
			break;
		}
		default:
		{
			BView::MessageReceived(msg);
			break;
		}
	}
}

class FontWindow : public BWindow
{
public:
	FontWindow(const BRect &frame, float fontsize);
	~FontWindow(void) {}
	bool QuitRequested(void);
	
	void ReallyQuit(void) { fReallyQuit=true; }
	
	FontView *fView;

private:
	bool fReallyQuit;
};

FontWindow::FontWindow(const BRect &frame, float fontsize)
 : BWindow(frame,"Choose a Font", B_TITLED_WINDOW_LOOK,B_NORMAL_WINDOW_FEEL, B_NOT_CLOSABLE)
{
	SetSizeLimits(400,2400,300,2400);
	fReallyQuit=false;
	
	fView=new FontView(Bounds(), fontsize);
	AddChild(fView);
}

bool FontWindow::QuitRequested(void)
{
	if(fReallyQuit)
		return true;
	
	return false;
}
 
FontPanel::FontPanel(BMessenger *target,BMessage *msg, float size, bool modal,
		bool hide_when_done)
{
	fWindow=new FontWindow(BRect(200,200,600,500),size);
	
	if(target)
		fWindow->fView->SetTarget(*target);
	
	if(msg)
		fWindow->fView->SetMessage(msg);
	
	if(modal)
		fWindow->SetFeel(B_MODAL_APP_WINDOW_FEEL);
	
	
	fWindow->fView->SetFontSize(size);
	fWindow->fView->SetHideWhenDone(hide_when_done);
}

FontPanel::~FontPanel(void)
{
	fWindow->ReallyQuit();
	fWindow->PostMessage(B_QUIT_REQUESTED);
}

void FontPanel::SelectFont(const BFont &font)
{
	fWindow->fView->SelectFont(font);
}

void FontPanel::Show()
{
	fWindow->Show();
}

void FontPanel::Hide()
{
	fWindow->Hide();
}

bool FontPanel::IsShowing(void) const
{
	return !fWindow->IsHidden();
}

BWindow *FontPanel::Window(void) const
{
	return fWindow;
}

void FontPanel::SetTarget(BMessenger msgr)
{
	fWindow->fView->SetTarget(msgr);
}

void FontPanel::SetMessage(BMessage *msg)
{
	fWindow->fView->SetMessage(msg);
}

void FontPanel::SetHideWhenDone(bool value)
{
	fWindow->fView->SetHideWhenDone(value);
}

bool FontPanel::HideWhenDone(void) const
{
	return fWindow->fView->HideWhenDone();
}

void FontPanel::SetFontSize(uint16 size)
{
	fWindow->fView->SetFontSize(size);
}

