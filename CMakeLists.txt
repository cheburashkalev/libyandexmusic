cmake_minimum_required(VERSION 3.5)

project(yandexmusic LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(yandexmusic SHARED
  yandexmusic_global.h
  yandexmusic.c
  yandexmusic.h
)

target_compile_definitions(yandexmusic PRIVATE YANDEXMUSIC_LIBRARY)

target_link_libraries(yandexmusic PRIVATE curl json-c)
