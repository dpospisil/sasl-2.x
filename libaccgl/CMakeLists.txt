cmake_minimum_required(VERSION 2.8)

project(sasl-accgl)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.h)

include_directories(${SASL_INCL_DIRS})

add_library(sasl-accgl STATIC ${SOURCES})
