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
ADD_LIBRARY(QSsh
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

# This will let the source code export symbols when building a DLL.
TARGET_COMPILE_DEFINITIONS(QSsh PRIVATE QSSH_LIBRARY)

# Link required Botan library.
TARGET_LINK_LIBRARIES(QSsh Botan)

# Make header files of this library available to dependent targets.
TARGET_INCLUDE_DIRECTORIES(QSsh INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

# Link Qt5.
QT5_USE_MODULES(QSsh ${OVITO_REQUIRED_QT_MODULES})

# This library is part of the installation package.
IF(NOT OVITO_MONOLITHIC_BUILD)
	INSTALL(TARGETS QSsh 
		RUNTIME DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}"
		LIBRARY DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}")
ENDIF()
