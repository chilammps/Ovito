###############################################################################
# 
#  Copyright (2013) Alexander Stukowski
#
#  This file is part of OVITO (Open Visualization Tool).
#
#  OVITO is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  OVITO is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################

######################### Setup CPack #######################

# Gather required system libraries and install them
SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP ON)
INCLUDE(InstallRequiredSystemLibraries)
IF(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS AND WIN32)
	INSTALL_PROGRAMS("${OVITO_RELATIVE_BINARY_DIRECTORY}" ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
ENDIF(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS AND WIN32)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OVITO (The Open Visualization Tool) is a scientific visualization and analysis software for atomistic simulation data.")
SET(CPACK_PACKAGE_VENDOR "Alexander Stukowski")
SET(CPACK_PACKAGE_CONTACT "Alexander Stukowski <mail@ovito.org>")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR ${OVITO_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${OVITO_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${OVITO_VERSION_REVISION})
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
EXECUTE_PROCESS(COMMAND "uname" "-m" OUTPUT_VARIABLE MACHINE_HARDWARE_NAME OUTPUT_STRIP_TRAILING_WHITESPACE)
EXECUTE_PROCESS(COMMAND "lsb_release" "-s" "-i" OUTPUT_VARIABLE LINUX_DISTRIBUTOR_NAME OUTPUT_STRIP_TRAILING_WHITESPACE)
EXECUTE_PROCESS(COMMAND "lsb_release" "-s" "-r" OUTPUT_VARIABLE LINUX_RELEASE_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
IF(LINUX_DISTRIBUTOR_NAME STREQUAL "SUSE LINUX")
	SET(LINUX_DISTRIBUTOR_NAME "OpenSUSE")
ENDIF(LINUX_DISTRIBUTOR_NAME STREQUAL "SUSE LINUX")

IF(NOT OVITO_MONOLITHIC_BUILD)
	SET(CPACK_PACKAGE_FILE_NAME "ovito-${OVITO_PROJ_VERSION_MAJOR}.${OVITO_PROJ_VERSION_MINOR}.${OVITO_PROJ_VERSION_REVISION}-${LINUX_DISTRIBUTOR_NAME}-${LINUX_RELEASE_VERSION}-${MACHINE_HARDWARE_NAME}")
	IF(WIN32 AND NOT UNIX)
	  # There is a bug in NSI that does not handle full unix paths properly. Make
	  # sure there is at least one set of four backslashes..
	  SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
	  SET(CPACK_NSIS_INSTALLED_ICON_NAME "${OVITO_RELATIVE_BINARY_DIRECTORY}\\\\ovito.exe")
	  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} OVITO")
	  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.ovito.org")
	  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.ovito.org")
	  SET(CPACK_NSIS_CONTACT "mail@ovito.org")
	  SET(CPACK_NSIS_MODIFY_PATH ON)
	ELSE(WIN32 AND NOT UNIX)
	  SET(CPACK_STRIP_FILES "${OVITO_RELATIVE_BINARY_DIRECTORY}/ovito")
	  SET(CPACK_SOURCE_STRIP_FILES "")
	  # Setup package dependencies, which are specific for each Linux distro.
	  IF(LINUX_DISTRIBUTOR_NAME STREQUAL "Ubuntu")
		SET(CPACK_GENERATOR "DEB")
		SET(CPACK_DEBIAN_PACKAGE_SECTION "science")
		IF(LINUX_RELEASE_VERSION STREQUAL "8.04")
			### Ubuntu 8.04 LTS
			SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt4-core (>= 4.4), libqt4-gui (>= 4.4), libqt4-help (>= 4.4), libqt4-network (>= 4.4), libqt4-opengl (>= 4.4), libqt4-assistant (>= 4.4), libqt4-xml (>= 4.4), qt4-dev-tools (>= 4.4), libboost-iostreams1.34.1, libboost-python1.34.1, python (>= 2.5), libqscintilla2-3, povray, povray-includes, libgsl0ldbl")
		ELSEIF(LINUX_RELEASE_VERSION STREQUAL "9.10")
			### Ubuntu 9.10
			SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libqtcore4 (>= 4.4), libqtgui4 (>= 4.4), libqt4-help (>= 4.4), libqt4-network (>= 4.4), libqt4-opengl (>= 4.4), libqt4-assistant (>= 4.4), libqt4-xml (>= 4.4), qt4-dev-tools (>= 4.4), libboost-iostreams1.38.0, povray, povray-includes, libgsl0ldbl")
		ELSEIF(LINUX_RELEASE_VERSION STREQUAL "10.04")
			### Ubuntu 10.04
			SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libqtcore4 (>= 4.4), libqtgui4 (>= 4.4), libqt4-help (>= 4.4), libqt4-network (>= 4.4), libqt4-opengl (>= 4.4), libqt4-assistant (>= 4.4), libqt4-xml (>= 4.4), qt4-dev-tools (>= 4.4), libboost-iostreams1.40.0, libboost-python1.40.0, povray, povray-includes, libgsl0ldbl")
		ELSE(LINUX_RELEASE_VERSION STREQUAL "8.04")
			### Ubuntu 10.10 and later
			SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libqtcore4 (>= 4.4), libqtgui4 (>= 4.4), libqt4-help (>= 4.4), libqt4-network (>= 4.4), libqt4-opengl (>= 4.4), libqt4-assistant (>= 4.4), libqt4-xml (>= 4.4), qt4-dev-tools (>= 4.4), libboost-iostreams1.42.0, libboost-python1.42.0, povray, povray-includes, libgsl0ldbl")
		ENDIF(LINUX_RELEASE_VERSION STREQUAL "8.04")
	  ELSEIF(LINUX_DISTRIBUTOR_NAME STREQUAL "Debian")
		SET(CPACK_GENERATOR "DEB")
		SET(CPACK_DEBIAN_PACKAGE_SECTION "science")
		SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libqtcore4 (>= 4.4), libqtgui4 (>= 4.4), libqt4-help (>= 4.4), libqt4-network (>= 4.4), libqt4-opengl (>= 4.4), libqt4-assistant (>= 4.4), libqt4-xml (>= 4.4), qt4-dev-tools (>= 4.4), libboost-iostreams1.34.1, libboost-python1.34.1, python (>= 2.5), povray, povray-includes, libqscintilla2-3, libgsl0ldbl")
	  ELSEIF(LINUX_DISTRIBUTOR_NAME STREQUAL "OpenSUSE")
		SET(CPACK_GENERATOR "RPM")
		IF(LINUX_RELEASE_VERSION STREQUAL "11.2") 
			# OpenSUSE 11.2:
			SET(CPACK_RPM_PACKAGE_REQUIRES "libqt4 >= 4.4, libqt4-x11 >= 4.4, libqt4-devel-doc >= 4.4, libboost_iostreams1_39_0, libboost_serialization1_39_0, libboost_python1_39_0, povray, python >= 2.5, libqscintilla2-5, gsl") 
		ELSE(LINUX_RELEASE_VERSION STREQUAL "11.2")	
			# OpenSUSE 11.4:
			SET(CPACK_RPM_PACKAGE_REQUIRES "libqt4 >= 4.4, libqt4-x11 >= 4.4, libqt4-devel-doc >= 4.4, libboost_iostreams1_44_0, libboost_serialization1_44_0, libboost_python1_44_0, povray, python >= 2.5, gsl") 
		ENDIF(LINUX_RELEASE_VERSION STREQUAL "11.2")
	  ELSEIF(LINUX_DISTRIBUTOR_NAME STREQUAL "Fedora")
		SET(CPACK_GENERATOR "RPM")
		SET(CPACK_RPM_PACKAGE_REQUIRES "qt >= 4.4, boost >= 1.34, python >= 2.5, qscintilla >= 2.0, gsl >= 1.0") 
	  ELSEIF(LINUX_DISTRIBUTOR_NAME STREQUAL "MandrivaLinux")
		SET(CPACK_GENERATOR "RPM")
		IF(MACHINE_HARDWARE_NAME STREQUAL "x86_64")
			SET(CPACK_RPM_PACKAGE_REQUIRES "lib64qtcore4 >= 4.6, lib64qtgui4 >= 4.6, lib64qtopengl4 >= 4.6, lib64qtxml4 >= 4.6, qt4-assistant, lib64boost_iostreams1.42.0, lib64boost_serialization1.42.0, lib64boost_python1.42.0, povray, python >= 2.5, lib64gsl0") 
		ELSE(MACHINE_HARDWARE_NAME STREQUAL "x86_64")
			SET(CPACK_RPM_PACKAGE_REQUIRES "libqtcore4 >= 4.6, libqtgui4 >= 4.6, libqtopengl4 >= 4.6, libqtnetwork4 >= 4.6, libqtxml4 >= 4.6, qt4-assistant, libboost_iostreams1.42.0, libboost_serialization1.42.0, libboost_python1.42.0, povray, python >= 2.5, libgsl0") 
		ENDIF(MACHINE_HARDWARE_NAME STREQUAL "x86_64")
	  ELSE(LINUX_DISTRIBUTOR_NAME STREQUAL "Ubuntu")
		#MESSAGE(WARNING "Unknown Linux distribution: ${LINUX_DISTRIBUTOR_NAME}. Creation of a repository package with cpack will not work.")
	  ENDIF(LINUX_DISTRIBUTOR_NAME STREQUAL "Ubuntu")
	ENDIF(WIN32 AND NOT UNIX)
ELSE(NOT OVITO_MONOLITHIC_BUILD)
	SET(CPACK_PACKAGE_FILE_NAME "ovito-${OVITO_PROJ_VERSION_MAJOR}.${OVITO_PROJ_VERSION_MINOR}.${OVITO_PROJ_VERSION_REVISION}-${MACHINE_HARDWARE_NAME}")
	INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.txt" DESTINATION "./")
	INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt" DESTINATION "./")
	IF(OVITO_BUILD_DOCUMENTATION)
		INSTALL(PROGRAMS "${QT_BINARY_DIR}/assistant" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}/")
	ENDIF(OVITO_BUILD_DOCUMENTATION)
ENDIF(NOT OVITO_MONOLITHIC_BUILD)

SET(CPACK_PACKAGE_EXECUTABLES "${OVITO_RELATIVE_BINARY_DIRECTORY}/ovito" "OVITO (The Open Visualization Tool)")

INCLUDE(CPack)

