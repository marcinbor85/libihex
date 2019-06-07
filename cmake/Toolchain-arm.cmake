
set( CMAKE_SYSTEM_NAME Linux )
set( CMAKE_SYSTEM_VERSION 1 )

set( CMAKE_C_COMPILER   arm-linux-gnueabihf-gcc )
set( CMAKE_CXX_COMPILER arm-linux-gnueabihf-gcc )

set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )

add_definitions( -Wall -std=c11 )
