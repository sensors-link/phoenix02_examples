cmake_minimum_required(VERSION 3.5)

set(CMAKE_BUILD_TYPE Release)
set(TARGET fdm32vs10x)
include("$ENV{PHNX_SDF}/tools/cmake/project.cmake")

file(GLOB PROJECT_SRCS "src/*.c")
set(PROJECT_REQUIRES "freertos")

set(PROJECT_DEFINITIONS "USE_STDPERIPH_DRIVER" "HXTAL_VALUE=8000000")

project(app)
