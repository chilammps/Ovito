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

# Build library
ADD_LIBRARY(QSsh ${OVITO_DEFAULT_LIBRARY_TYPE}
	sftpchannel.cpp
	sftpdefs.cpp
	sftpfilesystemmodel.cpp
	sftpincomingpacket.cpp
	sftpoperation.cpp
	sftpoutgoingpacket.cpp
	sftppacket.cpp
	sshcapabilities.cpp
	sshchannel.cpp
	sshchannelmanager.cpp
	sshconnection.cpp
	sshconnectionmanager.cpp
	sshcryptofacility.cpp
	sshincomingpacket.cpp
	sshkeyexchange.cpp
	sshkeygenerator.cpp
	sshkeypasswordretriever.cpp
	sshoutgoingpacket.cpp
	sshpacket.cpp
	sshpacketparser.cpp
	sshremoteprocess.cpp
	sshremoteprocessrunner.cpp
	sshsendfacility.cpp
)

# Export symbols when building a Windows DLL.
TARGET_COMPILE_DEFINITIONS(QSsh PRIVATE QSSH_LIBRARY)

# Link to Botan library.
TARGET_LINK_LIBRARIES(QSsh PRIVATE Botan)

# Link to Qt5.
TARGET_LINK_LIBRARIES(QSsh PUBLIC Qt5::Core Qt5::Gui Qt5::Network Qt5::Widgets)

# Make header files of this library available to dependent targets.
TARGET_INCLUDE_DIRECTORIES(QSsh PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

# Make this library part of the installation package.
IF(NOT OVITO_MONOLITHIC_BUILD)
	INSTALL(TARGETS QSsh EXPORT OVITO
		RUNTIME DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}"
		LIBRARY DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}")
ENDIF()

SET_TARGET_PROPERTIES(QSsh  PROPERTIES MACOSX_RPATH TRUE)

# Export target to make it accessible for external plugins.
IF(CMAKE_VERSION VERSION_LESS "3")
	EXPORT(TARGETS QSsh NAMESPACE "Ovito::" FILE "${${PROJECT_NAME}_BINARY_DIR}/OVITOTargets.cmake")
ENDIF()
