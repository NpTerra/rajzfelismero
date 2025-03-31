cmake_minimum_required(VERSION 3.5.0)
project(nagyhazi VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

file(GLOB_RECURSE PROJECT_SOURCES "src/*.c" "src/*.h")
file(GLOB_RECURSE TOOLS "tools/*.h")

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${TOOLS}
)

include_directories(src/headers)
include_directories(tools)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Platform is Windows")
    set(RAYLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/raylib-5.0_win64_mingw-w64)
    include_directories(${RAYLIB_DIR}/include)
    target_link_libraries(${PROJECT_NAME} ${RAYLIB_DIR}/lib/libraylibdll.a)

elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Platform is Linux")
    set(RAYLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/raylib-5.0_linux_amd64)
    include_directories(${RAYLIB_DIR}/include)
    target_link_libraries(${PROJECT_NAME} ${RAYLIB_DIR}/lib/libraylib.a m)

else()
    message(STATUS "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()
