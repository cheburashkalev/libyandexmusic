#include "yandexmusic.h"
#include <curl/curl.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <stdlib.h>
#include <stddef.h>
#include "jsmn.h"

size_t writedata(void*, size_t, size_t, struct response*);
static int jsoneq(const char*, jsmntok_t*, const char*);

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
            char* search_query = calloc(query_len, sizeof(char*));
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

            jsmn_parser jsmn_resp;
            jsmntok_t tokens[1024];

            jsmn_init(&jsmn_resp);
            int r = jsmn_parse(&jsmn_resp, response.data, response.len, tokens, 1024);

            if(r < 0){
                printf("ERR:: JSON Parse error .. %d\n", r);
                goto end;
            }else if(r < 1 || tokens[0].type != JSMN_OBJECT){
                printf("ERR:: JSON Object expected .. %d\n", r);
                goto end;
            }
            uint toks;
            for(toks = 0; toks < r; toks++){
//                if(jsoneq(response.data, &tokens[toks], "title") == 0){
//                    printf("%.*s", tokens[toks + 1].end - tokens[toks + 1].start, response.data + tokens[toks + 1].start);
//                    //toks = 1250;
//                }
                if(tokens[toks].type == JSMN_OBJECT){
                    printf("%.*s", tokens[toks + 1].end - tokens[toks + 1].start, response.data + tokens[toks + 1].start);
                }
            }

            cJSON *JSON_resp = cJSON_Parse(response.data);

//            char* error_ptr;
//            if(!JSON_resp){
//                 error_ptr = (char*)cJSON_GetErrorPtr();
//                 if(error_ptr != NULL){
//                     fprintf(stderr, "Parsing error. Before: %s\n", error_ptr);
//                   }
//                 goto end;
//              }

            cJSON* result = cJSON_GetObjectItemCaseSensitive(JSON_resp, "result");
            cJSON* tracks = cJSON_GetObjectItemCaseSensitive(result, "tracks");
            cJSON* results = cJSON_GetObjectItemCaseSensitive(tracks, "results");

            tracks_info = get_track_info(results);

            if(JSON_resp)cJSON_Delete(JSON_resp);
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
        userdata->data = calloc(new_len + 1, sizeof(char*));
        memcpy(userdata->data, data, new_len);
        userdata->data[new_len] = '\0';
        userdata->len = new_len;

        return size*nmemb;
    }
    return 0;
}

/* jsmn */
static int jsoneq(const char* json, jsmntok_t* tok, const char* s){
  if(tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

static int objeq(const char* json, jsmntok_t* tok, const char* s){
    if(tok->type == JSMN_OBJECT && (int)strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
      return 0;
    }
    return -1;
}

static char* get_link(jsmntok_t* tokens, char* JSON, uint count){
    char* tmp = calloc(200, sizeof(download));
    uint j = 0;
    /* Yuobtvayoumat */
    start:
        j++;
        if(jsoneq(JSON, &tokens[j], "downloadInfoUrl") == 0){
            tmp = calloc(512, sizeof(char*));
            snprintf(tmp, 512, "%.*s", tokens[j+1].end - tokens[j+1].start, JSON + tokens[j+1].start);
            goto end;
         }else{tmp = NULL;}
    if(j < count){goto start;}
    end:
    return tmp;
}

tracks* get_track_info(cJSON* input_data){
    size_t trackItm_s = cJSON_GetArraySize(input_data);
    struct tracks* tmp = calloc(1, sizeof(tracks));
    tmp->item = calloc(trackItm_s, sizeof(struct track));
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
            tmp->item[i].album = calloc(tmp->item[i].albums_amount, sizeof(struct album));
            tmp->item[i].artist = calloc(tmp->item[i].artists_amount, sizeof(struct artist));

            if(title){
                size_t title_s = strlen(title->valuestring);
                tmp->item[i].title = calloc(title_s + 1, sizeof(char*));
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
                    tmp->item[i].album->name = calloc(album_s + 1, sizeof(char*));
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
                    tmp->item[i].artist->name = calloc(artist_s + 1, sizeof(char*));
                    memcpy(tmp->item[i].artist->name, ari_name->valuestring, artist_s);
                    tmp->item[i].artist->name[artist_s] = '\0';
                }
                if(ari_id)tmp->item[i].artist->id = ari_id->valueint;
            }
        }

    return tmp;
}

char* get_download_url(int trackId){
    response response;
    response.len = 0;
    char* url = calloc(75, sizeof(char*));;
    char* download_link = calloc(512, sizeof(char));
    CURL* curl = curl_easy_init();
        if(curl){
            snprintf(url, 75, "%s%d%s", "https://api.music.yandex.net/tracks/", trackId, "/download-info");

            curl_easy_setopt(curl, CURLOPT_URL, url);
            //curl_easy_setopt(curl, CURLOPT_USERAGENT, "libyandexmusic");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_perform(curl);

            curl_easy_cleanup(curl);

            jsmn_parser jsmn_resp;
            jsmntok_t t[70];
            jsmn_init(&jsmn_resp);
            int r = jsmn_parse(&jsmn_resp, response.data, response.len, t, 70);

            if(r < 0){
                printf("ERR:: JSON Parse error .. %d\n", r);
                goto end;
            }else if(r < 1 || t[0].type != JSMN_OBJECT){
                printf("ERR:: JSON Object expected .. %d\n", r);
                goto end;
            }

            char* link = get_link(t, response.data, r);

            if(link){
                free(url);
                free(response.data);
                response.len = 0;
                CURL* curl = curl_easy_init();
                if(curl){
                     curl_easy_setopt(curl, CURLOPT_URL, link);
                     //curl_easy_setopt(curl, CURLOPT_USERAGENT, "libyandexmusic");
                     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata);
                     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
                     curl_easy_perform(curl);


                     char* sign = calloc(128, sizeof(char));
                     char* startptr;
                     startptr = strstr(link, "?sign=") + 6;
                     snprintf(sign, 128, "%s", startptr);
                     int p = 0;
                     p = 0;
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

                     char* path = calloc(128, sizeof(char));
                     startptr = strstr(startptr, "<path>") + 6;
                     snprintf(path, 256, "%s", startptr);
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

                     snprintf(download_link, 768, "%s%s%s%s%c%s%s", "https://", host, "/get-mp3/", sign, '/', ts, path);
                     printf("%s", download_link);
                }
            }
    }

    end:
    curl_global_cleanup();
    return download_link;
}
