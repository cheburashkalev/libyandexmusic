#ifndef INSIDE_H
#define INSIDE_H

#include <stddef.h>
#include "yandexmusic.h"
#include <json-c/json.h>
#include <stdlib.h>
typedef struct response{
    char* data;
    size_t len;
}response;

typedef unsigned int uint;

size_t writedata(void*, size_t, size_t, struct response*);
tracks* get_tracks_info(struct json_object* tracks);
#endif // INSIDE_H
