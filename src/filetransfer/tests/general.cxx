#include <iostream>
#include <chrono>
#include <unordered_map>
#include <string>
#include <exception>

#include "CTransferTunnel.hxx"

using namespace std::chrono ;
using namespace transfer_protocol ;
using namespace network::ip ;

// 
// usage :
// general_test operation address port max_filesize ( > < output )
//

namespace 
{
    void send ( std::size_t size , const std::string& addr , port_type port )
    {
        CSocket peer { EAddressFamily::IPv4 , ESocketType::STREAM } ;
        peer.connect( addr , port ) ;
        CTransferTunnel_TCP::send_from_stream( std::cin , peer , size ) ;
    }

    void recv( std::size_t size , const std::string& addr , port_type port )
    {
        CSocket peer { EAddressFamily::IPv4 , ESocketType::STREAM } ;
        peer.bind( addr , port ) ;
        peer.listen( 1 ) ;
        auto source = peer.accept() ;
        CTransferTunnel_TCP::recv_to_stream( std::cout , source , size ) ;
    }
}

int main ( int args_num , char ** args ) try
{
    enum ARGS { OPERATION = 1 , ADDRESS , PORT , SIZE , ARGS_NUM } ;
    std::cout.sync_with_stdio( false ) ;
    std::cin.sync_with_stdio( false ) ;
    
    const std::unordered_map < std::string , void ( * ) ( std::size_t , const std::string& , port_type ) > 
        op_map { { "send" , send } , 
                 { "recv" , recv } } ;
    
    if ( args_num < ARGS_NUM ) { std::cerr << "too few arguments" ;
                                 return - 1 ; }
    
    try 
    { 
        op_map.at( args[ OPERATION ] )( std::stoi( args[ SIZE ] ) , args[ ADDRESS ] , std::stoi( args[ PORT ] ) ) ; 
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
