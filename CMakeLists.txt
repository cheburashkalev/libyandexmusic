cmake_minimum_required(VERSION 3.5)

project(yandexmusic LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(WIN32)
add_library(yandexmusic SHARED
  yandexmusic.c
  yandexmusic.h
  cover.c
  inside.h
)
target_compile_definitions(yandexmusic PRIVATE YANDEXMUSIC_LIBRARY)

target_link_libraries(yandexmusic PRIVATE ${CMAKE_SOURCE_DIR}/libcurl.dll ${CMAKE_SOURCE_DIR}/libjson-c.a)
endif()

if(UNIX)
    add_library(yandexmusic SHARED
      yandexmusic.c
      yandexmusic.h
      cover.c
      inside.h
    )
target_compile_definitions(yandexmusic PRIVATE YANDEXMUSIC_LIBRARY)

target_link_libraries(yandexmusic PRIVATE curl json-c)
endif()

