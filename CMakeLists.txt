cmake_minimum_required(VERSION 3.15)
project(chatcurse VERSION 1.0)

add_library(compiler_flags INTERFACE)
target_compile_features(compiler_flags INTERFACE cxx_std_23)
target_compile_options(compiler_flags INTERFACE -Wall -Wextra -Wpedantic)

find_package(Curses REQUIRED)
# SET(CURSES_USE_NCURSES TRUE)

file(GLOB_RECURSE SOURCE_FILES src/*.h src/*.cc)
add_executable(chatcurse ${SOURCE_FILES})
target_include_directories(chatcurse PUBLIC ${CURSES_INCLUDE_DIRS})
target_compile_options(chatcurse PUBLIC ${CURSES_CFLAGS})
target_link_libraries(chatcurse PUBLIC compiler_flags ${CURSES_LIBRARIES} panel)

message(CURSES_INCLUDE_DIRS=${CURSES_INCLUDE_DIRS})
message(CURSES_CFLAGS=${CURSES_CFLAGS})
message(CURSES_LIBRARIES=${CURSES_LIBRARIES})
