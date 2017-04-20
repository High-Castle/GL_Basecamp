cmake_minimum_required ( VERSION 2.8 ) # todo : take a look at versions > 3

project( network )

#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" ) 
set( CMAKE_CXX_STANDARD "11" )

if( WIN32 )
    set( CURRENT_IMPL "WinAPI" )
elseif ( UNIX )
    set ( CURRENT_IMPL "POSIX" )
endif ()

include_directories( ${PROJECT_SOURCE_DIR} ) # defined by project() command
#add_library( source STATIC ${PROJECT_SOURCE_DIR}/source/${CURRENT_IMPL}/Source.cxx  )
add_executable( basic_semantics ${PROJECT_SOURCE_DIR}/tests/basic_semantics.cxx ${PROJECT_SOURCE_DIR}/source/${CURRENT_IMPL}/Source.cxx  )
#target_link_libraries
