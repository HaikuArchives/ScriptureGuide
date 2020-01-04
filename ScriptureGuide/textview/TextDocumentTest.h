/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef TEXT_DOCUMENT_TEST_H
#define TEXT_DOCUMENT_TEST_H


#include <Application.h>
#include <PopUpMenu.h>
#include <MenuField.h>
#include <Resources.h>
#include <Bitmap.h>

class TextDocumentTest : public BApplication {
public:
								TextDocumentTest();
	virtual						~TextDocumentTest();

	virtual	void				ReadyToRun();
			void				MessageReceived(BMessage* message);
					
private:
			void				Init(void);
	
			void				BuildToolBar(void);
			BBitmap*			LoadIcon(char *icon_name);
			void 				BuildFontMenu(void);
			void				_UpdateFontmenus(bool setInitialfont = false);
			void				BuildUI(void);
			BMenuField*			fFontMenuField;
			BMenu*				fFontFamilyMenu;
			BResources			*res;
			uint32				toolIconWidth,toolIconHeight;

};


#endif // TEXT_DOCUMENT_TEST_H
