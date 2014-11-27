#ifndef __LAPP_H__
#define __LAPP_H__

class SGApp : public BApplication 
{
public:
					SGApp();
					~SGApp(void);
	virtual void	MessageReceived(BMessage *message);
	status_t		StartupCheck(void);
};

const char *	GetAppPath(void);
bool 			HelpAvailable(void);

#endif
