#ifndef YANDEXMUSIC_H
#define YANDEXMUSIC_H

#include "yandexmusic_global.h"
#include <stdbool.h>
#include <stddef.h>
#include "jsmn.h"
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
    char* gain;
    char* preview;
    char* downloadInfoUrl;
    char* direct;
    unsigned int bitrateInKbps;
}download;

tracks* yam_search(char* query);
tracks* get_track_info(jsmntok_t* tokens, response response, uint tokenCount);
char* get_download_url(int trackId);

#endif /* YANDEXMUSIC_H */
