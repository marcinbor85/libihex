cmake_minimum_required( VERSION 3.0 )

project( libihex LANGUAGES C )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

enable_testing( )

include_directories( ${PROJECT_SOURCE_DIR}/src )

file( GLOB SRC_FILES src/*.c )
add_library( ihex SHARED ${SRC_FILES} )
set_target_properties( ihex PROPERTIES VERSION 0.1.0 SOVERSION 1 )

install( TARGETS ihex DESTINATION lib )
install( FILES src/ihex.h DESTINATION include )

add_executable( test_parse tests/test_parse.c )
target_link_libraries( test_parse ihex )
add_test( test_parse ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_parse )

add_executable( test_dump tests/test_dump.c )
target_link_libraries( test_dump ihex )
add_test( test_dump ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_dump )

add_custom_target( check COMMAND ${CMAKE_CTEST_COMMAND} --verbose )
add_custom_target( cleanall COMMAND rm -rf Makefile CMakeCache.txt CMakeFiles/ bin/ lib/ cmake_install.cmake CTestTestfile.cmake Testing/ )
add_custom_target( uninstall COMMAND xargs rm < install_manifest.txt )