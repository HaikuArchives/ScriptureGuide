/*
	FontPanel.cpp: A general-purpose font selection class
	Written by DarkWyrm <bpmagic@columbus.rr.com>, Copyright 2004
	Released under the MIT license.
*/

#include "FontPanel.h"
#include "Spinner.h"

#include <Application.h>
#include <Catalog.h>
#include <ColumnListView.h>
#include <ColumnTypes.h>
#include <Invoker.h>
#include <LayoutBuilder.h>
#include <String.h>
#include <ScrollView.h>

#include <stdio.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "FontPanel"

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
	const char* FullName(void) { return fFullName; }
private:
	BFont fFont;
	char fFullName[255];
};


FontField::FontField(font_family family, font_style style)
{
	fFont.SetFamilyAndStyle(family, style);
	sprintf(fFullName,"%s %s",family, style);
}


void FontField::SetFont(BFont font)
{
	fFont = font;
	font_family fam;
	font_style sty;
	font.GetFamilyAndStyle(&fam, &sty);
	sprintf(fFullName,"%s %s",fam, sty);
}


class FontColumn : public BTitledColumn
{
public:
	FontColumn(const char* title, float width, float minWidth, float maxWidth,
			uint32 truncate, alignment align = B_ALIGN_LEFT);
	virtual void DrawField (BField* field, BRect rect, BView* parent);
	virtual int	CompareFields (BField* field1, BField* field2);
	virtual	bool AcceptsField(const BField* field) const;
	const char* Text(void) { return fText.String(); }
	void SetText(const char *text);
	void SetFontSize(float size);
	float FontSize(void) const { return fFontSize; }
	void DrawString(const char* string, BView* parent, BRect rect);
private:
	BString fText, fClippedText;
	float fWidth;
	float fFontSize;
};

#define PREVIEW_STR "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz1234567890"

FontColumn::FontColumn(const char* title, float width, float minWidth,
		float maxWidth, uint32 truncate, alignment align)
 :BTitledColumn(title, width, minWidth, maxWidth, align)
{
	fText = PREVIEW_STR;
	fFontSize = 32;
}


void FontColumn::SetFontSize(float size)
{
	fFontSize = size;
	font_height fh;
	BFont font;
	font.SetSize(size);
	font.GetHeight(&fh);
}


void FontColumn::DrawField (BField* field, BRect rect, BView* parent)
{
	float width = rect.Width() - 16;
	FontField* ffield = static_cast<FontField*>(field);

	if (width != fWidth)
	{
		BString	out_string(fText);

		parent->TruncateString(&out_string, B_TRUNCATE_END, width + 2);
		fClippedText = out_string.String();
		fWidth = width;
	}
	BFont font = ffield->Font();
	font.SetSize(fFontSize);
	parent->SetFont(&font);
	DrawString(fClippedText.String(), parent, rect);
}


// We implement this over the one in BTitledColumn because 
// the text layout calculations suck for our purposes. 
void FontColumn::DrawString(const char* string, BView* parent, BRect rect)
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
			parent->MovePenTo(rect.left + 8 + 
				((width - font.StringWidth(string)) / 2), y);
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


class FontWindow : public BWindow
{
public:
	FontWindow(const BRect& frame, float fontsize, BHandler* target,
			BMessage *msg);
	~FontWindow() {}
	
	virtual void MessageReceived(BMessage* msg);

	void SetFontSize(uint16 size);
	void SelectFont(const BFont &font);

	bool QuitRequested(void);
	void ReallyQuit(void) { fReallyQuit = true; }
	
private:
	bool fReallyQuit;
	BHandler* fTarget;
	BMessage fMessage;
	
	BColumnListView* fFontList;
	BButton* fOK;
	BButton* fCancel;
	Spinner* fSpinner;
};

FontWindow::FontWindow(const BRect& frame, float fontsize, BHandler* target,
				BMessage *msg)
 : BWindow(frame, "Choose a Font", B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
				B_NOT_CLOSABLE | B_CLOSE_ON_ESCAPE)
{
	fReallyQuit = false;
	fTarget = target;

	if (msg == NULL)
		fMessage.what = M_FONT_SELECTED;
	else
		fMessage = *msg;
	
	fFontList = new BColumnListView("fontlist", B_WILL_DRAW | B_NAVIGABLE);

	float width = fFontList->StringWidth(B_TRANSLATE("Font Name"));
	BStringColumn* col = new BStringColumn(B_TRANSLATE("Font Name"), width*3, width,
			width*20, B_TRUNCATE_END);
	fFontList->AddColumn(col, 0);
	fFontList->SetSortColumn(col, true, true);
	fFontList->SetSelectionMode(B_SINGLE_SELECTION_LIST);
	
	// tweak the display colors so they seem a little more BeOS-like
	fFontList->SetColor(B_COLOR_BACKGROUND, make_color(255, 255, 255, 255));
	fFontList->SetColor(B_COLOR_SELECTION, make_color(230, 230, 230, 255));
	fFontList->SetInvocationMessage(new BMessage(M_OK));

	width = fFontList->StringWidth(B_TRANSLATE("Preview"));
	float previewWidth = fFontList->StringWidth(PREVIEW_STR);
	FontColumn* fcol = new FontColumn(B_TRANSLATE("Preview"), previewWidth, width, previewWidth*2,
			B_TRUNCATE_END);
	fcol->SetFontSize(fontsize);
	fFontList->AddColumn(fcol, 1);
	fFontList->SetColumnFlags(B_ALLOW_COLUMN_RESIZE);
	
	fSpinner=new Spinner("spinner", B_TRANSLATE("Font Size: "), new BMessage(M_SIZE_CHANGE));
	BTextControl* tcontrol = fSpinner->TextControl();
	tcontrol->SetDivider(fFontList->StringWidth(B_TRANSLATE("Font Size: "))+5);
	fSpinner->SetRange(6, 999);
	fSpinner->SetValue(fontsize);
	
	fCancel = new BButton("Cancel", "Cancel", new BMessage(M_CANCEL));
	fOK = new BButton("OK", B_TRANSLATE("OK"), new BMessage(M_OK));
	
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fFontList)
		.AddGroup(B_HORIZONTAL)
			.Add(fSpinner)
			.AddGlue()
			.Add(fCancel)
			.Add(fOK)
		.End()
	.End();
	
	fOK->SetTarget(this);
	fCancel->SetTarget(this);
	fFontList->SetTarget(this);
	fSpinner->SetTarget(this);
	
	fFontList->MakeFocus(true);
	
	// Available fonts
	font_family plain_family;
	font_style plain_style;
	be_plain_font->GetFamilyAndStyle(&plain_family,&plain_style);
	int32 rowsize= int32(4*fontsize)/3;
	BRow* selectionrow;
	
	int32 numFamilies = count_font_families();
	for ( int32 i = 0; i < numFamilies; i++ )
	{
		font_family localfamily;
		if ( get_font_family ( i, &localfamily ) == B_OK ) 
		{
			int32 numStyles = count_font_styles(localfamily);
			for (int32 j = 0; j<numStyles; j++)
			{
				font_style style;
				uint32 flags;
				if (get_font_style(localfamily, j, &style, &flags) == B_OK)
				{
					BRow* row = new BRow(rowsize);
					fFontList->AddRow(row);
					if (i == 0)
						selectionrow = row;
					
					BString string(localfamily);
					string+=" ";
					string+=style;
					
					BStringField* field = new BStringField(string.String());
					row->SetField(field, 0);
					FontField* ffield = new FontField(localfamily, style);
					row->SetField(ffield, 1);
				}
			}
		}
	}
	fFontList->SetFocusRow(selectionrow, true);
}


void FontWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case M_SIZE_CHANGE:
		{
			SetFontSize(fSpinner->Value());
			break;
		}
		case M_CANCEL:
		{
			BMessage* cancel = new BMessage(B_CANCEL);
			cancel->AddPointer("source",this);
			Hide();
			
			break;
		}
		case M_OK:
		{
			BMessage* ok = new BMessage;
			ok = &fMessage;
			
			BRow* row = fFontList->FocusRow();
			FontField* field = (FontField*)row->GetField(1);
			BFont font = field->Font();
			
			font_family family;
			font_style style;
			
			font.GetFamilyAndStyle(&family, &style);
			
			ok->AddString("family",family);
			ok->AddString("style",style);
			ok->AddFloat("size",fSpinner->Value());
			
			fTarget->Looper()->PostMessage(ok, fTarget);
			Hide();
			break;
		}
		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}
}


void FontWindow::SetFontSize(uint16 size)
{
	DisableUpdates();
	
	fSpinner->SetValue(size);
	
	FontColumn* col=(FontColumn*)fFontList->ColumnAt(1);
	col->SetFontSize(fSpinner->Value());

	int32 newheight = (4*fSpinner->Value())/3;
	for (int32 i = 0; i<fFontList->CountRows(); i++)
	{
		BRow* row = fFontList->RowAt(i);
//		row->SetHeight(newheight); // TODO
		fFontList->UpdateRow(row);
	}
	
	fFontList->ScrollTo(fFontList->FocusRow());
	EnableUpdates();
}


void FontWindow::SelectFont(const BFont &font)
{
	font_family fam;
	font_style sty;
	font.GetFamilyAndStyle(&fam,&sty);
	
	int32 rowcount = fFontList->CountRows();
	int32 fontID = font.FamilyAndStyle();
	
	for (int32 i = 0; i<rowcount; i++)
	{
		BRow* row = fFontList->RowAt(i);
		
		FontField* ffield=(FontField*) row->GetField(1);
		
		// We actually do not want to just use the == operator because
		// it also compares size. We only care about family and style, ignoring
		// size, so we compare the 32-bit identifiers that each BFont object
		// uses internally to identify itself from other family/style combinations
		if (ffield->Font().FamilyAndStyle()==fontID)
		{
			fFontList->SetFocusRow(row, true);
			fFontList->ScrollTo(row);
			return;
		}
	}
}


bool FontWindow::QuitRequested(void)
{
	if (fReallyQuit)
		return true;
	else
		Hide();
		
	return false;	
}


FontPanel::FontPanel(BHandler* target, BMessage* msg, float size, bool modal,
		bool hide_when_done)
{
	fWindow = new FontWindow(BRect(200, 200, 600, 500), size, target, msg);

	if (modal)
		fWindow->SetFeel(B_MODAL_APP_WINDOW_FEEL);
	
}


FontPanel::~FontPanel(void)
{
	fWindow->ReallyQuit();
	fWindow->PostMessage(B_QUIT_REQUESTED);
}


void FontPanel::SelectFont(const BFont &font)
{
	fWindow->SelectFont(font);
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


BWindow* FontPanel::Window(void) const
{
	return fWindow;
}
