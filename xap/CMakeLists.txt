cmake_minimum_required(VERSION 2.8)

project(sasl-xap)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.h)

include_directories(${SASL_INCL_DIRS} 
                    ${SASL_3RD_DIR}/include/luajit-2.0
                    ../libavionics
                    ../libaccgl
                    ../alsound
				    ${SASL_3RD_DIR}/include 
					${SASL_SDK_DIR}/CHeaders/Wrappers
					${SASL_SDK_DIR}/CHeaders/Widgets
					${SASL_SDK_DIR}/CHeaders/XPLM
					${SASL_SDK_DIR}/CHeaders
                    )
link_directories(${SASL_LINK_DIRS}	${SASL_3RD_DIR}/lib/${SASL_OS}/64)

add_library(sasl-xap SHARED ${SOURCES} ${HEADERS})

target_link_libraries(sasl-xap sasl-alsound sasl-accgl sasl-avionics ${SASL_LINK_LIBS} )

set_target_properties(sasl-xap PROPERTIES OUTPUT_NAME ${SASL_OS})
set_target_properties(sasl-xap PROPERTIES PREFIX "")
set_target_properties(sasl-xap PROPERTIES SUFFIX ".xpl")
set_target_properties(sasl-xap PROPERTIES LINK_FLAGS "${SASL_LINK_FLAGS}")
set_target_properties(sasl-xap PROPERTIES LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../misc/linkscript.${SASL_OS}")

install(TARGETS sasl-xap DESTINATION sasl/64)

