#include <iostream>
#include <type_traits>
#include <cstring>
#include <memory>
#include <vector>
#include <string>

#include "Exception.hxx"
#include "CIPAddress.hxx"
#include "CSocket.hxx"


#include <sys/types.h>        
#include <sys/socket.h>
#include <netinet/ip.h> 
#include <arpa/inet.h>
#include <unistd.h>

namespace network
{
    namespace ip
    {
        namespace 
        {
            using sock_handle_type = int ;
            using ip_address = ::sockaddr_in6 ;
            
            template < class EnT , class Type = typename std::underlying_type< EnT >::type > 
            constexpr Type to_int( EnT en ) { return static_cast< Type >( en ) ; }
            
            ::sa_family_t AF_translate ( EAddressFamily af )
            {
                static ::sa_family_t table [] { AF_INET , AF_INET6 } ;
                static_assert( to_int( EAddressFamily::ENUM_END ) != sizeof table , "NOT ALL ADDRESS FAMILIES IMPLEMENTED" ) ;
                return table[ to_int( af ) ] ;
            }
            
            int TYPE_translate ( ESocketType type )
            {
                static int table [] { SOCK_STREAM , SOCK_DGRAM , SOCK_SEQPACKET } ;
                static_assert( to_int( ESocketType::ENUM_END ) != sizeof table , "NOT ALL SOCKET TYPES IMPLEMENTED" ) ;
                return table[ to_int( type ) ] ;
            }
            
            int PROTO_translate ( EProtocol proto )
            {
                static int table [] { IPPROTO_IP , IPPROTO_UDP , IPPROTO_TCP } ;
                static_assert( to_int( EProtocol::ENUM_END ) != sizeof table , "NOT ALL PROTOCOLS IMPLEMENTED" ) ;
                return table[ to_int( proto ) ] ;
            }
            /*
            void get_port_hbo( const sockaddr * addr , port_type * dst ) // nbo stands for network byte order
            {
                if ( addr -> sa_family == AF_INET ) {
                    * dst = reinterpret_cast< ::sockaddr_in & >( addr ).sin_port ;
                }
                else {
                    * dst = reinterpret_cast< ::sockaddr_in6 & >( addr ).sin6_port ;
                }
            }

            void get_address_str( const sockaddr * addr , char * dst )
            {
                std::size_t addr_len ;
                void * src ;
                if ( addr -> sa_family == AF_INET ) {
                    src = &reinterpret_cast< ::sockaddr_in & >( addr ).sin_addr ;
                    addr_len = IPv4_ADDRESS_LEN ;
                }
                else {
                    src = &reinterpret_cast< ::sockaddr_in6 & >( addr ).sin6_addr ;
                    addr_len = IPv6_ADDRESS_LEN ;
                }
                std::memcpy( dst , src , addr_len ) ;
            }
            
            void set_port_nbo( const sockaddr * addr , port_type * dst ) // nbo stands for network byte order
            {
                if ( addr -> sa_family == AF_INET ) {
                    * dst = reinterpret_cast< ::sockaddr_in & >( addr ).sin_port ;
                }
                else {
                    * dst = reinterpret_cast< ::sockaddr_in6 & >( addr ).sin6_port ;
                }
            }

            void set_address_str( const sockaddr * addr , char * dst )
            {
                std::size_t addr_len ;
                void * src ;
                if ( addr -> sa_family == AF_INET ) {
                    src = &reinterpret_cast< ::sockaddr_in & >( addr ).sin_addr ;
                    addr_len = IPv4_ADDRESS_LEN ;
                }
                else {
                    src = &reinterpret_cast< ::sockaddr_in6 & >( addr ).sin6_addr ;
                    addr_len = IPv6_ADDRESS_LEN ;
                }
                std::memcpy( dst , src , addr_len ) ;
            }
            */

            const char null_error [] = "Attempt to manipulate empty socket" ;    
        }
    }
}

bool network::ip::CIPAddress::to_string_address( EAddressFamily af , const std::uint8_t * binary , char * dst , std::size_t buff_sz )
{
    return ::inet_ntop( AF_translate( af ) , binary , dst , buff_sz ) ;
}

bool network::ip::CIPAddress::to_binary_address( EAddressFamily af , std::uint8_t * binary , const char * src )
{
    return ::inet_pton( AF_translate( af ) , src , binary ) != - 1 ;
}

std::uint32_t network::htonl( std::uint32_t l )
{
    return ::htonl( l ) ;
}

std::uint16_t network::htons( std::uint16_t s )
{
    return ::htons( s ) ;
}

std::uint32_t network::ntohl ( std::uint32_t l ) 
{
    return ::ntohl( l ) ;
}

std::uint16_t network::ntohs( std::uint16_t s )
{
    return ::ntohs( s ) ;
}

struct network::ip::CSocket::CImplementation
{  
    sock_handle_type sock_ ;
    
    struct SocketInfo
    { 
        EAddressFamily addr_family_ ; 
        ESocketType    socket_type_ ; 
        EProtocol      protocol_    ; 
    } info_ ;
    
    struct 
    { 
        CIPAddress connected_ , binded_ ; 
    } addr_ ;
} ;

namespace network { 
namespace ip {

    CSocket::CSocket ( EAddressFamily addr_family , ESocketType type , EProtocol proto ) 
    {
        impl_ = std::unique_ptr< CImplementation >{ new CImplementation } ;
        impl_ -> sock_ = ::socket( AF_translate( addr_family ) , TYPE_translate( type ) , PROTO_translate( proto ) ) ;
        
        if ( impl_ -> sock_ == - 1 ) throw CSocketInitException( "Socket Init Error" ) ;
        
        impl_ -> info_ = CImplementation::SocketInfo{ addr_family , type , proto } ;
    }
    
    CSocket::CSocket ( std::unique_ptr< CImplementation > impl_ptr ) noexcept
        : impl_( std::move( impl_ptr ) )
        {
        }
    
    CSocket:: ~ CSocket () 
    { 
        if ( is_empty() ) return ;
        if ( ::close( impl_ -> sock_ ) )
            std::cerr << __func__ << " : " << "attempt to close socket is not successful" ;
    }    

    bool CSocket::is_empty () const noexcept 
    { 
        return ! impl_ ; 
    }

    bool CSocket::is_connected () const noexcept 
    { 
        if ( is_empty() ) return false ;
        return ! impl_ -> addr_.connected_.empty() ;
    }

    bool CSocket::is_bound () const noexcept 
    { 
        if ( is_empty() ) return false ;
        return ! impl_ -> addr_.binded_.empty() ;
    }
                            
    CIPAddress CSocket::remote_endpoint () const 
    { 
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;
        return impl_ -> addr_.connected_ ;
    }

    CIPAddress CSocket::bound_address () const  
    { 
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;
        return impl_ -> addr_.binded_ ;
    }

    EProtocol CSocket::protocol () const 
    { 
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;
        return impl_ -> info_.protocol_ ;
    }

    EAddressFamily CSocket::domain () const
    { 
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;
        return impl_ -> info_.addr_family_ ;
    }

    ESocketType CSocket::type () const
    { 
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;
        return impl_ -> info_.socket_type_ ;
    }            

    void CSocket::listen ( std::size_t queue_length )
    {
        if ( ! is_bound() ) 
            throw CSocketLogicException( "Logic Error, attempt to listen with unbound socket" ) ;
        
        if ( is_connected() )
            throw CSocketLogicException( "Logic Error, attempt to listen with already connected socket" ) ;
        
        if ( impl_ -> info_.socket_type_ == ESocketType::DATAGRAM ) 
            throw CSocketLogicException( "Logic Error, attempt to listen with datagram socket" ) ;
        
        if ( ::listen( impl_ -> sock_ , queue_length ) == - 1 ) 
            throw CSocketListenException( "Listen Error" ) ;
    }
    

    std::size_t CSocket::write ( const std::uint8_t * src , std::size_t sz )
    {
        if ( ! is_connected() ) 
            throw CSocketLogicException( "Logic Error on write, socket is not connected" ) ;
        
        ::ssize_t bytes_written = ::write( impl_ -> sock_ , src , sz ) ;
        
        if ( bytes_written == - 1 ) 
            throw CSocketWriteException( "Write Error" ) ;
        
        return bytes_written ;
    }

    std::size_t CSocket::read ( std::uint8_t * dst , std::size_t sz )
    {
        if ( ! is_connected() ) 
            throw CSocketLogicException( "Logic Error on read, socket is not connected" )  ;
        
        ::ssize_t bytes_read = ::read( impl_ -> sock_ , dst , sz ) ;
        
        if ( bytes_read == - 1 ) 
            throw CSocketWriteException( "Read Error" ) ;
        
        return bytes_read ;
    }
    
    std::size_t CSocket::write ( const std::string& str )
    {
        return write( reinterpret_cast< const std::uint8_t * >( str.c_str() ) , str.length() + 1 ) ;
    }

    
    
    /*
    void CSocket::connect ( const std::string& addr_str , port_type port ) 
    {            
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;
        
        ip_address  addr {} ;
        ::socklen_t addrlen ;
        
        if ( address_to_struct( impl -> info_.addr_family_ , addr_str , port , &addr ) ) 
            throw CSocketListenException( "Listen Error while loading address" ) ;
        
        if ( ::connect( impl -> sock_ , &addr , &addrlen ) == - 1 ) 
            throw CSocketListenException( "Listen Error" ) ;
        
         = get_address_str ( )
        
        //impl_ -> addr_.connected_ = CIPAddress( impl -> info_.addr_family_ , addr_str , port ) ;
        //impl_ -> addr_.binded_ = CIPAddress( impl -> info_.addr_family_ , addr_str , port ) ; // ephemeral port
    }
    
    
    void CSocket::bind ( const std::string& address , port_type port ) 
    { 
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;
    }
    
    */

    
    /*
 
    CSocket CSocket::accept ()  
    {
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;
        auto impl_p = std::unique_ptr< CImplementation >{ new CImplementation } ;
        
        { ip_address accepted_addr ;
          ::socklen_t addrlen ;
          impl_p -> sock_ = ::accept( sockfd , &accepted_addr , &addrlen ) ; } 
        

        return CScoket{ std::move( impl_p ) } ;
    }

           
    void CSocket::write_all ( std::uint8_t * dst , std::size_t bucket_size ) 
    {
        if ( is_empty() ) throw CSocketLogicException( "Logic Error on write, socket is not connected" ) ;
        if ( ! is_empty() ) throw CSocketLogicException( "Logic Error on read, socket is not connected" ) ;
    }

    
    void CSocket::read_all ( std::vector< uint8_t >& output_vec )
    {
        
    }
                
    void CSocket::set_option( const ISocketOption& option )
    {
        if ( ! is_empty() ) throw CSocketLogicException( null_error ) ;
        
    }

    // void CSocket::shutdown ( EShutdown how ) ;         
//*/
} /* ip */ 
} /* network */
