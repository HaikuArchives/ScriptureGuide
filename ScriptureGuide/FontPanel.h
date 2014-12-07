/*
	FontPanel.cpp: A general-purpose font selection class
	Written by DarkWyrm <bpmagic@columbus.rr.com>, Copyright 2004
	Released under the MIT license.
*/
#ifndef FONTPANEL_H_
#define FONTPANEL_H_

#include <View.h>
#include <Window.h>
#include <Button.h>
#include <ListView.h>
#include <ListItem.h>
#include <Font.h>
#include <Message.h>
#include <Handler.h>

class FontWindow;

enum
{
	M_FONT_SELECTED='mfsl'
};

class FontPanel
{
public:
	FontPanel(BHandler* target = NULL,
		BMessage *message = NULL,
		float size = 12,
		bool modal = false,
		bool hide_when_done = true);
	virtual ~FontPanel(void);
	void SelectFont(const BFont &font);
	void Show();
	void Hide();
	bool IsShowing(void) const;
	BWindow *Window(void) const;
	void SetFontSize(uint16 size);
private:
	FontWindow *fWindow;
	
};

#endif
