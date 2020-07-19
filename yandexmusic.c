#include "yandexmusic.h"
#include <curl/curl.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <stdlib.h>
#include <stddef.h>

tracks* yam_search(char* query){
    response response;
    struct tracks* tracks_info;
    response.len = 0;
    response.data = NULL;

    CURL* curl = curl_easy_init();
        if(curl){
            char* ptr = strchr(query, ' ');
            while(ptr){ *ptr = '+'; ptr = strchr(query, ' '); }

            size_t query_len = strlen(query) + 99;

            char* search_query = malloc(query_len);

            snprintf(search_query, query_len, "%s%s%s", "https://api.music.yandex.net/search?text=", query, "&nocorrect=false&type=all&page=0&playlist-in-best=true");

            curl_easy_setopt(curl, CURLOPT_URL, search_query);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "android");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_perform(curl);

            if(!response.data)goto end;

            cJSON *JSON_resp = cJSON_Parse(response.data);

            char* error_ptr;
            if(!JSON_resp){
                 error_ptr = (char*)cJSON_GetErrorPtr();
                 if(error_ptr != NULL){
                     fprintf(stderr, "Parsing error. Before: %s\n", error_ptr);
                   }
                 goto end;
              }

            cJSON* result = cJSON_GetObjectItemCaseSensitive(JSON_resp, "result");
            cJSON* tracks = cJSON_GetObjectItemCaseSensitive(result, "tracks");
            cJSON* results = cJSON_GetObjectItemCaseSensitive(tracks, "results");

            tracks_info = get_track_info(results);


            if(JSON_resp)cJSON_Delete(JSON_resp);
        }

    end:
    curl_global_cleanup();
    return tracks_info;
}

size_t writedata(void* data, size_t size, size_t nmemb, struct response *userdata){
    if(userdata->len == 0){
        size_t new_len = size*nmemb;
        userdata->data = malloc(new_len + 1);
        memcpy(userdata->data, data, new_len);
        userdata->data[new_len] = '\0';
        userdata->len = new_len;

        return size*nmemb;
    }
    return 0;
}

tracks* get_track_info(cJSON* input_data){
    size_t trackItm_s = cJSON_GetArraySize(input_data);
    struct tracks* tmp = (tracks*)malloc(sizeof(tracks));
    tmp->item = (struct track*)malloc(sizeof(struct track) * trackItm_s);
    tmp->tracks_col = trackItm_s;

    uint i, k, j;
    for(i = 0; i < tmp->tracks_col; i++){
            cJSON* item = cJSON_GetArrayItem(input_data, i);
            cJSON* title = cJSON_GetObjectItemCaseSensitive(item, "title");
            cJSON* id = cJSON_GetObjectItemCaseSensitive(item, "id");

            cJSON* albums = cJSON_GetObjectItemCaseSensitive(item, "albums");
            cJSON* artists = cJSON_GetObjectItemCaseSensitive(item, "artists");

            tmp->item[i].albums_amount = cJSON_GetArraySize(albums);
            tmp->item[i].artists_amount = cJSON_GetArraySize(artists);
            tmp->item[i].album = (struct album*)malloc(sizeof(struct album) * tmp->item[i].albums_amount);
            tmp->item[i].artist = (struct artist*)malloc(sizeof(struct artist) * tmp->item[i].artists_amount);

            if(title){
                size_t title_s = strlen(title->valuestring);
                tmp->item[i].title = malloc(title_s + 1);
                memcpy(tmp->item[i].title, title->valuestring, title_s);
                tmp->item[i].title[title_s] = '\0';
            }
            if(id)tmp->item[i].id = id->valueint;

            for(k = 0; k < tmp->item[i].albums_amount; k++){
                cJSON* album_item = cJSON_GetArrayItem(albums, k);
                cJSON* ali_name = cJSON_GetObjectItemCaseSensitive(album_item, "title");
                cJSON* ali_id = cJSON_GetObjectItemCaseSensitive(album_item, "id");
                if(ali_name){
                    size_t album_s = strlen(ali_name->valuestring);
                    tmp->item[i].album->name = malloc(album_s + 1);
                    memcpy(tmp->item[i].album->name, ali_name->valuestring, album_s);
                    tmp->item[i].album->name[album_s] = '\0';
                }
                if(ali_id)tmp->item[i].album->id = ali_id->valueint;
            }

            for(j = 0; j < tmp->item[i].artists_amount; j++){
                cJSON* artist_item = cJSON_GetArrayItem(artists, j);
                cJSON* ari_name = cJSON_GetObjectItemCaseSensitive(artist_item, "name");
                cJSON* ari_id = cJSON_GetObjectItemCaseSensitive(artist_item, "id");
                if(ari_name){
                    size_t artist_s = strlen(ari_name->valuestring);
                    tmp->item[i].artist->name = malloc(artist_s + 1);
                    memcpy(tmp->item[i].artist->name, ari_name->valuestring, artist_s);
                    tmp->item[i].artist->name[artist_s] = '\0';
                }
                if(ari_id)tmp->item[i].artist->id = ari_id->valueint;
            }
        }

    return tmp;
}

void get_download_url(int trackId, char* codec, int bitrate){
    response response;
    response.len = 0;
    CURL* curl = curl_easy_init();
        if(curl){
            char* url = malloc(75);
            snprintf(url, 75, "%s%d%s", "https://api.music.yandex.net/tracks/", trackId, "/download-info");

            curl_easy_setopt(curl, CURLOPT_URL, url);
            //curl_easy_setopt(curl, CURLOPT_USERAGENT, "libyandexmusic");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_perform(curl);

            cJSON* cJSON_resp = cJSON_Parse(response.data);
            cJSON* codec;
            if(cJSON_resp){
                codec = cJSON_GetObjectItemCaseSensitive(cJSON_resp->child->next->child, "codec");
            }
            printf("dd");

        }
}
