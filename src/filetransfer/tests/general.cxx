#include <iostream>
#include <chrono>
#include <unordered_map>
#include <string>
#include <exception>

#include "CTransferTunnel.hxx"

using namespace std::chrono ;
using namespace transfer_protocol ;
using namespace network::ip ;

namespace 
{
    void send ( std::size_t size , const std::string& addr , port_type port )
    {
        CSocket peer { EAddressFamily::IPv4 , ESocketType::STERAM } ;
        sock.connect( addr , port ) ;
        CTransferTunnel::send_from_stream( std::cin , sock , size ) ;
    }

    void recv( std::size_t size , const std::string& addr , port_type port )
    {
        CSocket peer { EAddressFamily::IPv4 , ESocketType::STERAM } ;
        sock.bind( addr , port ) ;
        sock.listen( 1 ) ;
        auto source = sock.accept() ;
        CTransferTunnel::recv_to_stream( std::cout , source , size ) ;
    }
}

int main ( int args_num , char ** args ) try
{
    enum { SIZE = 200 , ARGS_NUM = 4 } ;
    
    const std::unordered_map < std::string , void ( * ) ( std::size_t , const std::string& , port_type ) > 
        map { { "send" , send } , 
              { "recv" , recv } } ;
    
    if ( args_num < ARGS_NUM ) { std::cerr << "too few arguments" ;
                                 return - 1 ; }
    
    try { map.at( args[ 1 ] )( SIZE , args[ 2 ] , std::stoi( args[ 3 ] ) ) ; }
    catch ( const std::out_of_bound& ) {
        std::cerr << "invalid args" ;
        return - 1 ;
    }
}
catch ( const std::exception& e )
{
    std::cerr << e.what() ;
    return - 1 ;
}
