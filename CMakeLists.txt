cmake_minimum_required(VERSION 2.8)

set(CMAKE_VERBOSE_MAKEFILE 1)

include (cmake.config)

set(SASL_OS)
set(SASL_CXX_FLAGS)
set(SASL_LINK_DIRS)
set(SASL_INCL_DIRS)
set(SASL_LINK_LIBS)
set(SASL_LINK_FLAGS)


if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
#
#   Linux
#
	set(SASL_OS "lin")
	set(SASL_CXX_FLAGS "${SASL_CXX_FLAGS} -DLIN=1 -DXPLM200 -DXPLM210 -DNDEBUG=1 -DSNAPSHOT=${SASL_SNAPSHOT} -Wall -fPIC -fno-stack-protector")
	set(SASL_INCL_DIRS "${SASL_INCL_DIRS}" "/usr/include/SOIL")
	set(SASL_LINK_LIBS luajit /usr/lib/libSOIL.a)


elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#
#   MacOS
#
    set(SASL_OS "mac")
	set(SASL_CXX_FLAGS "${SASL_CXX_FLAGS} -D__APPLE__ -DAPL=1 -DSNAPSHOT=${SASL_SNAPSHOT}")
	set(SASL_CXX_FLAGS "${SASL_CXX_FLAGS} -fno-stack-protector -DXPLM200 -DXPLM210 -fvisibility=hidden -ffast-math -fno-rtti -fno-exceptions")
    set(SASL_CXX_FLAGS "${SASL_CXX_FLAGS} -arch x86_64 -mmacosx-version-min=10.7")

	set(SASL_INCL_DIRS "/opt/local/include/SOIL")

	set(SASL_LINK_DIRS "${SASL_SDK_DIR}/Libraries/Mac" /opt/local/lib)
	set(SASL_LINK_LIBS luajit  /opt/local/lib/libSOIL.a "-F${SASL_SDK_DIR}/Libraries/Mac  -framework XPWidgets -framework XPLM")
	set(SASL_LINK_LIBS "${SASL_LINK_LIBS} -framework OpenGL -framework OpenAL -framework CoreFoundation -framework IOKit")
	set(SASL_LINK_LIBS "${SASL_LINK_LIBS} -arch x86_64 -mmacosx-version-min=10.7")


elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
#
#   Windows
#
	set(SASL_OS "win")
	set(SASL_CXX_FLAGS "${SASL_CXX_FLAGS} /DIBM=1 /DXPLM200  /DXPLM210 /DSNAPSHOT=${SASL_SNAPSHOT}")
	set(SASL_CXX_FLAGS "${SASL_CXX_FLAGS} /DNDEBUG /EHsc /DWINDOWS /nologo /MT /O2 /GL /DNDEBUG /D_LIB /fp:precise /TP /DWIN32")
	set(SASL_INCL_DIRS "${SASL_INCL_DIRS};${SASL_OPENAL}/include;${SASL_3RD_DIR}/include/SOIL")
	
	set(SASL_LINK_FLAGS "/nologo /LTCG")
	set(SASL_LINK_DIRS "${SASL_SDK_DIR}/Libraries/Win;${SASL_OPENAL}/libs/Win64")
	set(SASL_LINK_LIBS lua51 SOIL opengl32 OpenAL32 XPLM_64 XPWidgets_64 ws2_32)


endif()

set(CMAKE_CXX_FLAGS "${SASL_CXX_FLAGS}")

add_subdirectory(alsound)
add_subdirectory(libaccgl)
add_subdirectory(libavionics)
add_subdirectory(xap)

