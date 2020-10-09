/*
	libyandexmusic
    Copyright (C) 2020  Steftim

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
    USA 
*/

#include "yandexmusic.h"
#include "inside.h"
#include <curl/curl.h>

cover* get_cover(char* url, char* proxy, char* proxy_type){
    response response;
    response.len = 0;
    response.data = NULL;
    cover* coverData = NULL;
    CURL* curl = curl_easy_init();
    CURLcode res;
    if(curl){
        curl_easy_setopt(curl, CURLOPT_URL, url);
        if(proxy != NULL){
            curl_easy_setopt(curl, CURLOPT_PROXYTYPE, proxy_type);
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy);
        }
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
#ifdef _WIN32
        curl_easy_setopt(curl, CURLOPT_CAINFO, "crt\\cacert.pem");
        curl_easy_setopt(curl, CURLOPT_CAPATH, "crt\\cacert.pem");
#endif
#ifdef DEBUG
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 2);
#endif
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Windows 10");
        curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, CURL_MAX_READ_SIZE);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "X-Yandex-Music-Client: WindowsPhone/4.20");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s, at get_cover()\n", curl_easy_strerror(res));
        }
        if(response.data != NULL){
            coverData = calloc(1, sizeof(cover));
            coverData->data = calloc(response.len, sizeof(char));
            coverData->data = response.data;
            coverData->len = response.len;
        }
    }
    curl_easy_cleanup(curl);
    return coverData;
}
