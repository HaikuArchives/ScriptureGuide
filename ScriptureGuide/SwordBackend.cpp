/* Scripture Guide - SwordBackend.cpp
 *
 * Published under the GNU General Public License
 * see LICENSE for details
 *
 * SwordBackend is the connection to the Sword Library
 * I introduced it to make things a bit easier. Not sure if it helped.
 * This backend should be improved in future.
 */                                                                                                             
#include <StatusBar.h>

#include <swmgr.h>
#include <swtext.h>
#include <localemgr.h>
#include <markupfiltmgr.h>
#include <gbfplain.h>
#include <gbfstrongs.h>

#include <vector.h>
#include <map>
#include <string.h>

#include <String.h>
#include "SwordBackend.h"
#include "constants.h"

using namespace std;

#ifndef NO_SWORD_NAMESPACE
using namespace sword;
#endif

// path for the modules; to be stored in a config file in future
#define CONFIGPATH "/boot/home/config/add-ons/scripture-guide/"

SGModule::SGModule(const BString &name, const BString &fullname)
{
	fName=name;
	fFullName=fullname;
}


// Constructor
SwordBackend::SwordBackend()
{
	mymgr = new SWMgr(CONFIGPATH, true, new MarkupFilterMgr(FMT_GBF, ENC_UTF8));
	modType=TEXT_UNASSIGNED;
	
	// We are going to replace getModuleDescriptions with some methods which
	// are a little easier to deal with outside the class
	
	// First, lists to contain the names of each type of text
	bibleList=new SGModuleList(20,true);
	commentList=new SGModuleList(20,true);
	lexiconList=new SGModuleList(20,true);
	textList=new SGModuleList(20,true);
	
	ModMap::iterator it;
        SWModule* curMod = 0;
	vector<const char*> tmp;

	// Adds headings; becomes obsolete, when we have the module manager
	for (it = mymgr->Modules.begin(); it != mymgr->Modules.end(); it++) 
	{
		curMod = (*it).second;
		
		if(!strcmp(curMod->Type(), "Biblical Texts"))
			bibleList->AddItem(new SGModule(curMod->Name(),curMod->Description()));
		else
		if(!strcmp(curMod->Type(), "Commentaries"))
			commentList->AddItem(new SGModule(curMod->Name(),curMod->Description()));
		else
		if(!strcmp(curMod->Type(), "Lexicons / Dictionaries"))
			lexiconList->AddItem(new SGModule(curMod->Name(),curMod->Description()));
		else
		if(!strcmp(curMod->Type(), "Generic Books"))
			lexiconList->AddItem(new SGModule(curMod->Name(),curMod->Description()));
		else
		{
			printf("Found module %s with type %s\n",curMod->Description(),curMod->Type());
		}
	}
}

// Destructor
SwordBackend::~SwordBackend()
{
	delete mymgr;
	
	delete bibleList;
	delete commentList;
	delete lexiconList;
	delete textList;
}

int32 SwordBackend::CountModules(void) const
{
	return mymgr->Modules.size();
}

int32 SwordBackend::CountBibles(void) const
{
	return bibleList->CountItems();
}

int32 SwordBackend::CountCommentaries(void) const
{
	return commentList->CountItems();
}

int32 SwordBackend::CountLexicons(void) const
{
	return lexiconList->CountItems();
}

int32 SwordBackend::CountGeneralTexts(void) const
{
	return textList->CountItems();
}

SGModule *SwordBackend::BibleAt(const int32 &index) const
{
	return bibleList->ItemAt(index);
}

SGModule *SwordBackend::CommentaryAt(const int32 &index) const
{
	return commentList->ItemAt(index);
}

SGModule *SwordBackend::LexiconAt(const int32 &index) const
{
	return lexiconList->ItemAt(index);
}

SGModule *SwordBackend::GeneralTextAt(const int32 &index) const
{
	return textList->ItemAt(index);
}


// Returns a vector with the available module names and descriptions
vector<const char*> SwordBackend::getModuleDescriptions()
{
	ModMap::iterator it;
        SWModule* curMod = 0;
	vector<const char*> tmp;

	// Adds headings; becomes obsolete, when we have the module manager
	tmp.push_back("heading");
	tmp.push_back("Bible");	
	tmp.push_back("--"); tmp.push_back("--");
        for (it = mymgr->Modules.begin(); it != mymgr->Modules.end(); it++) 
	{
        	curMod = (*it).second;
		if(!strcmp(curMod->Type(), "Biblical Texts"))
		{
			tmp.push_back((const char*)curMod->Name());
			tmp.push_back((const char*)curMod->Description());
		}
	}
	tmp.push_back("--"); tmp.push_back("--");
	tmp.push_back("heading");
	tmp.push_back("Commentary");	
	tmp.push_back("--"); tmp.push_back("--");
        for (it = mymgr->Modules.begin(); it != mymgr->Modules.end(); it++) 
	{
        	curMod = (*it).second;
		if(!strcmp(curMod->Type(), "Commentaries"))
		{
			tmp.push_back((const char*)curMod->Name());
			tmp.push_back((const char*)curMod->Description());
		}
	}
	
	return tmp;
}

// Sets the current module
status_t SwordBackend::setModule(const char* module)
{
	sword::SWModule *newModule = mymgr->Modules[module];
	
	if(!newModule)
		return B_ERROR;
	
	myModule = newModule;
	// myModule->AddRenderFilter(new GBFStrongs());
	myModule->AddRenderFilter(new GBFPlain());
	
	return B_OK;
}

// returns the type of module
TextType SwordBackend::Type()
{
	if(modType==TEXT_UNASSIGNED)
	{
		if(strcmp(myModule->Type(), "Biblical Texts")==0)
			modType=TEXT_BIBLE;
		else
		if(strcmp(myModule->Type(), "Commentaries")==0)
			modType=TEXT_COMMENTARY;
		else
		if(strcmp(myModule->Type(), "Lexicons / Dictionaries")==0)
			modType=TEXT_LEXICON;
		else
		if(strcmp(myModule->Type(), "Generic Text")==0)
			modType=TEXT_GENERIC;
		else
			modType=TEXT_UNKNOWN;
	}
	
	return modType;
}

// returns true, when module is Greek
bool SwordBackend::isGreek()
{
	if((!strcmp(myModule->Lang(), "grc"))||(!strcmp(myModule->Lang(), "el")))
		return true;
	else
		return false;
}

// returns true, when module is Hebrew
bool SwordBackend::isHebrew()
{
	if(!strcmp(myModule->Lang(), "he"))
		return true;
	else
		return false;
}

// returns a list with all bible books
vector<const char*> SwordBackend::getBookNames()
{
	vector<const char*> tmpVector;
	for(int i=0; i<2; i++)
		for(int j=0; j<VerseKey::builtin_BMAX[i]; j++)
			tmpVector.push_back((const char*)VerseKey::builtin_books[i][j].name);

	return tmpVector;
}

// returns the number of chapters in a given book
int SwordBackend::getNumberOfChapters(const char* book)
{
	VerseKey myKey(book);
	int i = myKey.Testament() - 1;
	int j = myKey.Book() - 1;
	return VerseKey::builtin_books[i][j].chapmax;
}

// returns the number of verses in a given chapter
int SwordBackend::getNumberOfVerses(const char* book, int chapter)
{
	VerseKey myKey(book);
	int i = myKey.Testament() - 1;
	int j = myKey.Book() - 1;
	return VerseKey::builtin_books[i][j].versemax[chapter-1];
}

// returns a verse for a given book, chapter, verse
const char *SwordBackend::getVerse(const char* book, int chapter, int verse)
{
	VerseKey myKey(book);
	myKey.Chapter(chapter);
	myKey.Verse(verse);
	myModule->SetKey(myKey);
	return (const char*)*myModule;
}

// returns a verse for a given key
const char *SwordBackend::getVerse(const char* key)
{
	VerseKey myKey(key);
	myModule->SetKey(myKey);
	return (const char*)*myModule;
}

// returns a book name for a given key
const char *SwordBackend::getBookName(const char* key)
{
	VerseKey myKey(key);
	int i = myKey.Testament() - 1;
	int j = myKey.Book() - 1;
	return (const char*)VerseKey::builtin_books[i][j].name;
}

// Obtains a string key for the current book, chapter, and verse
const char *SwordBackend::getKey(void)
{
	VerseKey *key=(VerseKey*)myModule->getKey();
	
	if(!key)
		return NULL;
	
	return key->getText();
}

void SwordBackend::setKey(const char *key)
{
	// TODO: Convert this to return a status_t - B_ERROR on failure.
	// This will depend on finding out what kinds of error codes are returned
	// when sword::SWModule::setKey fails and succeeds
	if(!key)
		return;
	
	VerseKey vkey(key);
	myModule->setKey(vkey);
}

void SwordBackend::setVerse(const char* book, int chapter, int verse)
{
	// TODO: Convert this to return a status_t - B_ERROR on failure.
	// This will depend on finding out what kinds of error codes are returned
	// when sword::SWModule::setKey fails and succeeds
	VerseKey myKey(book);
	myKey.Chapter(chapter);
	myKey.Verse(verse);
	myModule->SetKey(myKey);
}

// returns the chapter number for a given key
int SwordBackend::getChapterNumber(const char* key)
{
	VerseKey myKey(key);
	return myKey.Chapter();
}

// returns the verse number for a given key
int SwordBackend::getVerseNumber(const char* key)
{
	VerseKey myKey(key);
	return myKey.Verse();
}

// callback function: sword library calls it with percentage-done during a search
void percentUpdate(char percent, void *userData)
{
#ifdef DEBUG
	printf("Search is %d%% done.\n", percent);
#endif
	BStatusBar* bar;
	bar = (BStatusBar *)userData;
	bar->Update((float)percent - bar->CurrentValue());
}

// returns a list of search results on the current module with
// searchType: word search, phrase search, regex search
// flags: standard regex flags: see regex.h from POSIX
// searchText: the text to search for
// scopeFrom: book name to search from
// scopeTo: book name to search to
vector<const char*> SwordBackend::searchModule(int searchType, int flags, const char* searchText, 
		const char* scopeFrom, const char* scopeTo, BStatusBar* statusBar)
{
	vector<const char*> tmpVector;
	char tmpStr[100] = "";
	char tmpNum[5] = "";
	strcat(tmpStr,scopeFrom);
	strcat(tmpStr," 1:1-");
	strcat(tmpStr,scopeTo);
	int ch = getNumberOfChapters(scopeTo);
	int vs = getNumberOfVerses(scopeTo,ch);
	sprintf(tmpNum," %d:%d",ch,vs);
	strcat(tmpStr,tmpNum);

	VerseKey parse = "Gen 1:1";
	ListKey scope = parse.ParseVerseList(tmpStr, parse, true);

/*
#ifdef DEBUG
	printf("isSearchOptimallySupported? ");
	if (myModule->isSearchOptimallySupported(searchText,searchType, flags, &scope))
		printf("Yes.\n");
	else
	{
		printf("No.\n");
		
		// Disable this for a *huge* speedup
		printf("Created framework: %d\n", myModule->createSearchFramework());
	}
#endif
*/

	ListKey &lk = myModule->search(searchText,searchType, flags, &scope, 0, &percentUpdate, statusBar);
	
	lk.Persist(true);
	myModule->SetKey(lk);

	for (lk = TOP; !lk.Error(); lk++) 
	{ 
		tmpVector.push_back((const char*) lk);
	}

	return tmpVector;
}
