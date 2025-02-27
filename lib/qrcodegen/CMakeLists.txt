project(qrcodegen VERSION 1.8.0 LANGUAGES CXX)

add_library(qrcodegen qrcodegen.hpp qrcodegen.cpp)
target_include_directories(qrcodegen INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_directories(qrcodegen PUBLIC compiler_flags)
