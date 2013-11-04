#ifndef __LAPP_H__
#define __LAPP_H__

/* Scripture Guide - LogosApp.h
 *
 * Published under the GNU General Public License
 * see LICENSE for details
 *
 */

class LogosApp : public BApplication 
{
	public:
		LogosApp();
		~LogosApp(void);
		virtual void MessageReceived(BMessage *message);
		status_t StartupCheck(void);
	private:
		int32 window_count;
		int32 next_untitled_number;
		
		int32 old_font_cache_size;
};

const char *GetAppPath(void);
bool HelpAvailable(void);

#endif
