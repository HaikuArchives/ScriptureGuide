/*
	CurlGetter.h: A simple download-only wrapper for libcurl
	Written by DarkWyrm <bpmagic@columbus.rr.com>, Copyright 2007
	Released under the MIT license.
*/
#ifndef CURLGETTER_H
#define CURLGETTER_H

#include <OS.h>
#include <String.h>
#include <DataIO.h>
#include <curl/curl.h>
#include <curl/easy.h>

class CurlGetter
{
public:
				CurlGetter(BDataIO *out);
	virtual		~CurlGetter(void);
	
	status_t	InitCheck(void);
	
	void		SetOutput(BDataIO *out);
	BDataIO *	Output(void) const { return fOutput; }
	
	void		SetUserPassword(const char *user, const char *pass);
	const char *UserPassword(void) const { return fAuthString.String(); }
	
	void		ShowDebugOutput(const bool &value);
	bool		IsShowingDebugOutput(void) const { return fVerbose; }
	
	void		IncludeHeader(const bool &value);
	bool		IsIncludingHeader(void) const { return fAddHeader; }
	
	CURLcode	GetURL(const char *url);
	CURLcode	GetFTPListing(const char *url, const char *dir, BString &output);
	
	virtual size_t	Write(void *data, size_t size);
	virtual int		Progress(double dcurrent, double dtotal, double ucurrent,
							double utotal);
	
	const char *ErrorString(void) { return curl_easy_strerror(fStatus); }
	
	
private:
	static size_t	WriteGlue(void *ptr, size_t size, size_t nmemb, 
							  void *stream);
	static int		ProgressGlue(void *ptr, double dtotal, double dcurrent,
								double utotal, double ucurrent);
	
	CURL		*fHandle;	
	
	BDataIO		*fOutput;
	
	bool		fDirty;
	bool		fVerbose;
	bool		fAddHeader;
	
	BString		fURL;
	BString		fAuthString;
	
	CURLcode	fStatus;
};

#endif
