/* FSproxy Agent v1.0 RC2
 * Copyright (C) 2009  Micha³ Siejak
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "fsproxy.h"

#define CURL_STATICLIB
#include "curl/curl.h"

size_t CurlWrite(void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t totalSize = size * nmemb;
	if(totalSize > 255) totalSize = 255;
	memcpy_s(stream, 256, ptr, totalSize);
	((char*)stream)[totalSize] = 0;
	return totalSize;
}

int SendCtlLoop(const char *host, const char *ctl, const size_t timeout, const int repeat)
{
	int i, retcode;
	for(i=0; i<repeat; i++)
	{
		retcode = SendCtl(host, ctl, timeout);
		if(retcode == 0)
			return retcode;
	}
	return retcode;
}

int SendCtl(const char *host, const char *ctl, const size_t timeout)
{
	static BOOL curlInit = FALSE;
	CURL *handle;
	char url[256];
	char buffer[256];
	int retcode;

	if(!curlInit)
	{
		curl_global_init(CURL_GLOBAL_WIN32);
		curlInit = TRUE;
	}

	sprintf_s(url, 256, "http://%s/ctl.cgi?%s", host, ctl);
	handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, url);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, CurlWrite);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, buffer);
	if(timeout > 0)
		curl_easy_setopt(handle, CURLOPT_TIMEOUT, timeout);

	if(curl_easy_perform(handle) != 0)
	{
		curl_easy_cleanup(handle);
		return -1;
	}
	curl_easy_cleanup(handle);

	if(sscanf_s(buffer, "%d", &retcode) != 1)
		return -1;
	return retcode;
}