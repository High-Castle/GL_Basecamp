cmake_minimum_required(VERSION 2.8.10)

project( splitted_transfer )

set( CMAKE_CXX_STANDARD "11" )

if( WIN32 )
    set( CURRENT_IMPL "WinAPI" )
elseif ( UNIX )
    set ( CURRENT_IMPL "POSIX" )
    set ( CMAKE_CXX_FLAGS "-Wall -pedantic -traditional-cpp" )
    #set ( CMAKE_CXX_COMPILER "icc" )
endif ()

include_directories( ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/network  ${PROJECT_SOURCE_DIR}/filesystem ${PROJECT_SOURCE_DIR}/filetransfer )

function ( build_library )
    add_library( FileSystem STATIC ${PROJECT_SOURCE_DIR}/filesystem/source/${CURRENT_IMPL}/Source.cxx )
    add_library( Network STATIC ${PROJECT_SOURCE_DIR}/network/source/${CURRENT_IMPL}/Source.cxx )
    add_library( Transfer STATIC ${PROJECT_SOURCE_DIR}/filetransfer/CTransferTunnel.cxx )
endfunction()

function ( build_tests )
    add_executable( splitted_transfer ${PROJECT_SOURCE_DIR}/splitted_transfer.cxx )
    target_link_libraries( splitted_transfer FileSystem Network Transfer )
endfunction()

build_library()
build_tests()
