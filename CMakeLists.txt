#
# mupen64plus-input-gcn
#
cmake_minimum_required(VERSION 3.26)

project(mupen64plus-input-gcn)

# require pthreads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# require libusb
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

# disable lib prefix
set(CMAKE_SHARED_LIBRARY_PREFIX "")

set(MUPEN64PLUS_INPUT_GCN_SOURCES
    config.c
    gc_adapter.c
    log.c
    mapping.c
    main.c
)

add_library(mupen64plus-input-gcn SHARED ${MUPEN64PLUS_INPUT_GCN_SOURCES})

target_link_libraries(mupen64plus-input-gcn
    ${LIBUSB_LIBRARIES}
    Threads::Threads
)

target_include_directories(mupen64plus-input-gcn PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LIBUSB_INCLUDE_DIRS}
    ../mupen64plus-core/src/api
)

set_property(TARGET mupen64plus-input-gcn PROPERTY C_EXTENSIONS OFF)
set_property(TARGET mupen64plus-input-gcn PROPERTY C_STANDARD 99)
