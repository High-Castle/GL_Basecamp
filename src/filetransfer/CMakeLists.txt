cmake_minimum_required ( VERSION 2.8 ) # todo : take a look at versions > 3


project( transfer_protocol )


set( CMAKE_CXX_STANDARD "11" )

# todo : see tutorials on dependecies 


include_directories( ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/../network ) 
add_library( CTransferTunnel CTransferTunnel.cxx "${PROJECT_SOURCE_DIR}/../network/source/POSIX/Source.cxx" ) 
add_executable( general_test ${PROJECT_SOURCE_DIR}/tests/general.cxx )
target_link_libraries( general_test CTransferTunnel ) 
