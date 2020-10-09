cmake_minimum_required(VERSION 3.5)

project(yandexmusic LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_C_FLAGS "-march=native -mtune=generic -ftree-vectorize")

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
	if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
		include_directories("/usr/local/include/")
		link_directories("/usr/local/lib/")
	endif()
    add_library(yandexmusic SHARED
      yandexmusic.c
      yandexmusic.h
      cover.c
      inside.h
    )
target_compile_definitions(yandexmusic PRIVATE YANDEXMUSIC_LIBRARY)

target_link_libraries(yandexmusic PRIVATE curl json-c)
endif()

