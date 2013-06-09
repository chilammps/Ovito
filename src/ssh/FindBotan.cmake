# - Find Botan
# Find the native Botan includes and library.
# Once done this will define
#
#  BOTAN_INCLUDE_DIRS   - where to find botan.h.
#  BOTAN_LIBRARIES      - List of libraries when using Botan.
#  BOTAN_FOUND          - True if Botan was found.

FIND_PATH(BOTAN_INCLUDE_DIR botan.h PATH_SUFFIXES botan)

FIND_LIBRARY(BOTAN_LIBRARY NAMES botan)
MARK_AS_ADVANCED(BOTAN_LIBRARY BOTAN_INCLUDE_DIR)

IF(BOTAN_INCLUDE_DIR)
  IF(BOTAN_LIBRARY)
    SET(BOTAN_FOUND "YES")
    SET(BOTAN_INCLUDE_DIRS ${BOTAN_INCLUDE_DIR})
    SET(BOTAN_LIBRARIES ${BOTAN_LIBRARY})
  ENDIF(BOTAN_LIBRARY)
ENDIF(BOTAN_INCLUDE_DIR)

