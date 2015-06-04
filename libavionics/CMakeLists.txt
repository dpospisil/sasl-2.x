cmake_minimum_required(VERSION 2.8)

project(sasl-avionics)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.h)

include_directories(${SASL_INCL_DIRS} ${SASL_3RD_DIR}/include/luajit-2.0
                                    ${SASL_3RD_DIR}/include 
                                        ${SASL_SDK_DIR}/CHeaders/Wrappers
                                        ${SASL_SDK_DIR}/CHeaders/Widgets
                                        ${SASL_SDK_DIR}/CHeaders/XPLM
                                        ${SASL_SDK_DIR}/CHeaders
)

add_library(sasl-avionics STATIC ${SOURCES})
