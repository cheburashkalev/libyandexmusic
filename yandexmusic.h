#ifndef YANDEXMUSIC_H
#define YANDEXMUSIC_H

#include "yandexmusic_global.h"
#include <stdbool.h>
#include <stddef.h>
#include <cjson/cJSON.h>

struct response{
    char* data;
    size_t len;
};

struct artist{
    int id;
    char* name;
};

struct album{
    int id;
    char* name;
};

struct track{
    char* title;
    struct artist artist[25];
    struct album album[25];
    int id;
    size_t artists_amount;
    size_t albums_amount;
};

struct tracks{
    struct track item[100];
    size_t tracks_col;
};

typedef struct response response;
typedef struct tracks tracks;

tracks* yam_search(char*);
size_t writedata(void*, size_t, size_t, response*);
tracks* get_track_info(cJSON*);

#endif /* YANDEXMUSIC_H */
