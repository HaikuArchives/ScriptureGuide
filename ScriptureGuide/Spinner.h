#ifndef SPINNER_H_
#define SPINNER_H_

#include <Control.h>
#include <TextView.h>
#include <Button.h>
#include <StringView.h>
#include <TextControl.h>

enum
{
	M_UP='mmup',
	M_DOWN,
	M_TEXT_CHANGED='mtch'
};

class Spinner : public BControl
{
public:
	Spinner(const char *name, const char *label, BMessage *msg
			,uint32 flags=B_WILL_DRAW|B_NAVIGABLE);
	virtual ~Spinner(void);
	virtual void AttachedToWindow();
	virtual void ValueChanged(int32 value);
	virtual void MessageReceived(BMessage *msg);

	virtual void SetSteps(int32 stepsize);
	int32 GetSteps(void) const { return fStep; }
	
	virtual void SetRange(int32 min, int32 max);
	void GetRange(int32 *min, int32 *max);
	
	virtual void SetMax(int32 max);
	int32 GetMax(void) const { return fMax; }
	virtual void SetMin(int32 min);
	int32 GetMin(void) const { return fMin; }
	
	virtual void MakeFocus(bool value=true);
	
	virtual void SetValue(int32 value);
	virtual void SetLabel(const char *text);
	BTextControl *TextControl(void) const { return fTextControl; }
	
	virtual void SetEnabled(bool value);
	
private:
	BTextControl *fTextControl;
	BButton *fUpButton, *fDownButton;
	
	int32 fStep;
	int32 fMin, fMax;
};

#endif
