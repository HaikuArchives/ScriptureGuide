/*
	CurlGetter.cpp: A simple download-only wrapper for libcurl
	Written by DarkWyrm <bpmagic@columbus.rr.com>, Copyright 2007
	Released under the MIT license.
*/
#include "CurlGetter.h"

CurlGetter::CurlGetter(BDataIO *out)
 :	fHandle(curl_easy_init()),
 	fOutput(out),
 	fDirty(false),
 	fVerbose(false),
 	fAddHeader(false),
 	fStatus(CURLE_OK)
{
	if (!out)
		debugger("CurlGetter objects cannot take a NULL pointer.");
}


CurlGetter::~CurlGetter(void)
{
	if (fHandle)
		curl_easy_cleanup(fHandle);
}


status_t
CurlGetter::InitCheck(void)
{
	return (!fHandle || !fOutput) ? B_OK : B_ERROR;
}


void
CurlGetter::SetOutput(BDataIO *out)
{
	if (!out)
		debugger("CurlGetter objects cannot take a NULL pointer.");
	fOutput = out;
}


void
CurlGetter::SetUserPassword(const char *user, const char *pass)
{
	if (user) {
		fAuthString = user;
		fAuthString << ":" << pass;
	} else {
		fAuthString = "";
	}
}


void
CurlGetter::ShowDebugOutput(const bool &value)
{
	if (!fHandle)
		return;
	
	if (value != fVerbose) {
		fVerbose = value;
		curl_easy_setopt(fHandle,CURLOPT_VERBOSE, value ? 1 : 0);
	}
}


void
CurlGetter::IncludeHeader(const bool &value)
{
	if (!fHandle)
		return;
	
	if (value != fAddHeader) {
		fAddHeader = value;
		curl_easy_setopt(fHandle,CURLOPT_HEADER, value ? 1 : 0);
	}
}


CURLcode
CurlGetter::GetURL(const char *url)
{
	if (!url)
		return CURLE_URL_MALFORMAT;
	
	if (!fOutput)
		return CURLE_FAILED_INIT;
	
	if (fDirty) {
		curl_easy_reset(fHandle);
		
		// If we have to do a reset, it resets the options, so we have
		// to reinitialize the handle to use them just as before
		curl_easy_setopt(fHandle,CURLOPT_VERBOSE, fVerbose ? 1 : 0);
		curl_easy_setopt(fHandle,CURLOPT_HEADER, fAddHeader ? 1 : 0);
	}
	
	curl_easy_setopt(fHandle,CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(fHandle,CURLOPT_WRITEDATA, this);
	curl_easy_setopt(fHandle,CURLOPT_WRITEFUNCTION, WriteGlue);
	curl_easy_setopt(fHandle,CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(fHandle,CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(fHandle,CURLOPT_PROGRESSFUNCTION, ProgressGlue);
	
	if (fAuthString.CountChars())
		curl_easy_setopt(fHandle, CURLOPT_USERPWD, fAuthString.String());
	
	fDirty = true;
	fURL = url;
	
	curl_easy_setopt(fHandle,CURLOPT_URL, fURL.String());
	
	fStatus = curl_easy_perform(fHandle);
	
	return fStatus;
}


CURLcode
CurlGetter::GetFTPListing(const char *host, const char *dir, BString &output)
{
	if (!host)
		return CURLE_URL_MALFORMAT;
	
	if (!fOutput)
		return CURLE_FAILED_INIT;
	
	if (fDirty) {
		curl_easy_reset(fHandle);
		
		// If we have to do a reset, it resets the options, so we have
		// to reinitialize the handle to use them just as before
		curl_easy_setopt(fHandle,CURLOPT_VERBOSE, fVerbose ? 1 : 0);
		curl_easy_setopt(fHandle,CURLOPT_HEADER, fAddHeader ? 1 : 0);
	}
	
	curl_easy_setopt(fHandle,CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(fHandle,CURLOPT_WRITEDATA, this);
	curl_easy_setopt(fHandle,CURLOPT_WRITEFUNCTION, WriteGlue);
	
	curl_easy_setopt(fHandle,CURLOPT_FTPLISTONLY, 1);
	
	if (fAuthString.CountChars())
		curl_easy_setopt(fHandle, CURLOPT_USERPWD, fAuthString.String());
	
	fDirty = true;
	fURL = host;
	
	curl_easy_setopt(fHandle,CURLOPT_URL, fURL.String());
	
	// The funny thing about this method is that libcurl connects and gets the 
	// listing for the current folder without changing directories. Weird. As a result,
	// we will manually change to the passed dir if we are given one
	struct curl_slist *slist = NULL;
	if (dir) {
		BString changecmd = "CWD ";
		changecmd << dir;
		slist = curl_slist_append(slist, changecmd.String());
		curl_easy_setopt(fHandle, CURLOPT_QUOTE, slist);
	}
	
	// We need to save fOutput because we are going to use a temporary object
	BDataIO *saved = fOutput;
	
	BMallocIO memio;
	fOutput = &memio;
	
	fStatus = curl_easy_perform(fHandle);
	
	output = (char *)memio.Buffer();
	fOutput = saved;
	
	if (slist)
		curl_slist_free_all(slist);
	
	return fStatus;
}


size_t
CurlGetter::WriteGlue(void *ptr, size_t size, size_t nmemb, void *stream)
{
	// This is a glue function to use with libcurl so that we have a nice interface
	// with the rest of BeOS
	CurlGetter *getter = (CurlGetter*)stream;
	return getter->Write(ptr, size * nmemb);
}


int
CurlGetter::ProgressGlue(void *ptr, double dtotal, double dcurrent, double utotal,
						  double ucurrent)
{
	CurlGetter *getter = (CurlGetter*)ptr;
	return getter->Progress(dcurrent,dtotal,ucurrent,utotal);
}


size_t
CurlGetter::Write(void *data, size_t size)
{
	// Most child classes won't need to implement this, considering the flexibility
	// that the BDataIO class provides, but some still might have a need. :)
	return fOutput->Write(data,size);
}


int
CurlGetter::Progress(double dcurrent, double dtotal, double ucurrent,
					  double utotal)
{
	// default version doesn't do anything. Returns 0 to indicate no error.
	
	// This function gets called about every second, regardless of whether there
	// is a change in progress or not.
	
	return 0;
}
