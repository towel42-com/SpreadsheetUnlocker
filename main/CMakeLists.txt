# The MIT License (MIT)
#
# Copyright (c) 2025 Scott Aron Bloom
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
cmake_minimum_required(VERSION 3.22)

get_property(ROOT_PROJECT GLOBAL PROPERTY MAIN_EXE_PROJECT_NAME)
project(${ROOT_PROJECT}) 

include( include.cmake )
include( ${CMAKE_SOURCE_DIR}/T42-CMakeUtils/QtProject.cmake )

add_executable( ${PROJECT_NAME} )
target_sources( ${PROJECT_NAME} PRIVATE ${_PROJECT_DEPENDENCIES} )

if ( NOT DEPLOYQT_EXECUTABLE )
    message( FATAL_ERROR "DEPLOYQT_EXECUTABLE not set" )
endif()
          
get_filename_component( QTDIR ${DEPLOYQT_EXECUTABLE} DIRECTORY )
include_directories( ${CMAKE_BINARY_DIR} )
set ( DEBUG_PATH 
        "%PATH%"
        "$<TARGET_FILE_DIR:Towel42Utils>"
        "${QTDIR}"
        )

set_target_properties( ${PROJECT_NAME} PROPERTIES 
        FOLDER Apps
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>" 
        VS_DEBUGGER_COMMAND "$<TARGET_FILE:${PROJECT_NAME}>" 
        VS_DEBUGGER_ENVIRONMENT "PATH=${DEBUG_PATH}" 
        )


target_link_libraries( ${PROJECT_NAME}
      PUBLIC
          ${project_pub_DEPS}
      PRIVATE
          ${project_pri_DEPS}
)

DeployQt( ${PROJECT_NAME} . 
    EXTRA_TARGETS
       Towel42Utils
)

DeploySystem( ${PROJECT_NAME} . INSTALL_ONLY 1 )

INSTALL( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin )
INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${PROJECT_NAME}.pdb DESTINATION . CONFIGURATIONS Debug RelWithDebInfo )


