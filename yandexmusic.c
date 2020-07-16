#include "yandexmusic.h"
#include <curl/curl.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <stdlib.h>

void yandex_search(char* query){
    //char* resstr;
    struct response response;
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
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.42.0");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_perform(curl);

            if(!response.data)goto end;

            cJSON *JSON_resp = cJSON_Parse(response.data);

            cJSON* result = cJSON_GetObjectItemCaseSensitive(JSON_resp, "result");
            cJSON* tracks = cJSON_GetObjectItemCaseSensitive(result, "tracks");
            cJSON* results = cJSON_GetObjectItemCaseSensitive(tracks, "results");

            struct tracks tracks_info;
            tracks_info.tracks_col = cJSON_GetArraySize(results);

            get_track_info(results, &tracks_info);


            //cJSON_Delete(results);

            //cJSON* track0_title = cJSON_GetObjectItemCaseSensitive(tracks_array[0], "title");
            //cJSON* track0_id = cJSON_GetObjectItemCaseSensitive(tracks_array[0], "title");

            //if(track0_title != NULL) printf("%s", track0_title->valuestring);

            cJSON_Delete(JSON_resp);
            //cJSON_Delete(result);
            //cJSON_Delete(tracks);
            curl_easy_cleanup(curl);
        }

    end:

    curl_global_cleanup();
    return;
}

static size_t writedata(void* data, size_t size, size_t nmemb, struct response *userdata){
    size_t new_len = size*nmemb;
    userdata->data = malloc(new_len + 1);
    memcpy(userdata->data + userdata->len, data, new_len);
    userdata->data[new_len] = '\0';
    userdata->len = new_len;
    printf("%s", data);

    return size*nmemb;
}

static void get_track_info(cJSON* input_data, tracks* output_data){
    uint i, k, j;
    for(i = 0; i < output_data->tracks_col; i++){
            cJSON* item = cJSON_GetArrayItem(input_data, i);
            cJSON* title = cJSON_GetObjectItemCaseSensitive(item, "title");
            cJSON* id = cJSON_GetObjectItemCaseSensitive(item, "id");

            cJSON* albums = cJSON_GetObjectItemCaseSensitive(item, "albums");
            cJSON* artists = cJSON_GetObjectItemCaseSensitive(item, "artists");

            output_data->item[i].albums_amount = cJSON_GetArraySize(albums);
            output_data->item[i].artists_amount = cJSON_GetArraySize(artists);

            if(title)output_data->item[i].title = title->valuestring;
            if(id)output_data->item[i].id = id->valueint;

            for(k = 0; k < output_data->item[i].albums_amount; k++){
                cJSON* album_item = cJSON_GetArrayItem(albums, k);
                cJSON* ali_name = cJSON_GetObjectItemCaseSensitive(album_item, "title");
                cJSON* ali_id = cJSON_GetObjectItemCaseSensitive(album_item, "id");
                if(ali_name)output_data->item[i].album->name = ali_name->valuestring;
                if(ali_id)output_data->item[i].album->id = ali_id->valueint;
//                cJSON_Delete(ali_name);
//                cJSON_Delete(ali_id);
//                cJSON_Delete(album_item);
            }

            for(j = 0; j < output_data->item[i].artists_amount; j++){
                cJSON* artist_item = cJSON_GetArrayItem(artists, k);
                cJSON* ari_name = cJSON_GetObjectItemCaseSensitive(artist_item, "name");
                cJSON* ari_id = cJSON_GetObjectItemCaseSensitive(artist_item, "id");
                if(ari_name)output_data->item[i].artist->name = ari_name->valuestring;
                if(ari_id)output_data->item[i].artist->id = ari_id->valueint;
//                cJSON_Delete(ari_name);
//                cJSON_Delete(ari_id);
//                cJSON_Delete(artist_item);
            }

//            cJSON_Delete(item);
//            cJSON_Delete(title);
//            cJSON_Delete(id);
//            cJSON_Delete(albums);
//            cJSON_Delete(artists);

        }

    return;
}

