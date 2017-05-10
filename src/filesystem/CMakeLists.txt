cmake_minimum_required ( VERSION 2.8 ) # todo : take a look at versions > 3

project( network )

set( CMAKE_CXX_STANDARD "11" )


if( WIN32 )
    set( CURRENT_IMPL "WinAPI" )
elseif ( UNIX )
    set ( CURRENT_IMPL "POSIX" )
endif ()

include_directories( ${PROJECT_SOURCE_DIR} )

function ( build_library )
    add_library( FileSystem STATIC ${PROJECT_SOURCE_DIR}/source/${CURRENT_IMPL}/Source.cxx )
endfunction()

function ( build_tests )
    add_executable( general ${PROJECT_SOURCE_DIR}/tests/general.cxx )
    target_link_libraries( general FileSystem )
endfunction()

build_library()
build_tests()
