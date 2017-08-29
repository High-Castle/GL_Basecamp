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
    add_library( CSocket STATIC ${PROJECT_SOURCE_DIR}/source/${CURRENT_IMPL}/Source.cxx )
endfunction()

function ( build_tests )
    add_executable( basic_semantics ${PROJECT_SOURCE_DIR}/tests/basic_semantics.cxx )
    target_link_libraries( basic_semantics CSocket )
endfunction()

build_library()
build_tests()

