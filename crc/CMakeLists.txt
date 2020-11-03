cmake_minimum_required(VERSION 3.5)

set(CMAKE_BUILD_TYPE Release)
set(TARGET fdm32vs10x)
include("$ENV{PHNX_SDF}/tools/cmake/project.cmake")

file(GLOB PROJECT_SRCS "*.c")

project(app)
