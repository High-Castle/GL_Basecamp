// POSIX implementation file
#include <iostream>

#include "CSocket.hxx"
#include "CIPAddress.hxx"

#include <cassert>

int main () 
{
    namespace ip = network::ip ;
    
    ip::CSocket sock { ip::EAddressFamily::IPv4 , ip::ESocketType::STREAM } ;
    
    assert( ! sock.is_empty() ) ;
    assert( ! sock.is_bound() ) ;
    assert( ! sock.is_connected() ) ;
    
    try 
    {
        //auto sock2 = std::move( sock ) ;
        sock.bind( "0.0.0.0" , 8081 ) ;
        
        sock.listen( 1 ) ;
        
        ip::CSocket acc_sock = sock.accept() ;
        
        acc_sock.write( "ip::CSocket" ) ;
        
        std::cout << sock.bound_address().address() << " " << sock.bound_address().port() << "\n" ;
    }
    catch ( const ip::CSocketException& e )
    {
        std::cerr << e.what() << "\n" ;
    }
        
    
    ip::CIPAddress addr { ip::EAddressFamily::IPv4 , "192.168.67.5" , 1235 } ;
    ip::CIPAddress addr2 ;
    assert( addr.is_valid_string() ) ;
    assert( addr2.empty() ) ;
    addr2 = addr ;
    assert( addr == addr2 ) ;
    std::cerr << addr.address() ;
    assert( ( addr == ip::CIPAddress{ ip::EAddressFamily::IPv4 , "192.168.67.5" , 1235 } ) ) ;
    assert( addr.port() == 1235 ) ;
}
