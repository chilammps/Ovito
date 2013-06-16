
# Compiles a .qrc file to a .rcc file using the Qt Resource Compiler

MACRO (QT_EXTRACT_OPTIONS _qt_files _qt_options)
	SET(${_qt_files})
	SET(${_qt_options})
	SET(_QT_DOING_OPTIONS FALSE)
	FOREACH(_currentArg ${ARGN})
	  IF ("${_currentArg}" STREQUAL "OPTIONS")
	    SET(_QT_DOING_OPTIONS TRUE)
	  ELSE ("${_currentArg}" STREQUAL "OPTIONS")
	    IF(_QT_DOING_OPTIONS) 
	      LIST(APPEND ${_qt_options} "${_currentArg}")
	    ELSE(_QT_DOING_OPTIONS)
	      LIST(APPEND ${_qt_files} "${_currentArg}")
	    ENDIF(_QT_DOING_OPTIONS)
	  ENDIF ("${_currentArg}" STREQUAL "OPTIONS")
	ENDFOREACH(_currentArg) 
ENDMACRO(QT_EXTRACT_OPTIONS)

MACRO (QT_COMPILE_RESOURCES target outfile)
    QT_EXTRACT_OPTIONS(rcc_files rcc_options ${ARGN})

    SET(_RC_DEPENDS ${rcc_files})
    FOREACH (it ${rcc_files})
      GET_FILENAME_COMPONENT(outfilename ${it} NAME_WE)
      GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
      GET_FILENAME_COMPONENT(rc_path ${infile} PATH)
      #  parse file for dependencies 
      #  all files are absolute paths or relative to the location of the qrc file
      FILE(READ "${infile}" _RC_FILE_CONTENTS)
      STRING(REGEX MATCHALL "<file[^<]+" _RC_FILES "${_RC_FILE_CONTENTS}")
      FOREACH(_RC_FILE ${_RC_FILES})
        STRING(REGEX REPLACE "^<file[^>]*>" "" _RC_FILE "${_RC_FILE}")
        STRING(REGEX MATCH "^/|([A-Za-z]:/)" _ABS_PATH_INDICATOR "${_RC_FILE}")
        IF(NOT _ABS_PATH_INDICATOR)
          SET(_RC_FILE "${rc_path}/${_RC_FILE}")
        ENDIF(NOT _ABS_PATH_INDICATOR)
        SET(_RC_DEPENDS ${_RC_DEPENDS} "${_RC_FILE}")
      ENDFOREACH(_RC_FILE)
    ENDFOREACH (it)

	ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
	    COMMAND ${Qt5Core_RCC_EXECUTABLE}
	    ARGS -binary -o ${outfile} ${rcc_files}
	    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	    DEPENDS ${_RC_DEPENDS}
	    COMMENT "Compiling resource file(s) ${rcc_files}")

	ADD_CUSTOM_TARGET("${target}_Resources" DEPENDS ${outfile})
	ADD_DEPENDENCIES(${target} "${target}_Resources")

ENDMACRO(QT_COMPILE_RESOURCES)

