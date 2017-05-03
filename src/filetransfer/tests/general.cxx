#include <iostream>
#include <chrono>
#include <unordered_map>
#include <string>
#include <exception>
#include <memory>
#include <fstream>

#include "CTransferTunnel.hxx"

using namespace std::chrono ;
using namespace transfer_protocol ;
using namespace network::ip ;

// 
// usage :
// general_test operation address port filename
//


namespace 
{
    void send ( const std::string& addr , port_type port , const std::string& filename )
    {
        std::ifstream stream ( filename , std::ios_base::binary ) ;
        CSocket peer { EAddressFamily::IPv4 , ESocketType::STREAM } ;
        peer.connect( addr , port ) ;
        peer.set_option( CWriteTimeout{ seconds{ 5 } } ) ;
        peer.set_option( CReadTimeout{ seconds{ 5 } } ) ;
        CTransferTunnel_TCP::send_stream( stream , peer ) ;
    }

    void recv( const std::string& addr , port_type port , const std::string& filename )
    {
        std::ofstream stream ( filename , std::ios_base::binary ) ;
        CSocket peer { EAddressFamily::IPv4 , ESocketType::STREAM } ;
        peer.bind( addr , port ) ;
        peer.listen( 1 ) ;
        auto source = peer.accept() ;
        source.set_option( CWriteTimeout{ seconds{ 5 } } ) ;
        source.set_option( CReadTimeout{ seconds{ 5 } } ) ;
        CTransferTunnel_TCP::recv_stream( stream , source ) ;
    }
}

int main ( int args_num , char ** args ) try
{
    enum ARGS { OPERATION = 1 , ADDRESS , PORT , FNAME , ARGS_NUM } ;
    
    const std::unordered_map < std::string , void ( * ) ( const std::string& , port_type , const std::string& ) > 
        op_map { { "send" , send } , 
                 { "recv" , recv } } ;
    
    if ( args_num < ARGS_NUM ) { std::cerr << "usage : general_test operation address port filename" ;
                                 return - 1 ; }
    
    try 
    { 
        op_map.at( args[ OPERATION ] )( args[ ADDRESS ] , std::stoi( args[ PORT ] ) , args[ FNAME ] ) ; 
    }
    catch ( const std::out_of_range& ) 
    {
        std::cerr << "invalid args" ;
        return - 1 ;
    }
}
catch ( const std::exception& e )
{
    std::cerr << e.what() ;
    return - 1 ;
}
