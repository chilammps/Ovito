# Try to find the Voro++ library
#  VOROPLUSPLUS_FOUND - system has Voro++ lib
#  VoroPlusPlus_INCLUDE_DIRS - the include directories needed
#  VoroPlusPlus_LIBRARIES - libraries needed

FIND_PATH(VoroPlusPlus_INCLUDE_DIR NAMES voro++.hh)
FIND_LIBRARY(VoroPlusPlus_LIBRARY NAMES voro++)

SET(VoroPlusPlus_INCLUDE_DIRS ${VoroPlusPlus_INCLUDE_DIR})
SET(VoroPlusPlus_LIBRARIES ${VoroPlusPlus_LIBRARY})

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(VoroPlusPlus DEFAULT_MSG VoroPlusPlus_LIBRARY VoroPlusPlus_INCLUDE_DIR)

MARK_AS_ADVANCED(VoroPlusPlus_INCLUDE_DIR VoroPlusPlus_LIBRARY)
