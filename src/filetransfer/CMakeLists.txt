cmake_minimum_required ( VERSION 2.8 ) # todo : take a look at versions > 3


project( transfer_protocol )


set( CMAKE_CXX_STANDARD "11" )

# todo : see tutorials on dependecies 

if( WIN32 )
    set( CURRENT_IMPL "WinAPI" )
elseif ( UNIX )
    set ( CURRENT_IMPL "POSIX" )
endif ()

include_directories( ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/../network ) 
add_library( CTransferTunnel CTransferTunnel.cxx "${PROJECT_SOURCE_DIR}/../network/source/${CURRENT_IMPL}/Source.cxx" ) 
add_executable( general_test ${PROJECT_SOURCE_DIR}/tests/general.cxx )
target_link_libraries( general_test CTransferTunnel ) 
