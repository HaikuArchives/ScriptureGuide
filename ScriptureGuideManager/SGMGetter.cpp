#include "SGMGetter.h"
#include <Window.h>
#include <math.h>

SGMGetter::SGMGetter(BDataIO *dataout, BTextView *stdout)
 :	CurlGetter(dataout),
 	fTextView(stdout)
{

}
	
int SGMGetter::Progress(double dcurrent, double dtotal, double ucurrent,
						double utotal)
{
	if (fTextView->Window())
	{
		double percentage = floor(dcurrent / dtotal);
		if (percentage != fProgress)
		{
			fProgress = percentage;
			
			BString output;
			if (int32(fProgress) % 5 == 0)
				output << int32(fProgress);
			else
				output = ".";
			
			fTextView->Window()->Lock();
			fTextView->Insert(output.String());
			fTextView->Window()->Unlock();
		}
	}
	return 0;
}

