INCLUDE_DIRECTORIES(${SXPLAYER_INCLUDE_DIRS} ${COREVIDEO_INCLUDE_DIR})
LINK_DIRECTORIES(${SXPLAYER_LIBRARY_DIRS})
LINK_LIBRARIES(${SXPLAYER_LIBRARIES} ${COREVIDEO_LIBRARY})

INCLUDE_DIRECTORIES(${DEP_DIR}/include)


IF(APPLE)
    IF (OSG_BUILD_PLATFORM_IPHONE OR OSG_BUILD_PLATFORM_IPHONE_SIMULATOR)
        ADD_DEFINITIONS(-DREADER_PLATFORM_IOS)
        SET(PLATFORM_SRC IOSFFmpegImageStream.mm)
        SET(PLATFORM_HEADER IOSFFmpegImageStream.hpp)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc -fvisibility-inlines-hidden -x objective-c++")
    ENDIF()
ENDIF()

SET(TARGET_SRC
    FFmpegImageStream.cpp
    ReaderWriterFFmpeg.cpp
    ${PLATFORM_SRC}
)

SET(TARGET_H
    FFmpegImageStream.hpp
    FFmpegParameters.hpp
    ${PLATFORM_HEADER}
)

SETUP_PLUGIN(sffmpeg sffmpeg)

#
# Ugly hack because CMake sucks
#
# The libsxplayer linker flags extracted from pkg-config/Libs entry contains the
# "-lfoo -lbar ..." flags as well as the "-framework Abc -framework ..." from
# Apple (used for hwaccel) necessary when linking with it as a static library.
#
# Unfortunately, CMake does not translate them into the libraries variable,
# breaking the link phase.
#
# Instead, it put these -framework parameters into SXPLAYER_LDFLAGS_OTHER and
# leave them here. So we take them again, replace the ';' by spaces (because
# they were replaced for whatever fucking nonsense reason) and force the linker
# flags with the result.
#

IF (APPLE)
    STRING(REGEX REPLACE ";" " " FUCK_CMAKE "${SXPLAYER_LDFLAGS_OTHER}")
    set_property(TARGET osgdb_sffmpeg PROPERTY LINK_FLAGS "${FUCK_CMAKE}")
ENDIF()
