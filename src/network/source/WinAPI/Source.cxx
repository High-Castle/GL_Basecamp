#include <cstdlib>

#include <winsock2.h>

#include "CSocket.hxx"
#include "ImplSpecificTypes.hxx"
#include "CImplmentation.hxx"



namespace
{
    WSADATA const& attempt_init_WSA () 
    { 
        static struct Init 
        {
            Init () 
            {
                
                if ( WSAStratup( MAKEWORD( 2,2 ) , &wsa_data ) == -1 )
                    throw CSocketInitException( "Socket Init Error : WSAStartup Failed" ) ;
                std::atexit( [] () // DO NOT PLACE THAT IN DESTRUCTOR, NEVER. functions, that are set with std::atexit, will be called before destruction of globals,
                {
                    WSACleanup() ;
                } ) ; 
            }
            WSADATA wsa_data ;
        } init ;
        
        return init.wsa_data ;
    }
}

namespace network {
namespace ip {
    
    CSocket::CSocket ( EAddressFamily addr_family , ESocketType type , EProtocol proto ) 
    {
        attempt_init_WSA () ;
        impl_ = std::unique_ptr< CImplementation >{ new CImplementation } ;
        impl() -> sock_ = ::socket( AF_translate( addr_family ) , TYPE_translate( type ) , PROTO_translate( proto ) ) ;
        if ( impl() -> sock_ == - 1 ) throw CSocketInitException( "Socket Init Error" ) ;
        impl() -> info_ = CImplementation::SocketInfo{ addr_family , type , proto } ;
    }
    

    CSocket:: ~ CSocket () 
    { 
        if ( is_empty() ) return ;
        if ( ::closesocket( impl() -> sock_ ) )
            std::cerr << __func__ << " : " << "attempt to close socket is not successful" ;
    }  
    
    
    
    
} // ip
} // network
