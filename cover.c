#include "yandexmusic.h"
#include "inside.h"
#include <curl/curl.h>

cover* get_cover(char* url){
    response response;
    response.len = 0;
    response.data = NULL;
    cover* coverData = NULL;
    CURL* curl = curl_easy_init();
    CURLcode res;
    if(curl){
        curl_easy_setopt(curl, CURLOPT_URL, url);
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
    curl_global_cleanup();
    return coverData;
}
