cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME offscreen)
project (${PROJECT_NAME})

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLESv2 glesv2 REQUIRED)
pkg_check_modules(EGL egl REQUIRED)

include_directories(./include)
include_directories(${GLESv2_INCLUDE_DIRS})
include_directories(${EGL_INCLUDE_DIRS})

link_directories(
    ${GLESv2_LIBRARY_DIRS}
    ${EGL_LIBRARY_DIRS}
)

file(GLOB PROJECT_SOURCES main.cpp)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

set(LIBS
    ${LIBS}
    ${GLESv2_LIBRARIES}
    ${EGL_LIBRARIES}
    -lX11
)

target_link_libraries(${PROJECT_NAME} ${LIBS})

add_definitions("-std=c++11")
