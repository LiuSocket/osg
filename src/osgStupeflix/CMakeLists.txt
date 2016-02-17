# FIXME: For OS X, need flag for Framework or dylib
IF(DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGSTUPEFLIX_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSGSTUPEFLIX_LIBRARY_STATIC)
ENDIF()

INCLUDE_DIRECTORIES(${DEP_DIR}/include)

SET(LIB_NAME osgStupeflix)

SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})

SET(TARGET_H
    ${HEADER_PATH}/Animation
    ${HEADER_PATH}/ImageCleanVisitor
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/ImageColor
    ${HEADER_PATH}/ImageStream
    ${HEADER_PATH}/MarkActiveNodeVisitor
    ${HEADER_PATH}/RootUpdateCallback
    ${HEADER_PATH}/ScreenCapture
    ${HEADER_PATH}/TimeGroup
    ${HEADER_PATH}/RenderOnce
    ${HEADER_PATH}/RenderRange
    ${HEADER_PATH}/ConservativeViewer
    ${HEADER_PATH}/WindowCaptureCallback
    ${HEADER_PATH}/Bezier
    ${HEADER_PATH}/Interpolator
    ${HEADER_PATH}/Sampler
    ${HEADER_PATH}/VariableSpeedCubicBezier
    ${HEADER_PATH}/TexMat
    ${HEADER_PATH}/WidgetRemapperVisitor
    ${HEADER_PATH}/CameraHelper
    ${HEADER_PATH}/FontHelper
    ${HEADER_PATH}/Tracking
    ${HEADER_PATH}/Tracker
    ${HEADER_PATH}/TrackingMatrixUtils
    ${HEADER_PATH}/FbxAnimation
    ${HEADER_PATH}/AssetTimingsVisitor
    ${HEADER_PATH}/CaptureCallBack
    ${HEADER_PATH}/VideoBuffer
)
#    ${HEADER_PATH}/OpenCvTracker
#    ${HEADER_PATH}/OpenCvTrackerBase

IF(FBX_FOUND AND OSG_CPP_EXCEPTIONS_AVAILABLE)
    SET(FBX_SRC FbxAnimation.cpp)
ENDIF()


SET(TARGET_SRC
    Animation.cpp
    ImageCleanVisitor.cpp
    ImageColor.cpp
    ImageStream.cpp
    MarkActiveNodeVisitor.cpp
    RootUpdateCallback.cpp
    ScreenCapture.cpp
    TimeGroup.cpp
    RenderOnce.cpp
    RenderRange.cpp
    ConservativeViewer.cpp
    WindowCaptureCallback.cpp
    Bezier.cpp
    Interpolator.cpp
    Sampler.cpp
    VariableSpeedCubicBezier.cpp
    TexMat.cpp
    WidgetRemapperVisitor.cpp
    CameraHelper.cpp
    FontHelper.cpp
    AssetTimingsVisitor.cpp
    Tracking.cpp
    Tracker.cpp
    CaptureCallBack.cpp
    VideoBuffer.cpp
    ${FBX_SRC}
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)
#    OpenCvTracker.cpp
#    OpenCvTrackerBase.cpp

FIND_PACKAGE(M)
IF (ANDROID)
       ADD_DEFINITIONS(-DTRACKING_NOT_SUPPORTED=1)
           MESSAGE("[Warning] Tracking not supported. Install OpenCV 3.0.0")
ELSE()
       ADD_DEFINITIONS(-DTRACKING_NOT_SUPPORTED=1)
           MESSAGE("[Warning] Tracking not supported. Install OpenCV 3.0.0")

SET(OpenCV_FOUND 0)
#IF (OpenCV_INCLUDE_DIRS)
#   SET(OpenCV_FOUND 1)
#ELSE()
#   FIND_PACKAGE(OpenCV)
#   IF(NOT OpenCV_FOUND OR NOT M_FOUND)
#       ADD_DEFINITIONS(-DTRACKING_NOT_SUPPORTED=1)
#           MESSAGE("[Warning] Tracking not supported. Install OpenCV 3.0.0")
#   ENDIF()
#ENDIF()
ENDIF()

IF(OpenCV_FOUND AND M_FOUND)
    SET(OPENCV_FULL_LIBS ${OpenCV_LIBS} ${M_LIBRARY})
    INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})
ENDIF()

SET(TARGET_LIBRARIES
    osg
    osgViewer
    osgAnimation
    OpenThreads
    ${FONTATLAS_LIBRARIES}
)
#    ${OPENCV_FULL_LIBS}

#LINK_CORELIB_DEFAULT(${LIB_NAME})
#INCLUDE(ModuleInstall OPTIONAL)

SETUP_LIBRARY(${LIB_NAME})
