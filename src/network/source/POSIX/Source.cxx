#include <iostream>
#include <type_traits>
#include <cstring>
#include <memory>
#include <vector>
#include <string>

#include <initializer_list>

#include "Exception.hxx"
#include "CIPAddress.hxx"
#include "CSocket.hxx"


#include <sys/types.h>        
#include <sys/socket.h>
#include <netinet/ip.h> 
#include <arpa/inet.h>
#include <unistd.h>

#include <errno.h> 

// todo : handle SIGPIPE
// todo : write_all
// todo : select / poll

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
            
            int SHUT_translate ( EShutdown how )
            {
                static int table [] { SHUT_RD , SHUT_WR , SHUT_RDWR } ;
                static_assert( to_int( EShutdown::ENUM_END ) != sizeof table , "NOT ALL EShutdown IMPLEMENTED" ) ;
                return table[ to_int( how ) ] ;
            }
            
            template < class EnT > int FLAG_translate ( EnT flg ) = delete ;
            
            template <> int FLAG_translate <> ( EWriteFlags flg ) {
                static int table [] { MSG_DONTWAIT } ;
                static_assert( to_int( EWriteFlags::ENUM_END ) != sizeof table , "NOT ALL EWriteFlags IMPLEMENTED" ) ;
                return table[ to_int( flg ) ] ;
            }
            
            template <> int FLAG_translate <> ( EReadFlags flg ) {
                static int table [] { MSG_DONTWAIT , MSG_WAITALL } ;
                static_assert( to_int( EReadFlags::ENUM_END ) != sizeof table , "NOT ALL EReadFlags IMPLEMENTED" ) ;
                return table[ to_int( flg ) ] ;
            }
            
            template < class FlagsIt > int FLAGS_eval ( FlagsIt from , FlagsIt to ) {
                int acc ;
                for ( ; from != to ; ++ from ) 
                    acc |= FLAG_translate( * from ) ;
                return acc ;
            }
            
            bool get_address( const ::sockaddr * addr , char * str_dst , port_type * port_dst , EAddressFamily * af_dst )
            {
                if ( addr -> sa_family == AF_INET ) {
                    auto& ipaddr = reinterpret_cast< const ::sockaddr_in & >( * addr ) ;
                    if ( inet_ntop( AF_INET , &ipaddr.sin_addr , str_dst , INET_ADDRSTRLEN ) == NULL ) return false ;
                    * af_dst   = EAddressFamily::IPv4 ;
                    * port_dst = ::ntohs( ipaddr.sin_port ) ;
                }
                else {
                    auto& ip6addr = reinterpret_cast< const ::sockaddr_in6 & >( * addr ) ;
                    if ( inet_ntop( AF_INET6 , &ip6addr.sin6_addr , str_dst , INET6_ADDRSTRLEN ) == NULL ) return false ;
                    * af_dst   = EAddressFamily::IPv6 ;
                    * port_dst = ::ntohs( ip6addr.sin6_port ) ;
                }
                return true ;
            }
            
            bool set_address( EAddressFamily af , const char * addr_str , port_type port , ::sockaddr * addr )
            {
                if ( af == EAddressFamily::IPv4 ) {
                    auto& ipaddr = reinterpret_cast< ::sockaddr_in & >( * addr ) ;
                    if ( inet_pton( AF_INET , addr_str , &ipaddr.sin_addr ) != 1 ) return false ;
                    ipaddr.sin_family = AF_INET ;
                    ipaddr.sin_port  = ::htons( port ) ;
                }
                else {
                    auto& ip6addr = reinterpret_cast< ::sockaddr_in6 & >( * addr ) ;
                    if( inet_pton( AF_INET6 , addr_str , &ip6addr.sin6_addr ) != 1 ) return false ;
                    ip6addr.sin6_family = AF_INET6 ;
                    ip6addr.sin6_port = ::htons( port ) ;
                }
                return true ;
            }

            const char null_error [] = "Attempt to manipulate empty socket" ;    
        }
    }
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
                : network::ip::CSocket::ImplementationBase
{  
    CImplementation () : is_connected_{ false } , is_bound_{ false }
    { 
    }
    
    sock_handle_type sock_ ;   
    struct SocketInfo
    { 
        EAddressFamily addr_family_ ; 
        ESocketType    socket_type_ ; 
        EProtocol      protocol_    ; 
    } info_ ;
    std::uint8_t is_connected_ : 1 , is_bound_ : 1 ;
} ;

network::ip::CSocket::CImplementation * network::ip::CSocket::impl ()
{
    return static_cast< CImplementation * >( impl_.get() ) ;
}

const network::ip::CSocket::CImplementation * network::ip::CSocket::impl () const
{
    return static_cast< const CImplementation * >( impl_.get() ) ;
}

namespace network { 
namespace ip {

    CSocket::CSocket ( EAddressFamily addr_family , ESocketType type , EProtocol proto ) 
    {
        impl_ = std::unique_ptr< CImplementation >{ new CImplementation } ;
        impl() -> sock_ = ::socket( AF_translate( addr_family ) , TYPE_translate( type ) , PROTO_translate( proto ) ) ;
        
        if ( impl() -> sock_ == - 1 ) throw CSocketInitException( "Socket Init Error" ) ;
        
        impl() -> info_ = CImplementation::SocketInfo{ addr_family , type , proto } ;
    }
    
    CSocket::CSocket ( std::unique_ptr< ImplementationBase > impl_ptr ) noexcept
        : impl_( std::move( impl_ptr ) )
        {
        }
    
    CSocket:: ~ CSocket () 
    { 
        if ( is_empty() ) return ;
        if ( ::close( impl() -> sock_ ) )
            std::cerr << "cerr : " << __func__ << " : " << "attempt to close socket is not successful" ;
    }    

    bool CSocket::is_empty () const noexcept 
    { 
        return ! impl() ; 
    }

    bool CSocket::is_connected () const noexcept 
    { 
        if ( is_empty() ) return false ;
        return impl() -> is_connected_ ;
    }

    bool CSocket::is_bound () const noexcept 
    { 
        if ( is_empty() ) return false ;
        return impl() -> is_bound_ ;
    }
                            
    CIPAddress CSocket::remote_endpoint () const 
    { 
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;
        
        ip_address addr {} ;   
        ::socklen_t addrlen = sizeof addr ;
        
        if ( ::getpeername( impl() -> sock_ , ( ::sockaddr * ) &addr ,  &addrlen ) == - 1 )
        {
            std::cerr << "cerr : " << __func__ << " : " << "getpeername() failed" ;
            return {} ;
        }
        CIPAddress peer_addr ; // will be removed in next updates
        
        if ( ! get_address( ( ::sockaddr * ) &addr , peer_addr.addr_ , &peer_addr.port_ , &peer_addr.family_ ) ) 
        {
            std::cerr << "cerr : " << __func__ << " : " << "problems on getting address" ;
            return {} ;
        }
        return peer_addr ;
    }

    CIPAddress CSocket::bound_address () const  
    { 
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;

        ip_address addr {} ;
        ::socklen_t addrlen = sizeof addr ;
        
        if ( ::getsockname( impl() -> sock_ , ( ::sockaddr * ) &addr ,  &addrlen ) == - 1 )
        {
            std::cerr << "cerr : " << __func__ << " : " << "getsockname() failed" ;
            return {} ;
        }
        
        CIPAddress bound_addr ; // will be removed in next updates
        
        if ( ! get_address( ( ::sockaddr * ) &addr , bound_addr.addr_ , &bound_addr.port_ , &bound_addr.family_ ) ) 
        {
            std::cerr << "cerr : " << __func__ << " : " << "problems on getting address" ;
            return {} ;
        }
        
        return bound_addr ;
    }

    EProtocol CSocket::protocol () const 
    { 
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;
        return impl() -> info_.protocol_ ;
    }

    EAddressFamily CSocket::domain () const
    { 
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;
        return impl() -> info_.addr_family_ ;
    }

    ESocketType CSocket::type () const
    { 
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;
        return impl() -> info_.socket_type_ ;
    }            

    void CSocket::listen ( std::size_t queue_length )
    {
        if ( ! is_bound() ) 
            throw CSocketLogicException( "Logic Error, attempt to listen with unbound socket" ) ;
        
        if ( is_connected() )
            throw CSocketLogicException( "Logic Error, attempt to listen with already connected socket" ) ;
        
        if ( impl() -> info_.socket_type_ == ESocketType::DATAGRAM ) 
            throw CSocketLogicException( "Logic Error, attempt to listen with datagram socket" ) ;
        
        if ( ::listen( impl() -> sock_ , queue_length ) == - 1 ) 
        {
            switch ( errno )
            {
            }
            throw CSocketListenException( "Listen Exception" ) ;
        }
    }
    

    std::size_t CSocket::write ( const std::uint8_t * src , std::size_t sz , std::initializer_list< EWriteFlags > flags )
    {
        if ( ! is_connected() ) 
            throw CSocketLogicException( "Logic Error on write, socket is not connected" ) ;
        
        int flg_mask = FLAGS_eval( flags.begin() , flags.end() ) ;
        ::ssize_t bytes_written = ::send( impl() -> sock_ , src , sz , MSG_NOSIGNAL | flg_mask ) ;
        
        if ( bytes_written == - 1 ) {
            switch ( errno )
            {
                case ECONNRESET : throw CSocketConnectionException( "Connection on Write Exception : Connection is reset by peer" ) ;
                case EPIPE      : throw CSocketConnectionException( "Connection on Write Exception : Broken Pipe" ) ;
                
                case ENOMEM     : throw CSocketWriteException( "Write Exception : No Memory Available" ) ;
                case EFAULT     : throw CSocketWriteException( "Write Exception : Bad buffer pointer passed" ) ;
                
                // AS ASSERTIONS : 
                case EBADF      : throw CSocketWriteException( "Write Exception : Bad socket" ) ;
                case EINVAL     : throw CSocketWriteException( "Write Exception : Invalid argument passed" ) ;
            }
            throw CSocketWriteException( "Read Exception" ) ;
        }
        
        return bytes_written ;
    }

    std::size_t CSocket::read ( std::uint8_t * dst , std::size_t sz , std::initializer_list< EReadFlags > flags )
    {
        if ( ! is_connected() ) 
            throw CSocketLogicException( "Logic Error on read, socket is not connected" )  ;
        
        ::ssize_t bytes_read = ::recv( impl() -> sock_ , dst , sz , FLAGS_eval( flags.begin() , flags.end() ) ) ;
       
        if ( bytes_read <= 0 )
        {
            switch ( errno )
            {
                case ECONNRESET : throw CSocketConnectionException( "Connection on Read Exception : Connection is reset by peer" ) ;
                case ENOMEM     : throw CSocketReadException( "Read Exception : No Memory Available" ) ;
                // AS ASSERTIONS : 
                case EBADF      : throw CSocketReadException( "Read Exception : Bad socket" ) ;
                case EINVAL     : throw CSocketReadException( "Read Exception : Invalid argument passed" ) ;
            }
            
            if ( bytes_read == 0 )
                throw CSocketConnectionException( "Connection on Read Exception : Probably you shutdown the socket for read" ) ;
            
            throw CSocketReadException( "Read Exception" ) ;
        }
        
        return bytes_read ;
    }
    
    std::size_t CSocket::write ( const std::string& str , std::initializer_list< EWriteFlags > flags )
    {
        return write( reinterpret_cast< const std::uint8_t * >( str.c_str() ) , str.length() + 1 , flags ) ;
    }

    void CSocket::connect ( const std::string& addr_str , port_type port ) 
    {            
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;        
        EAddressFamily current_af = impl() -> info_.addr_family_ ;
        
        ip_address addr {} ;
        
        if ( ! set_address( current_af , addr_str.c_str() , port , ( ::sockaddr * ) &addr ) ) 
            throw CSocketConnectException( "Connection Error while loading address" ) ;
        
        if ( ::connect( impl() -> sock_ , ( ::sockaddr * ) &addr , sizeof( ip_address ) ) == - 1 ) 
        {
            switch ( errno )
            {
                
            }
            throw CSocketConnectException( "Connect Exception" ) ;
        }
    
        impl() -> is_connected_ = true ; // commit ;
        impl() -> is_bound_ = true ;
    }
      
    void CSocket::bind ( const std::string& addr_str , port_type port ) 
    { 
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;
        
        EAddressFamily current_af = impl() -> info_.addr_family_ ;
        ip_address addr {} ;
        
        if ( ! set_address( current_af , addr_str.c_str() , port , ( ::sockaddr * ) &addr ) ) 
            throw CSocketBindException( "Bind Error while loading address" ) ;
        
        if ( ::bind( impl() -> sock_ , ( ::sockaddr * ) &addr , sizeof( ip_address ) ) == - 1 ) 
        {
            switch ( errno )
            {
            }
            throw CSocketBindException( "Connect Exception" ) ;
        }
        
        impl() -> is_bound_ = true ; // commit ;
    }

    CSocket CSocket::accept ()  
    {
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;
        
        if ( impl() -> info_.socket_type_ == ESocketType::DATAGRAM ) 
            throw CSocketLogicException( "Logic Error, attempt to accept with datagram socket" ) ;   
        
        auto impl_p = std::unique_ptr< CImplementation >{ new CImplementation } ; // def
            
        ip_address addr {} ;
            
        { ::socklen_t addrlen = sizeof addr ;
            impl_p -> sock_ = ::accept( impl() -> sock_ , ( ::sockaddr * ) &addr , &addrlen ) ; }
            
        if ( impl_p -> sock_ == - 1 ) 
        {
            switch ( errno )
            {
                
            }
            throw CSocketAcceptException( "Accept Exception" ) ;
        }
        
        // noexcept :
        impl_p -> info_ = impl() -> info_ ;
        impl_p -> is_connected_ = true ;
        impl_p -> is_bound_ = true ;
        return CSocket{ std::move( impl_p ) } ;
    }
    
    void CSocket::shutdown ( EShutdown how )
    {
        if ( ! is_connected() ) 
            throw CSocketLogicException( "Shutdown Error : attempt to shutdown socket, that is not connected" ) ;
        
        if ( ::shutdown( impl() -> sock_ , SHUT_translate( how ) ) == - 1 )
            std::cerr << "cerr : " << __func__ << " : shutdown failed" ;
    }
    
              
    void CSocket::write_all ( std::uint8_t * dst , std::size_t bucket_size , std::initializer_list< EWriteFlags > flags ) 
    {
        if ( is_empty() ) 
            throw CSocketLogicException( null_error ) ;
        
        
    }

    /* 

                
    void CSocket::set_option( const ISocketOption& option )
    {
        if ( ! is_empty() ) throw CSocketLogicException( null_error ) ;
        
    }


    void CSocket::read_all ( std::vector< uint8_t >& output_vec )
    {
        
    }
//*/
} /* ip */ 
} /* network */
