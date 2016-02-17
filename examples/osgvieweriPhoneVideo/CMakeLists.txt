SET(TARGET_SRC 
    iphoneViewerAppDelegate.h 
    iphoneViewerAppDelegate.mm
    main.m
    osgPlugins.h
    osgIPhoneViewer-Info.plist
)

SET(TARGET_ADDED_LIBRARIES osgdb_osg osgdb_freetype osgdb_imageio osgdb_iosvideo)

#backup setting
SET(TMP_OSG_BUILD_APPLICATION_BUNDLES {$OSG_BUILD_APPLICATION_BUNDLES})
SET(OSG_BUILD_APPLICATION_BUNDLES TRUE)
SETUP_EXAMPLE(osgViewerIPhoneVideo)
#restore setting
SET(OSG_BUILD_APPLICATION_BUNDLES {$TMP_OSG_BUILD_APPLICATION_BUNDLES})
SET(CMAKE_EXE_LINKER_FLAGS "-framework CoreGraphics -framework AVFoundation -framework QuartzCore -framework Foundation -framework OpenGLES  -framework UIKit")
