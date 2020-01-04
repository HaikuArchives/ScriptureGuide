/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "TextDocumentTest.h"
#include "StyledEditPlusDefs.h"
#include "FontPanel.h"

#include <math.h>
#include <stdio.h>

#include <Button.h>
#include <IconUtils.h>
#include <Catalog.h>
#include <Roster.h>

#include <LayoutBuilder.h>
#include <ScrollView.h>
#include <StringView.h>
#include <Window.h>
#include <private/shared/ToolBar.h>


#include "MarkupParser.h"
#include "TextDocumentView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TextDocumentTest"

using namespace BPrivate;

TextDocumentTest::TextDocumentTest()
	:
	BApplication("application/x-vnd.Haiku-StyledEditPlus")
{
}


TextDocumentTest::~TextDocumentTest()
{
	
}


void
TextDocumentTest::ReadyToRun()
{
	Init();
	BRect frame(50.0, 50.0, 749.0, 549.0);

	BWindow* window = new BWindow(frame, "Text document test",
		B_TITLED_WINDOW, B_QUIT_ON_WINDOW_CLOSE | B_AUTO_UPDATE_SIZE_LIMITS);

	TextDocumentView* documentView = new TextDocumentView("text document view");

	BScrollView* scrollView = new BScrollView("text scroll view", documentView,
		false, true, B_NO_BORDER);
	BuildFontMenu();


	BToolBar* toolBar= new BToolBar();
	toolBar->AddAction(MENU_FILE_NEW,this,LoadIcon("document_new"));
	toolBar->AddAction(MENU_FILE_OPEN,this,LoadIcon("document_open"));
	toolBar->AddAction(MENU_FILE_SAVE,this,LoadIcon("document_save"));
	toolBar->AddSeparator();
	toolBar->AddView(new BButton("StyleList"));
	toolBar->AddView(fFontMenuField);
	toolBar->AddView(new BButton("Size"));
	toolBar->AddSeparator();
	toolBar->AddAction(FONTBOLD_MSG,this,LoadIcon("text_bold"),"Bold",NULL,false);
	toolBar->AddAction(FONTITALIC_MSG,this,LoadIcon("text_italic"),"Bold",NULL,true);;
	toolBar->AddAction(FONTUNDERLINE_MSG,this,LoadIcon("text_underline"),"Bold",NULL,true);;
	toolBar->AddSeparator();
	/*toolBar->AddView(new BButton("Left"));
	toolBar->AddView(new BButton("Center"));
	toolBar->AddView(new BButton("Right"));
	toolBar->AddView(new BButton("Block"));
	toolBar->AddSeparator();
	toolBar->AddView(new BButton("Inset Right"));
	toolBar->AddView(new BButton("Inset Left"));
	toolBar->AddSeparator();
	toolBar->AddView(new BButton("Bullet List"));
	toolBar->AddSeparator();*/
	
	
	
	toolBar->AddGlue();
	BToolBar* statusBar= new BToolBar();
	BFont* tmpFont	= new BFont();
	BStringView* stringView = new BStringView("firstStatus","Here will be the Status View");
	stringView->GetFont(tmpFont);
	tmpFont->SetSize(tmpFont->Size()-2);
	stringView->SetFont(tmpFont);
	statusBar->AddView(stringView);
	statusBar->AddSeparator();
	BLayoutBuilder::Group<>(window, B_VERTICAL,0)
		.Add(toolBar)
		.Add(scrollView)
		.SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED))
		.Add(statusBar)

	;

	CharacterStyle regularStyle;

	float fontSize = regularStyle.Font().Size();

	ParagraphStyle paragraphStyle;
	paragraphStyle.SetJustify(true);
	paragraphStyle.SetSpacingTop(ceilf(fontSize * 0.6f));
	paragraphStyle.SetLineSpacing(ceilf(fontSize * 0.2f));

	CharacterStyle boldStyle(regularStyle);
	boldStyle.SetBold(true);

	CharacterStyle italicStyle(regularStyle);
	italicStyle.SetItalic(true);

	CharacterStyle italicAndBoldStyle(boldStyle);
	italicAndBoldStyle.SetItalic(true);

	CharacterStyle bigStyle(regularStyle);
	bigStyle.SetFontSize(24);
	bigStyle.SetBold(true);
	bigStyle.SetForegroundColor(255, 50, 50);

	TextDocumentRef document(new TextDocument(), true);

	Paragraph paragraph(paragraphStyle);
	paragraph.Append(TextSpan("This is a", regularStyle));
	paragraph.Append(TextSpan(" test ", bigStyle));
	paragraph.Append(TextSpan("to see if ", regularStyle));
	paragraph.Append(TextSpan("different", boldStyle));
	paragraph.Append(TextSpan(" character styles already work.", regularStyle));
	document->Append(paragraph);

	paragraphStyle.SetSpacingTop(8.0f);
	paragraphStyle.SetAlignment(ALIGN_CENTER);
	paragraphStyle.SetJustify(false);

	paragraph = Paragraph(paragraphStyle);
	paragraph.Append(TextSpan("Different alignment styles ", regularStyle));
	paragraph.Append(TextSpan("are", boldStyle));
	paragraph.Append(TextSpan(" supported as of now!", regularStyle));
	document->Append(paragraph);

	paragraphStyle.SetAlignment(ALIGN_RIGHT);
	paragraphStyle.SetJustify(true);

	paragraph = Paragraph(paragraphStyle);
	paragraph.Append(TextSpan("I am on the ", regularStyle));
	paragraph.Append(TextSpan("Right", boldStyle));
	paragraph.Append(TextSpan("Side", italicStyle));
	document->Append(paragraph);

	// Test a bullet list
	paragraphStyle.SetSpacingTop(8.0f);
	paragraphStyle.SetAlignment(ALIGN_LEFT);
	paragraphStyle.SetJustify(true);
	paragraphStyle.SetBullet(Bullet("->", 12.0f));
	paragraphStyle.SetLineInset(10.0f);

	paragraph = Paragraph(paragraphStyle);
	paragraph.Append(TextSpan("Even bullet lists are supported.", regularStyle));
	document->Append(paragraph);

	paragraph = Paragraph(paragraphStyle);
	paragraph.Append(TextSpan("The wrapping in ", regularStyle));
	paragraph.Append(TextSpan("this", italicStyle));

	paragraph.Append(TextSpan(" bullet item should look visually "
		"pleasing. And ", regularStyle));
	paragraph.Append(TextSpan("why", italicAndBoldStyle));
	paragraph.Append(TextSpan(" should it not?", regularStyle));
	document->Append(paragraph);

/*	MarkupParser parser(regularStyle, paragraphStyle);

	TextDocumentRef document = parser.CreateDocumentFromMarkup(
		"== Text document test ==\n"
		"This is a test to see if '''different''' "
		"character styles already work.\n"
		"Different alignment styles '''are''' supported as of now!\n"
		" * Even bullet lists are supported.\n"
		" * The wrapping in ''this'' bullet item should look visually "
		"pleasing. And ''why'' should it not?\n"
	);*/

	documentView->SetTextDocument(document);
	documentView->SetTextEditor(TextEditorRef(new TextEditor(), true));
	documentView->MakeFocus();

	window->Show();
	FontPanel *fPanel = new FontPanel();
	fPanel->Show();	
}


void TextDocumentTest::BuildToolBar(void)
{	
}

void TextDocumentTest::BuildUI(void)
{
}

void TextDocumentTest::BuildFontMenu(void)
{
	fFontFamilyMenu = new BPopUpMenu("fontfamlilymenu");
	fFontMenuField = new BMenuField("FontMenuField",
//		B_TRANSLATE("Font:"), fFontFamilyMenu);
		"", fFontFamilyMenu);
	_UpdateFontmenus(true);
}

void
TextDocumentTest::_UpdateFontmenus(bool setInitialfont)
{
	BFont font = BFont();
	BMenu* stylemenu = NULL;

	font_family fontFamilyName, currentFamily;
	font_style fontStyleName, currentStyle;

	//GetFont(&font);
	font.GetFamilyAndStyle(&currentFamily, &currentStyle);

	const int32 fontfamilies = count_font_families();

	fFontFamilyMenu->RemoveItems(0, fFontFamilyMenu->CountItems(), true);

	for (int32 i = 0; i < fontfamilies; i++) {
		if (get_font_family(i, &fontFamilyName) == B_OK) {
			stylemenu = new BPopUpMenu(fontFamilyName);
			stylemenu->SetLabelFromMarked(false);
			const int32 styles = count_font_styles(fontFamilyName);
			//TODO change msg.. to a suitable msg
			BMessage* familyMsg = new BMessage('todo');
			familyMsg->AddString("_family", fontFamilyName);
			BMenuItem* familyItem = new BMenuItem(stylemenu, familyMsg);
			fFontFamilyMenu->AddItem(familyItem);

			for (int32 j = 0; j < styles; j++) {
				if (get_font_style(fontFamilyName, j, &fontStyleName) == B_OK) {
					//TODO change msg.. to a suitable msg
					BMessage* fontMsg = new BMessage('todo');
					fontMsg->AddString("_family", fontFamilyName);
					fontMsg->AddString("_style", fontStyleName);

					BMenuItem* styleItem = new BMenuItem(fontStyleName, fontMsg);
					styleItem->SetMarked(false);

					// setInitialfont is used when we attach the FontField
					if (!strcmp(fontStyleName, currentStyle)
						&& !strcmp(fontFamilyName, currentFamily)
						&& setInitialfont) {
						styleItem->SetMarked(true);
						familyItem->SetMarked(true);

						BString string;
						string << currentFamily << " " << currentStyle;

						if (fFontMenuField)
							fFontMenuField->MenuItem()->SetLabel(string.String());
					}
					stylemenu->AddItem(styleItem);
				}
			}

			stylemenu->SetRadioMode(true);
			stylemenu->SetTargetForItems(this);
		}
	}

	fFontFamilyMenu->SetLabelFromMarked(false);
	fFontFamilyMenu->SetTargetForItems(this);
}

void TextDocumentTest::Init(void)
{
	toolIconWidth	= 16;
	toolIconHeight	= 16;
	app_info info;
	be_app->GetAppInfo(&info);
	res	= new BResources();
	BFile file(&info.ref, B_READ_ONLY);
	if (res->SetTo(&file) != B_OK)
	{
		res=NULL;
	}
}

BBitmap* TextDocumentTest::LoadIcon(char *icon_name)
{
	size_t size;
	BBitmap		*bmp	= new BBitmap(BRect(0,0,toolIconWidth-1,toolIconHeight-1),B_RGBA32);
	if (res != NULL){
		const void* data = res->LoadResource(B_VECTOR_ICON_TYPE, icon_name, &size);
		if (data) {
			//conver the Icon using IconUtils because we can easly specify the size of the icons
			BIconUtils::GetVectorIcon((const uint8 *)data,size,bmp);
			//bmp = BTranslationUtils::GetBitmap(new BMemoryIO(data,size));
			if (bmp) 
				return bmp;
			else
				return NULL;
		}
		else
			return NULL;
	}
}

	
void TextDocumentTest::MessageReceived(BMessage* message)
{
	
}

int
main(int argc, char* argv[])
{
	TextDocumentTest().Run();
	return 0;
}



