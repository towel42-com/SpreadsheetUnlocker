# The MIT License (MIT)
#
# Copyright (c) 2026 Scott Aron Bloom
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.31)

set( MAIN_EXE_PROJECT_NAME "XLXSUnlocker" )
SET_PROPERTY(GLOBAL PROPERTY MAIN_EXE_PROJECT_NAME ${MAIN_EXE_PROJECT_NAME})
project(Root-${MAIN_EXE_PROJECT_NAME} VERSION 1.0)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/T42-CMakeUtils/Modules ${CMAKE_MODULE_PATH})
SET(TOWEL42_ENABLE_TESTING ON)

set( TOWEL42_NO_DEPLOY_SUBMODULES ON )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0043 NEW)


find_package(Qt6 COMPONENTS Core Widgets Gui REQUIRED)
find_package(Deploy REQUIRED)
find_package(AddUnitTest REQUIRED)
set_property( GLOBAL PROPERTY USE_FOLDERS ON )

include( VersionInfo.cmake )

file( REAL_PATH ~ HOMEDIR EXPAND_TILDE)
set( CMAKE_INSTALL_PREFIX ${HOMEDIR}/bin/${MAIN_EXE_PROJECT_NAME} )
if ( NOT EXISTS ${CMAKE_INSTALL_PREFIX} )
    file( MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX} )
endif()

add_subdirectory( T42-Utils )
add_subdirectory( MainWindow )

add_subdirectory( main )

SET( CPACK_PACKAGE_VERSION_MAJOR ${MAJOR_VERSION} )
SET( CPACK_PACKAGE_VERSION_MINOR ${MINOR_VERSION} )
SET( CPACK_PACKAGE_VERSION_PATCH ${VERSION_FILE_PATCH_VERSION} )
SET( CPACK_PACKAGE_VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${VERSION_FILE_PATCH_VERSION})

SET( CPACK_PACKAGE_NAME ${APP_NAME} )
SET( CPACK_PACKAGE_VENDOR "${VENDOR} ${EMAIL}" )
SET( CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE )
SET( CPACK_PACKAGE_HOMEPAGE_URL ${HOMEPAGE} )
set( CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${VENDOR}/${APP_NAME}" )
SET( CPACK_PACKAGE_DESCRIPTION "Application to find duplicate files" )

SET(CPACK_NSIS_CREATE_ICONS_EXTRA
    "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\${MAIN_EXE_PROJECT_NAME}.lnk' '$INSTDIR\\\\${MAIN_EXE_PROJECT_NAME}.exe'"
)

SET(CPACK_NSIS_MODIFY_PATH "ON")

set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
    CreateShortCut \\\"$DESKTOP\\\\${APP_NAME}.lnk\\\" \\\"$INSTDIR\\\\${MAIN_EXE_PROJECT_NAME}.exe\\\"
")

set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
    Delete \\\"$DESKTOP\\\\${APP_NAME}.lnk\\\"
")

SET( CPACK_GENERATOR NSIS ZIP )
SET( CPACK_NSIS_MUI_ICON ${CMAKE_SOURCE_DIR}/MainWindow/resources/application.ico )
SET( CPACK_NSIS_MUI_UNIICON ${CMAKE_SOURCE_DIR}/MainWindow/resources/application.ico )
include( CPack )

find_package(FolderizeQtAutoProjects REQUIRED)
FolderizeQtAutoProjects()
