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

SET(SSH_LIB_SOURCE_FILES 
	botan/botan.cpp

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

IF(APPLE)
	ADD_DEFINITIONS(
		-DBOTAN_TARGET_OS_IS_DARWIN 
		-DBOTAN_BUILD_COMPILER_IS_CLANG 
		-DBOTAN_TARGET_ARCH_IS_X86_64 
		-DBOTAN_HAS_MUTEX_PTHREAD
		-DBOTAN_TARGET_OS_IS_LINUX 
		-DBOTAN_TARGET_OS_HAS_DLOPEN 
		-DBOTAN_TARGET_OS_HAS_GMTIME_R 
		-DBOTAN_TARGET_OS_HAS_POSIX_MLOCK
		-DBOTAN_HAS_DYNAMICALLY_LOADED_ENGINE 
		-DBOTAN_HAS_DYNAMIC_LOADER
		-DBOTAN_TARGET_OS_HAS_GETTIMEOFDAY 
		-DBOTAN_HAS_ALLOC_MMAP
		-DBOTAN_HAS_ENTROPY_SRC_DEV_RANDOM 
		-DBOTAN_HAS_ENTROPY_SRC_EGD 
		-DBOTAN_HAS_ENTROPY_SRC_FTW
		-DBOTAN_HAS_ENTROPY_SRC_UNIX 
		-DBOTAN_HAS_MUTEX_PTHREAD 
		-DBOTAN_HAS_PIPE_UNIXFD_IO
	)
ELSEIF(UNIX)
	ADD_DEFINITIONS(
		-DBOTAN_TARGET_OS_HAS_GETTIMEOFDAY 
		-DBOTAN_HAS_ALLOC_MMAP
    	-DBOTAN_HAS_ENTROPY_SRC_DEV_RANDOM 
    	-DBOTAN_HAS_ENTROPY_SRC_EGD 
    	-DBOTAN_HAS_ENTROPY_SRC_FTW
    	-DBOTAN_HAS_ENTROPY_SRC_UNIX 
    	-DBOTAN_HAS_MUTEX_PTHREAD 
    	-DBOTAN_HAS_PIPE_UNIXFD_IO
	)
ELSEIF(WIN32)
	ADD_DEFINITIONS(
		-DBOTAN_TARGET_OS_IS_WINDOWS
        -DBOTAN_TARGET_OS_HAS_LOADLIBRARY -DBOTAN_TARGET_OS_HAS_WIN32_GET_SYSTEMTIME
        -DBOTAN_TARGET_OS_HAS_WIN32_VIRTUAL_LOCK -DBOTAN_HAS_DYNAMICALLY_LOADED_ENGINE
        -DBOTAN_HAS_DYNAMIC_LOADER -DBOTAN_HAS_ENTROPY_SRC_CAPI -DBOTAN_HAS_ENTROPY_SRC_WIN32
        -DBOTAN_HAS_MUTEX_WIN32
	)
	ADD_DEFINITIONS(-DBOTAN_DLL=Q_DECL_EXPORT)
	IF(MSVC)
		ADD_DEFINITIONS(-DBOTAN_BUILD_COMPILER_IS_MSVC -DBOTAN_TARGET_OS_HAS_GMTIME_S)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4251 -wd4290 -wd4250")
	ENDIF()
ELSE()
	MESSAGE(FATAL_ERROR "Building on this platform is not supported.")
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCXX)
	ADD_DEFINITIONS(-DBOTAN_BUILD_COMPILER_IS_GCC)
ENDIF()

# This will let the source code export symbols when building a DLL.
ADD_DEFINITIONS(-DQSSH_LIBRARY)

# Build library
ADD_LIBRARY(QSsh ${SSH_LIB_SOURCE_FILES})

# Link Qt5.
QT5_USE_MODULES(QSsh Core Network Widgets Gui)

# This library is part of the installation package.
IF(APPLE OR WIN32)
	INSTALL(TARGETS QSsh 
		RUNTIME DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}"
		LIBRARY DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}")
ENDIF()
