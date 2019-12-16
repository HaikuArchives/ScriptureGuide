#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Button.h>
#include <ColumnListView.h>
#include <ListView.h>
#include <MenuBar.h>
#include <Menu.h>
#include <MenuItem.h>
#include <ScrollView.h>
#include <StringList.h>
#include <TextView.h>
#include <Window.h>


class MainWindow : public BWindow
{
public:
	MainWindow(BRect frame);
	bool QuitRequested(void);
	void MessageReceived(BMessage *msg);
private:
	static int32 ApplyThread(void *data);
	BListView *fListView;
	BColumnListView	*fBookListView;
	BScrollView *fTextScrollView;
	BTextView *fTextView;
	BButton *fApplyButton;
	
	thread_id fApplyThread;
	BStringList fInstallList, fUninstallList;
};

#endif

