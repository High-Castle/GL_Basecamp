cmake_minimum_required ( VERSION 2.8 ) # todo : take a look at versions > 3


project( transfer_protocol )


set( CMAKE_CXX_STANDARD "11" )

# TODO : see tutorials on dependecies , rewrite as project include

if( WIN32 )
    set( CURRENT_IMPL "WinAPI" )
elseif ( UNIX )
    set ( CURRENT_IMPL "POSIX" )
   # set ( CMAKE_CXX_FLAGS "-pg -g3 -ggdb" ) # http://stackoverflow.com/questions/43747758/different-unexpected-errors-when-building-with-pg-option
   # set ( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pg" )
endif ()

include_directories( ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/../network ) 
add_library( CTransferTunnel CTransferTunnel.cxx "${PROJECT_SOURCE_DIR}/../network/source/${CURRENT_IMPL}/Source.cxx" ) 
add_executable( general_test ${PROJECT_SOURCE_DIR}/tests/general.cxx )
target_link_libraries( general_test CTransferTunnel ) 
