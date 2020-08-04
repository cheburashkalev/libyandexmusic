#include "yandexmusic.h"
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <json-c/json.h>

size_t writedata(void*, size_t, size_t, struct response*);

tracks* yam_search(char* query){
    response response;
    struct tracks* tracks_info = NULL;
    response.len = 0;
    response.data = NULL;

    CURL* curl = curl_easy_init();
    CURLcode code;
        if(curl){
            char* ptr = strchr(query, ' ');
            while(ptr){ *ptr = '+'; ptr = strchr(query, ' '); }

            size_t query_len = strlen(query) + 99;
            char* search_query = calloc(query_len, sizeof(char));
            snprintf(search_query, query_len, "%s%s%s", "https://api.music.yandex.net/search?text=", query, "&nocorrect=false&type=all&page=0&playlist-in-best=true");

            curl_easy_setopt(curl, CURLOPT_URL, search_query);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "android");
            curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 160000L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata);

            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "Yandex-music-client: Client");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            code = curl_easy_perform(curl);

            free(search_query);
            if(!response.data)goto end;

            int status;
            struct json_object* json_resp,* tracks,* result,* results;
            json_resp = json_tokener_parse(response.data);
            status = json_object_object_get_ex(json_resp, "result", &result);
            status = json_object_object_get_ex(result, "tracks", &tracks);
            status = json_object_object_get_ex(tracks, "results", &results);
            tracks_info = get_track_info(results);
        }

    end:
    free(response.data);
    curl_global_cleanup();
    return tracks_info;
}

/* curl */
size_t writedata(void* data, size_t size, size_t nmemb, struct response *userdata){
    if(userdata->len == 0){
        size_t new_len = size*nmemb;
        userdata->data = calloc(new_len + 1, sizeof(char));
        memcpy(userdata->data, data, new_len);
        userdata->data[new_len] = '\0';
        userdata->len = new_len;

        return size*nmemb;
    }
    return 0;
}

download* get_link(response response){
    json_object* json,* result;
    json = json_tokener_parse(response.data);
    result = json_object_object_get(json, "result");
    size_t arr_size = json_object_array_length(result);
    download* tmp = calloc(arr_size, sizeof(download));
    uint i;
    for(i = 0; i < arr_size; i++){
        json_object* item,* codec,* gain,* preview,* downloadInfoUrl,* direct,* bitrateInKbps;
        item = json_object_array_get_idx(result, i);
        codec = json_object_object_get(item, "codec");
        gain = json_object_object_get(item, "gain");
        preview = json_object_object_get(item, "preview");
        downloadInfoUrl = json_object_object_get(item, "downloadInfoUrl");
        direct = json_object_object_get(item, "direct");
        bitrateInKbps = json_object_object_get(item, "bitrateInKbps");

        tmp[i].codec = calloc(json_object_get_string_len(codec), sizeof(char));
        tmp[i].downloadInfoUrl = calloc(json_object_get_string_len(bitrateInKbps), sizeof(char));

        tmp[i].codec = (char*)json_object_get_string(codec);
        tmp[i].gain = json_object_get_boolean(gain);
        tmp[i].preview = json_object_get_boolean(preview);
        tmp[i].downloadInfoUrl = (char*)json_object_get_string(downloadInfoUrl);
        tmp[i].direct = json_object_get_boolean(direct);
        tmp[i].bitrateInKbps = json_object_get_int(bitrateInKbps);
    }
    free(json);
    return tmp;
}

tracks* get_track_info(json_object* input_info){
    size_t trackItm_s = json_object_array_length(input_info);
    struct tracks* tmp = malloc(sizeof(tracks));
    tmp->item = calloc(trackItm_s, sizeof(struct track));
    tmp->tracks_col = trackItm_s;

    uint i, k, j;
    for(i = 0; i < tmp->tracks_col; i++){
        json_object* item,* title,* id,* albums,* artists;

        item = json_object_array_get_idx(input_info, i);
        title = json_object_object_get(item, "title");
        id = json_object_object_get(item, "id");

        albums = json_object_object_get(item, "albums");
        artists = json_object_object_get(item, "artists");

        tmp->item[i].albums_amount = json_object_array_length(albums);
        tmp->item[i].artists_amount = json_object_array_length(artists);
        tmp->item[i].album = calloc(tmp->item[i].albums_amount, sizeof(struct album));
        tmp->item[i].artist = calloc(tmp->item[i].artists_amount, sizeof(struct artist));

        if(title){
            size_t title_s = json_object_get_string_len(title);
            tmp->item[i].title = calloc(title_s + 1, sizeof(char));
            tmp->item[i].title = (char*)json_object_get_string(title);
        }
        tmp->item[i].id = json_object_get_int(id);

        for(k = 0; k < tmp->item[i].albums_amount; k++){
            json_object* album_item,* ali_name,* ali_id;
            album_item = json_object_array_get_idx(albums, k);
            ali_name = json_object_object_get(album_item, "title");
            ali_id = json_object_object_get(album_item, "id");
            if(ali_name){
                size_t album_s = json_object_get_string_len(ali_name);
                tmp->item[i].album->name = calloc(album_s + 1, sizeof(char));
                tmp->item[i].album->name = (char*)json_object_get_string(ali_name);
            }
            if(ali_id)tmp->item[i].album->id = json_object_get_int(ali_id);
        }

        for(j = 0; j < tmp->item[i].artists_amount; j++){
            json_object* artist_item,* ari_name,* ari_id;
            artist_item = json_object_array_get_idx(artists, j);
            ari_name = json_object_object_get(artist_item, "name");
            ari_id = json_object_object_get(artist_item, "id");
            if(ari_name){
                size_t artist_s = json_object_get_string_len(ari_name);
                tmp->item[i].artist->name = calloc(artist_s + 1, sizeof(char));
                tmp->item[i].artist->name = (char*)json_object_get_string(ari_name);
            }
            if(ari_id)tmp->item[i].artist->id = json_object_get_int(ari_id);
        }
    }
    return tmp;
}

char* get_download_url(int trackId){
    response response;
    response.len = 0;
    char* url = calloc(75, sizeof(char));
    char* download_link = NULL;
    CURL* curl = curl_easy_init();
        if(curl){
            snprintf(url, 75, "%s%d%s", "https://api.music.yandex.net/tracks/", trackId, "/download-info");

            curl_easy_setopt(curl, CURLOPT_URL, url);
            //curl_easy_setopt(curl, CURLOPT_USERAGENT, "libyandexmusic");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_perform(curl);

            curl_easy_cleanup(curl);

            download* download_info = get_link(response);

            if(download_info[0].downloadInfoUrl){
                free(url);
                free(response.data);
                response.len = 0;
                CURL* curl2 = curl_easy_init();
                if(curl2){
                     curl_easy_setopt(curl2, CURLOPT_URL, download_info[0].downloadInfoUrl);
                     //curl_easy_setopt(curl, CURLOPT_USERAGENT, "libyandexmusic");
                     curl_easy_setopt(curl2, CURLOPT_WRITEFUNCTION, writedata);
                     curl_easy_setopt(curl2, CURLOPT_WRITEDATA, &response);
                     curl_easy_perform(curl2);


                     char* sign = calloc(128, sizeof(char));
                     char* startptr;
                     startptr = strstr(download_info[0].downloadInfoUrl, "?sign=") + 6;
                     snprintf(sign, 128, "%s", startptr);
                     int p = 0;
                     while(sign[p] != '&'){
                         p++;
                     }
                     sign[p] = '\0';

                     char* host = calloc(40, sizeof(char));
                     startptr = strstr(response.data, "<host>") + 6;
                     snprintf(host, 40, "%s", startptr);
                     p = 0;
                     while(host[p] != '<'){
                         p++;
                     }
                     host[p] = '\0';

                     char* path = calloc(512, sizeof(char));
                     startptr = strstr(startptr, "<path>") + 6;
                     snprintf(path, 512, "%s", startptr);
                     p = 0;
                     while(path[p] != '<'){
                         p++;
                     }
                     path[p] = '\0';

                     char* ts = calloc(24, sizeof(char));
                     startptr = strstr(startptr, "<ts>") + 4;
                     snprintf(ts, 24, "%s", startptr);
                     p = 0;
                     while(ts[p] != '<'){
                         p++;
                     }
                     ts[p] = '\0';

                     size_t link_s = 40 + strlen(host) + strlen(sign) + strlen(ts) + strlen(path);
                     download_link = calloc(link_s, sizeof(char));
                     snprintf(download_link, link_s, "%s%s%s%s%c%s%s%c", "https://", host, "/get-mp3/", sign, '/', ts, path, '\0');

                     end:
                     curl_easy_cleanup(curl2);
                     curl_global_cleanup();
                     return download_link;
                }else{goto end;}
            }else{goto end;}
    }
    goto end;
}
