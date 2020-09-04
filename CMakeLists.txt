cmake_minimum_required(VERSION 3.5)

project(yandexmusic LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_C_FLAGS "-march=native -mtune=generic -ftree-vectorize")

add_library(yandexmusic SHARED
  yandexmusic.c
  yandexmusic.h
  cover.c
  inside.h
)

target_compile_definitions(yandexmusic PRIVATE YANDEXMUSIC_LIBRARY)

target_link_libraries(yandexmusic PRIVATE curl json-c)
