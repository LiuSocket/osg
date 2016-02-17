if(PKGCONFIG_FOUND)
    pkg_check_modules(FREETYPE freetype2)
    link_directories("${FREETYPE_LIBRARY_DIRS}")
endif()

SET(TARGET_SRC osgff.cpp)
SET(TARGET_ADDED_LIBRARIES osgStupeflix)
SETUP_EXAMPLE(osgff)


