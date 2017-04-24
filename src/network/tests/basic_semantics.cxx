// POSIX implementation file
#include <iostream>

#include "CSocket.hxx"
#include "CIPAddress.hxx"

#include <cassert>

int main () 
{
    namespace ip = network::ip ;
    
    ip::CSocket sock { ip::EAddressFamily::IPv4 , ip::ESocketType::STREAM } ;
    
    sock.set_option( ip::CReuseAddress{ true } ) ;
    
    sock.set_option( ip::CReadTimeout{ ip::seconds{ 1 } } ) ;
    sock.set_option( ip::CWriteTimeout{ ip::seconds{ 5 } } ) ;
    
    {   ip::CReuseAddress opt ; 
        sock.get_option( opt ) ;
        assert( opt.value() ) ;   }
    
    {   ip::CReadTimeout opt ; 
        sock.get_option( opt ) ;
        std::cerr << std::chrono::duration_cast< ip::seconds >( opt.value() ).count() << "  " << ( opt.value() % ip::seconds{ 1 }).count() << "\n" ;
    }
    
    { ip::CBlock opt ;
      sock.get_option( opt ) ;
      assert( opt.value() ) ; }
    
    assert( ! sock.is_empty() ) ;
    assert( ! sock.is_bound() ) ;
    assert( ! sock.is_connected() ) ;
    
    try 
    {
        //auto sock2 = std::move( sock ) ;
        //sock.connect( "127.0.0.1" , 8084 ) ;
        sock.bind( "127.0.0.1" , 8084 ) ;
        
        sock.listen( 1 ) ;
        
        ip::CSocket acc_sock = sock.accept() ;
        
        std::size_t written ;
        try 
        { 
            char message [ 35 ] ; message[ 0 ] = '\0' ;
            acc_sock.read( ( std::uint8_t * ) message , sizeof message ) ;
            std::cout << message ;
            //sock.write_all( "ip::CSocket" , written ) ; 
        } 
            catch ( const ip::CSocketReadAttemptException& e )
            {
                std::cerr << e.what() ;
            }        
        std::cout << "\n" << sock.bound_address().address() << " " << sock.bound_address().port() << "\n" ;
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
