#include "yandexmusic.h"
#include <curl/curl.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <stdlib.h>
#include <stddef.h>

tracks yam_search(char* query){
    response response;
    struct tracks tracks_info;
    response.len = 0;
    response.data = NULL;

    CURL* curl = curl_easy_init();
        if(curl){
            char* ptr = strchr(query, ' ');
            while(ptr){ *ptr = '+'; ptr = strchr(query, ' '); }

            size_t query_len = strlen(query) + 99;

            char* search_query = malloc(query_len);

            snprintf(search_query, query_len, "%s%s%s", "https://api.music.yandex.net/search?text=", query, "&nocorrect=false&type=all&page=0&playlist-in-best=true");
            printf("%s\n", search_query);

            curl_easy_setopt(curl, CURLOPT_URL, search_query);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "android");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata);
            if(curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 20378) == CURLE_UNKNOWN_OPTION){
                printf("Bufsize err");
            }

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_perform(curl);

            size_t real_size = strlen(response.data);

            if(!response.data)goto end;

            cJSON *JSON_resp = cJSON_Parse(response.data);

            char* error_ptr;
            if(!JSON_resp){
                 error_ptr = (char*)cJSON_GetErrorPtr();
                 if(error_ptr != NULL){
                     fprintf(stderr, "Error before: %s\n", error_ptr);
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
    printf("%s", tracks_info.item[0].title);
    return tracks_info;
}

size_t writedata(void* data, size_t size, size_t nmemb, struct response *userdata){
    if(userdata->len == 0){
        size_t new_len = size*nmemb;
        userdata->data = malloc(new_len + 1);
        memcpy(userdata->data, data, new_len);
        size_t real_s = strlen(data);
        userdata->data[new_len] = '\0';
        userdata->len = new_len;

        return size*nmemb+1;
    }
    return 0;
}

tracks get_track_info(cJSON* input_data){
    struct tracks tmp;
    tmp.tracks_col = cJSON_GetArraySize(input_data);

    uint i, k, j;
    for(i = 0; i < tmp.tracks_col; i++){
            cJSON* item = cJSON_GetArrayItem(input_data, i);
            cJSON* title = cJSON_GetObjectItemCaseSensitive(item, "title");
            cJSON* id = cJSON_GetObjectItemCaseSensitive(item, "id");

            cJSON* albums = cJSON_GetObjectItemCaseSensitive(item, "albums");
            cJSON* artists = cJSON_GetObjectItemCaseSensitive(item, "artists");

            tmp.item[i].albums_amount = cJSON_GetArraySize(albums);
            tmp.item[i].artists_amount = cJSON_GetArraySize(artists);

            if(title){
                //tmp.item[i].title = malloc(strlen(title->valuestring) + 1);
                tmp.item[i].title = title->valuestring;
            }
            if(id)tmp.item[i].id = id->valueint;

            for(k = 0; k < tmp.item[i].albums_amount; k++){
                cJSON* album_item = cJSON_GetArrayItem(albums, k);
                cJSON* ali_name = cJSON_GetObjectItemCaseSensitive(album_item, "title");
                cJSON* ali_id = cJSON_GetObjectItemCaseSensitive(album_item, "id");
                if(ali_name){
                    //tmp.item[i].album->name = malloc(strlen(ali_name->valuestring) + 1);
                    tmp.item[i].album->name = ali_name->valuestring;
                }
                if(ali_id)tmp.item[i].album->id = ali_id->valueint;
            }

            for(j = 0; j < tmp.item[i].artists_amount; j++){
                cJSON* artist_item = cJSON_GetArrayItem(artists, j);
                cJSON* ari_name = cJSON_GetObjectItemCaseSensitive(artist_item, "name");
                cJSON* ari_id = cJSON_GetObjectItemCaseSensitive(artist_item, "id");
                if(ari_name){
                    //tmp.item[i].artist->name = malloc(strlen(ari_name->valuestring) + 1);
                    tmp.item[i].artist->name = ari_name->valuestring;
                }
                if(ari_id)tmp.item[i].artist->id = ari_id->valueint;
            }
        }

    return tmp;
}

