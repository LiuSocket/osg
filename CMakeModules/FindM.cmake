# This module defines
#  M_FOUND, If false, do not try to use CCV.
#  M_INCLUDE_DIR, header files for libm
#  M_LIBRARY, where to find the CCV library.

FIND_PATH(M_INCLUDE_DIR math.h)
FIND_LIBRARY(M_LIBRARY NAMES m)

IF(M_INCLUDE_DIR AND M_LIBRARY)
    SET(M_FOUND "YES")
    MESSAGE(STATUS "Found libM: ${M_LIBRARY}")
ELSE(M_INCLUDE_DIR AND M_LIBRARY)
    MESSAGE(STATUS "Warning: Unable to find libM")
ENDIF(M_INCLUDE_DIR AND M_LIBRARY)