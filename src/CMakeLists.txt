cmake_minimum_required(VERSION 2.8.10)

set( CMAKE_DISABLE_SOURCE_CHANGES ON )
set( CMAKE_DISABLE_IN_SOURCE_BUILD ON )

add_subdirectory( network ) 

add_subdirectory( filetransfer ) 

add_subdirectory( application )
