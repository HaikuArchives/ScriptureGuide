#ifndef SGMGETTER_H
#define SGMGETTER_H

#include <TextView.h>
#include "CurlGetter.h"

class SGMGetter : public CurlGetter
{
public:
	SGMGetter(BDataIO *dataout, BTextView *stdout);
	
	int Progress(double dcurrent, double dtotal, double ucurrent,
				double utotal);
	void ResetProgress(void) { fProgress = 0.0; }
	
private:
	BTextView	*fTextView;
	double		fProgress;
};

#endif
