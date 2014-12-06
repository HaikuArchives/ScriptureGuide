#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Window.h>
#include <MenuBar.h>
#include <Menu.h>
#include <MenuItem.h>
#include <ListView.h>
#include <TextView.h>
#include <ScrollView.h>
#include <Button.h>
#include <StringList.h>

class MainWindow : public BWindow
{
public:
	MainWindow(BRect frame);
	bool QuitRequested(void);
	void MessageReceived(BMessage *msg);
private:
	static int32 ApplyThread(void *data);
	BListView *fListView;
	BScrollView *fListScrollView;
	BScrollView *fTextScrollView;
	BTextView *fTextView;
	BButton *fApplyButton;
	
	thread_id fApplyThread;
	BStringList fInstallList, fUninstallList;
};

#endif

