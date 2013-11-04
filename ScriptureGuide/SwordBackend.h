#ifndef __SWORDBACKEND_H__
#define __SWORDBACKEND_H__

/* Scripture Guide - SwordBackend.h
 *
 * Published under the GNU General Public License
 * see LICENSE for details
 *                          
 */                                                                                   

#include <swmgr.h>
#include <swtext.h>
#include <vector>
#include "ObjectList.h"

typedef enum
{
	TEXT_UNASSIGNED=-1,
	TEXT_UNKNOWN=0,
	TEXT_GENERIC,
	TEXT_BIBLE,
	TEXT_COMMENTARY,
	TEXT_LEXICON
} TextType;

class SGModule
{
public:

	SGModule(const BString &name, const BString &fullname);
	const char *Name(void) const { return fName.String(); }
	const char *FullName(void) const { return fFullName.String(); }

private:
	
	BString fName, fFullName;
};

typedef BObjectList<SGModule> SGModuleList;

class SwordBackend
{
public:
	SwordBackend();
	~SwordBackend();
	
	int32 CountModules(void) const;
	int32 CountBibles(void) const;
	int32 CountCommentaries(void) const;
	int32 CountLexicons(void) const;
	int32 CountGeneralTexts(void) const;
	
	SGModule *BibleAt(const int32 &index) const;
	SGModule *CommentaryAt(const int32 &index) const;
	SGModule *LexiconAt(const int32 &index) const;
	SGModule *GeneralTextAt(const int32 &index) const;
	
	vector<const char*> getModuleDescriptions();
	status_t setModule(const char *module);
	vector<const char*> getBookNames();
	TextType Type();
	
	bool isGreek();
	bool isHebrew();
	
	int getNumberOfChapters(const char* book); 
	int getNumberOfVerses(const char* book, int chapter); 
	const char *getVerse(const char* book, int chapter, int verse);
	const char *getVerse(const char* key);
	const char *getKey(void);
	void setKey(const char *key);
	void setVerse(const char* book, int chapter, int verse);
	const char *getBookName(const char* key);
	int getChapterNumber(const char* key);
	int getVerseNumber(const char* key);
	vector<const char*> SwordBackend::searchModule(int searchType, int flags, 
				const char* searchText, const char* scopeFrom, const char* scopeTo, BStatusBar* statusBar);
 
private:
	sword::SWMgr *mymgr;
	sword::SWModule *myModule;
	TextType modType;
	
	SGModuleList *bibleList,*commentList,*lexiconList,*textList;
};
#endif
