#ifndef YANDEXMUSIC_H
#define YANDEXMUSIC_H

#include "yandexmusic_global.h"
#include <stdbool.h>
#include <stddef.h>
#include <json-c/json.h>
#include <stdlib.h>

typedef struct response{
    char* data;
    size_t len;
}response;

struct artist{
    unsigned int id;
    char* name;
};

struct album{
    unsigned int id;
    char* name;
};

struct track{
    char* title;
    struct artist* artist;
    struct album* album;
    unsigned int id;
    size_t artists_amount;
    size_t albums_amount;
};

typedef struct tracks{
    struct track* item;
    size_t tracks_col;
}tracks;

typedef struct download{
    char* codec;
    bool gain;
    bool preview;
    char* downloadInfoUrl;
    bool direct;
    unsigned int bitrateInKbps;
}download;

typedef struct userInfo{
    char* access_token;
    unsigned int expires_in;
    char* token_type;
    unsigned int uid;
}userInfo;

tracks* yam_search(char* query, userInfo* userinfo);
tracks* get_track_info(struct json_object* tracks);
char* get_download_url(unsigned int trackId, userInfo* userinfo);
userInfo* get_token(char* grant_type, char* username, char* password);

#endif /* YANDEXMUSIC_H */
