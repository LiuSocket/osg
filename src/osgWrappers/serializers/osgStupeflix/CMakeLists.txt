FILE(GLOB TARGET_SRC *.cpp)
FILE(GLOB TARGET_H *.h)

SET(TARGET_ADDED_LIBRARIES osgStupeflix )

if(PKGCONFIG_FOUND)
    pkg_check_modules(FREETYPE freetype2)
    link_directories("${FREETYPE_LIBRARY_DIRS}")
endif()

#### end var setup  ###
SETUP_PLUGIN(osgstupeflix)
