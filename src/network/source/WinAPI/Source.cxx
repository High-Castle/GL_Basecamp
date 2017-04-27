#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <cstring>
#include <memory>
#include <vector>
#include <string>

#include <initializer_list>
#include <utility>
#include <exception>
#include <chrono>

#include "Exception.hxx"
#include "CIPAddress.hxx"
#include "CSocket.hxx"

#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>

#if defined(_MSC_VER)

#include <BaseTsd.h>
#pragma comment(lib, "Ws2_32.lib")

using ssize_t = SSIZE_T ;

#else
    #error only VS compiler is supported for now on Windows
#endif

using sa_family_t = unsigned short ;

enum 
{
    SHUT_RD = SD_RECEIVE , 
    SHUT_WR = SD_SEND , 
    SHUT_RDWR = SD_BOTH ,
} ;

// TODO : PROPER EXCEPTIONS.! 

using std::chrono::microseconds ;
using std::chrono::milliseconds ;
using std::chrono::seconds      ;
using std::chrono::minutes      ; 

namespace
{
    struct WSA_Manager 
    {
        WSA_Manager() = delete ;
        
        static void attempt_init () 
        { 
            // TODO : sync ( with mutex )
            WSADATA data ;
            if ( ::WSAStartup( MAKEWORD( 2,2 ) , &data ) == SOCKET_ERROR )
                throw network::ip::CSocketInitException( "Socket Init Error : WSAStartup Failed" ) ;
            ++ clients() ;
        }
        
        static void attempt_free () noexcept
        {
            // TODO : sync
            -- clients() ;
            if ( ! clients() )
                ::WSACleanup() ;
        }
        
        private :
            static std::size_t& clients () noexcept
            {
                static std::size_t clients_ = 0 ; 
                return clients_ ;
            }
    } ;
}

namespace network
{
    namespace ip
    {
        namespace 
        {
            using sock_handle_type = SOCKET ;
            using ip_address = ::sockaddr_in6 ;
            
            template < class EnT , class Type = typename std::underlying_type< EnT >::type > 
            constexpr Type to_int( EnT en ) { return static_cast< Type >( en ) ; }
            
            template < class T , std::size_t N >
            constexpr std::size_t size( T(&)[ N ] ) { return N ; }
            
            ::sa_family_t AF_translate ( EAddressFamily af )
            {
                static ::sa_family_t table [] { AF_INET , AF_INET6 } ;
                static_assert( to_int( EAddressFamily::ENUM_END ) == size( table ) , "NOT ALL ADDRESS FAMILIES ARE IMPLEMENTED" ) ;
                return table[ to_int( af ) ] ;
            }
            
            int TYPE_translate ( ESocketType type )
            {
                static int table [] { SOCK_STREAM , SOCK_DGRAM , SOCK_SEQPACKET } ;
                static_assert( to_int( ESocketType::ENUM_END ) == size( table ) , "NOT ALL SOCKET TYPES ARE IMPLEMENTED" ) ;
                return table[ to_int( type ) ] ;
            }
            
            int PROTO_translate ( EProtocol proto )
            {
                static int table [] { IPPROTO_IP , IPPROTO_UDP , IPPROTO_TCP } ;
                static_assert( to_int( EProtocol::ENUM_END ) == size( table ) , "NOT ALL PROTOCOLS ARE IMPLEMENTED" ) ;
                return table[ to_int( proto ) ] ;
            }
            
            int SHUT_translate ( EShutdown how )
            {
                static int table [] { SHUT_RD , SHUT_WR , SHUT_RDWR } ;
                static_assert( to_int( EShutdown::ENUM_END ) == size( table ) , "NOT ALL EShutdown ARE IMPLEMENTED" ) ;
                return table[ to_int( how ) ] ;
            }
            
            template < class EnT > int FLAG_translate ( EnT flg ) = delete ;
            
            template <> int FLAG_translate <> ( EWriteFlags flg ) {
                static int table [] { MSG_OOB } ;
                static_assert( to_int( EWriteFlags::ENUM_END ) == size( table ) , "NOT ALL EWriteFlags ARE IMPLEMENTED" ) ;
                return table[ to_int( flg ) ] ;
            }
            
            template <> int FLAG_translate <> ( EReadFlags flg ) {
                static int table [] { MSG_WAITALL , MSG_OOB , MSG_PEEK } ;
                static_assert( to_int( EReadFlags::ENUM_END ) == size( table ) , "NOT ALL EReadFlags ARE IMPLEMENTED" ) ;
                return table[ to_int( flg ) ] ;
            }
            
            template < class FlagsIt > int FLAGS_eval ( FlagsIt from , FlagsIt to ) {
                int acc {} ;
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

namespace 
{
    template< class T , class U , class DU >
    std::unique_ptr< T , DU > unique_cast( std::unique_ptr< U , DU > ptr ) noexcept
    {
        return std::unique_ptr< T , DU >{ static_cast< T * >( ptr.release() ) } ;
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
{  
    CImplementation () : is_connected_{ false } , is_bound_{ false } , is_blocking_{ true }
    { 
    }
    
    sock_handle_type sock_ ;   
    struct SocketInfo
    { 
        EAddressFamily addr_family_ ; 
        ESocketType    socket_type_ ; 
        EProtocol      protocol_    ; 
    } info_ ;
    std::uint8_t is_connected_ : 1 , is_bound_ : 1 , is_blocking_ : 1 ; 
} ;

network::ip::CSocket::CImplementation * network::ip::CSocket::impl ()
{
    return impl_.get() ;
}

const network::ip::CSocket::CImplementation * network::ip::CSocket::impl () const
{
    return impl_.get() ;
}

void network::ip::CSocket::Deleter::delete_ptr( CImplementation * ptr ) noexcept // static member
{
    delete ptr ;
}

namespace network { 
namespace ip {

// options :   
    
    namespace 
    {
        void setsockopt_option( sock_handle_type sock , int level , int optname ,
                                const void * optval , ::socklen_t optlen )
        {
            int result = ::setsockopt( sock , level , optname , reinterpret_cast< const char * >( optval ) , optlen ) ;
            if ( result == SOCKET_ERROR )
            {
                switch ( WSAGetLastError() )
                {
                    // TODO
                }
                throw CSocketOptionException( "Set Option Error" ) ;
            }
        }
        
        void getsockopt_option( sock_handle_type sock , int level , int optname ,
                                void * optval , ::socklen_t& in_out_size )
        {
            int result = ::getsockopt ( sock , level , optname , reinterpret_cast< char * >( optval ) , &in_out_size ) ;
            if ( result == SOCKET_ERROR )
            {
                switch ( WSAGetLastError() )
                {
                    // TODO
                }
                throw CSocketOptionException( "Get Option Error" ) ;
            }                           
        }

        void perform_block ( sock_handle_type sock , u_long mode )
        {
            mode = ! mode ;
            int result = ::ioctlsocket( sock , FIONBIO , &mode ) ; 
            if ( result == SOCKET_ERROR ) 
            {
                switch ( WSAGetLastError() )
                {
                   // TODO 
                }
                throw CSocketOptionException( "Option Error" ) ;
            }
        }
    }
    
    struct ISocketOption::COptionParams : OptionParamsBase
    {
        virtual void set ( sock_handle_type , bool& ) const = 0 ; 
        virtual void get ( sock_handle_type , bool ) = 0 ;
    } ;
    
    struct CReuseAddress::CImplParams final : COptionParams
    {
        CImplParams ( int value ) : value_( value ) {}
        void set( sock_handle_type sock , bool& ) const final {  
            setsockopt_option( sock , SOL_SOCKET , SO_REUSEADDR , &value_ , sizeof value_ ) ; 
        }
        void get( sock_handle_type sock , bool ) final { 
            ::socklen_t in_out_size = sizeof value_ ;
            getsockopt_option( sock , SOL_SOCKET , SO_REUSEADDR , &value_ , in_out_size ) ; 
        }
        bool value() const { return value_ ; }
        private :
            int value_ ;
    } ;
    
    CReuseAddress::CReuseAddress( bool value ) 
        :  params_ { new CImplParams{ value } }
    {
    }
    
    bool CReuseAddress::value () const { return static_cast< const CImplParams& >( parameters() ).value() ; }
    auto CReuseAddress::parameters () const -> const COptionParams& { return static_cast< const CImplParams& >( * params_ ) ; }
    auto CReuseAddress::parameters () -> COptionParams& { return static_cast< CImplParams& >( * params_ ) ; }
    
    struct CBlock::CImplParams final : COptionParams
    {
        CImplParams ( int value ) : value_( value ) {}
        
        void set( sock_handle_type sock , bool& is_blck ) const final 
        { 
            perform_block( sock , value_ ) ;
            is_blck = value_ ; // FIXME : semantics error in general. ( but it doesn't matter in this case , at least for now )
        }
        
        void get( sock_handle_type , bool is_blck ) final 
        { 
            value_ = is_blck ;
        }
        
        bool value() const { return value_ ; }
        private :
            int value_ ;
    } ;
    
    CBlock::CBlock( bool value ) 
        :  params_ { new CImplParams{ value } }
    {
    }
    
    bool CBlock::value () const { return static_cast< const CImplParams& >( parameters() ).value() ; }
    auto CBlock::parameters () const -> const COptionParams& { return static_cast< const CImplParams& >( * params_ ) ; }
    auto CBlock::parameters () -> COptionParams& { return static_cast< CImplParams& >( * params_ ) ; }
    
    struct CTimeout::CImplParams final : COptionParams
    {
        CImplParams ( int op , int level , microseconds value ) 
            : level_( level ) , operation_( op )  
              {
                  value_ = std::chrono::duration_cast< milliseconds >( value ).count() ;
              }
        void set( sock_handle_type sock , bool& is_blck ) const final {  
            setsockopt_option( sock , level_ , operation_ , &value_ , sizeof value_ ) ; 
        }
        void get( sock_handle_type sock , bool ) final { 
            ::socklen_t in_out_size = sizeof value_ ;
            getsockopt_option( sock , level_ , operation_ , &value_ , in_out_size ) ; 
        } 
        
        int value() const { return value_ ; }
        private :
            int value_ ;
            const int level_ , operation_ ;
    } ;
    
    CTimeout::CTimeout( std::unique_ptr< OptionParamsBase > impl ) 
        : params_ { std::move( impl ) }
    {
    }
    
    microseconds CTimeout::value () const 
    { 
        auto delta = static_cast< const CImplParams& >( parameters() ).value() ;
        return microseconds{ delta } ; 
    }
    auto CTimeout::parameters () const -> const COptionParams& { return static_cast< const CImplParams& >( * params_ ) ; }
    auto CTimeout::parameters () -> COptionParams& { return static_cast< CImplParams& >( * params_ ) ; }
    
    // Read / Write Timeouts :
    
    CReadTimeout::CReadTimeout( microseconds value ) :
        CTimeout { std::unique_ptr< OptionParamsBase >
            ( new CTimeout::CImplParams{ SO_RCVTIMEO , SOL_SOCKET , value } ) }
    {
    }
    
    CWriteTimeout::CWriteTimeout( microseconds value ) :
        CTimeout { std::unique_ptr< OptionParamsBase >
            ( new CTimeout::CImplParams{ SO_SNDTIMEO , SOL_SOCKET , value } ) }
    {
    }
    
    void CSocket::get_option( ISocketOption& option ) const
    {
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;
        option.parameters().get( impl() -> sock_ , impl() -> is_blocking_ ) ;       
    }
    
    void CSocket::set_option( const ISocketOption& option )
    {
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;
        bool is_blck = impl() -> is_blocking_ ; 
        option.parameters().set( impl() -> sock_ , is_blck ) ;
        impl() -> is_blocking_ = is_blck ; // if blocking is changed
    }
    
// CSocket :
    CSocket::CSocket ( EAddressFamily addr_family , ESocketType type , EProtocol proto ) 
    {
        WSA_Manager::attempt_init() ;
        impl_ = ImplHolder{ new CImplementation } ;
        impl() -> sock_ = ::socket( AF_translate( addr_family ) , TYPE_translate( type ) , PROTO_translate( proto ) ) ;
        
        if ( impl() -> sock_ == SOCKET_ERROR ) throw CSocketInitException( "Socket Init Error" ) ;
        
        impl() -> info_ = CImplementation::SocketInfo{ addr_family , type , proto } ;
    }
    
    CSocket::CSocket ( ImplHolder impl_ptr ) noexcept
        : impl_( std::move( impl_ptr ) )
        {
            WSA_Manager::attempt_init() ;
        }
    
    CSocket& CSocket::operator = ( CSocket&& sock ) noexcept
    {
        if ( ! is_empty() && ::closesocket( impl() -> sock_ ) )
            std::cerr << __func__ << " : " << "attempt to close socket is not successful" ;
        WSA_Manager::attempt_free () ;
        impl_ = std::move( sock.impl_ ) ;
    }
    
    CSocket:: ~ CSocket () 
    { 
        if ( ! is_empty() && ::closesocket( impl() -> sock_ ) )
            std::cerr << __func__ << " : " << "attempt to close socket is not successful" ;
        WSA_Manager::attempt_free () ;
    }  
    
    bool CSocket::is_empty () const noexcept 
    { 
        return impl_.empty() ; 
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
        
        if ( ::getpeername( impl() -> sock_ , ( ::sockaddr * ) &addr ,  &addrlen ) == SOCKET_ERROR )
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
        
        if ( ::getsockname( impl() -> sock_ , ( ::sockaddr * ) &addr ,  &addrlen ) == SOCKET_ERROR )
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
        
        if ( ::listen( impl() -> sock_ , queue_length ) == SOCKET_ERROR ) 
        {
            switch ( WSAGetLastError() )
            {
                // TODO
            }
            throw CSocketListenException( "Listen Exception" ) ;
        }
    }
    
    namespace 
    {
        std::size_t no_checks_write ( sock_handle_type sock , const std::uint8_t * src , std::size_t sz , int flg_mask )
        {
            ::ssize_t bytes_written = ::send( sock , reinterpret_cast< const char * >( src ) , sz , flg_mask ) ;
        
            if ( bytes_written == SOCKET_ERROR ) {
                switch ( WSAGetLastError() )
                {
                    case WSAEWOULDBLOCK :
                    case WSAETIMEDOUT :
                        throw CSocketWriteAttemptException( "Write Exception : Buffer is full ( the socket is non-blocking ) or timeout expired" ) ;
                    case WSAECONNRESET : 
                        throw CSocketConnectionException( "Connection on Write Exception : Connection is reset by peer" ) ;
                    case WSA_NOT_ENOUGH_MEMORY : 
                        throw CSocketWriteException( "Write Exception : No Memory Available" ) ;
                    
                    case WSAEFAULT     : throw CSocketWriteException( "Write Exception : Bad buffer pointer passed" ) ;
                    case WSAEBADF      : throw CSocketWriteException( "Write Exception : Bad socket" ) ;
                    case WSAEINVAL     : throw CSocketWriteException( "Write Exception : Invalid argument passed" ) ;
                    case WSAEOPNOTSUPP : throw CSocketWriteException( "Write Exception : Operation is not supported" ) ;
                }
                throw CSocketWriteException( "Write Exception" ) ;
            }
            return bytes_written ;
        }    
        
        std::size_t no_checks_read ( sock_handle_type sock , std::uint8_t * dst , std::size_t sz , int flg_mask )
        {
            ::ssize_t bytes_read = ::recv( sock , reinterpret_cast< char * >( dst ) , sz , flg_mask ) ;
        
            if ( bytes_read == SOCKET_ERROR )
            {
                int e ;
                switch ( e = WSAGetLastError() )
                {
                    case WSAEWOULDBLOCK :
                    case WSAETIMEDOUT :
                        throw CSocketReadAttemptException( "Read Exception : Nothing to read ( the socket is non-blocking ) or timeout expired" ) ;
                        
                    case WSAECONNRESET  : throw CSocketConnectionException( "Connection on Read Exception : Connection is reset by peer" ) ;
                    case WSAESHUTDOWN   : throw CSocketReadException( "Read Exception : socket was shtdown" ) ;
                    
                    case WSA_NOT_ENOUGH_MEMORY : 
                        throw CSocketReadException( "Read Exception : No Memory Available" ) ;
                    
                    case WSAEBADF      : throw CSocketReadException( "Read Exception : Bad socket" ) ;
                    case WSAEINVAL     : throw CSocketReadException( "Read Exception : Invalid argument passed" ) ;
                    case WSAEOPNOTSUPP : throw CSocketWriteException( "Write Exception : Operation is not supported" ) ;
                }
                
                throw CSocketReadException( ( "Read Exception" + std::to_string( e ) ).c_str() ) ;
            }
            
            return bytes_read ;
        }
    }
    
    std::size_t CSocket::write ( const std::uint8_t * src , std::size_t sz , std::initializer_list< EWriteFlags > flags )
    {
        if ( ! is_connected() ) 
            throw CSocketLogicException( "Logic Error on write, socket is not connected" ) ;
        int flg_mask = FLAGS_eval( flags.begin() , flags.end() ) ;
        return no_checks_write( impl() -> sock_ , src , sz , flg_mask ) ;
    }

    std::size_t CSocket::read ( std::uint8_t * dst , std::size_t sz , std::initializer_list< EReadFlags > flags )
    {
        if ( ! is_bound() ) 
            throw CSocketLogicException( "Logic Error on read, socket is not bound" )  ;
        int flg_mask = FLAGS_eval( flags.begin() , flags.end() ) ;
        return no_checks_read( impl() -> sock_ , dst , sz , flg_mask ) ;
    }

    void CSocket::connect ( const std::string& addr_str , port_type port ) 
    {            
        if ( is_empty() ) throw CSocketLogicException( null_error ) ;        
        EAddressFamily current_af = impl() -> info_.addr_family_ ;
        
        ip_address addr {} ;
        
        if ( ! set_address( current_af , addr_str.c_str() , port , ( ::sockaddr * ) &addr ) ) 
            throw CSocketConnectException( "Connection Error while loading address" ) ;
        
        if ( ::connect( impl() -> sock_ , ( ::sockaddr * ) &addr , sizeof( ip_address ) ) == SOCKET_ERROR ) 
        {
            switch (  WSAGetLastError()  )
            {
                // TODO
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
        
        if ( ::bind( impl() -> sock_ , ( ::sockaddr * ) &addr , sizeof( ip_address ) ) == SOCKET_ERROR ) 
        {
            switch (  WSAGetLastError()  )
            {
                // TODO
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
        
        auto impl_p = ImplHolder{ new CImplementation } ; // def
            
        ip_address addr {} ;
            
        { ::socklen_t addrlen = sizeof addr ;
            impl_p -> sock_ = ::accept( impl() -> sock_ , ( ::sockaddr * ) &addr , &addrlen ) ; }
            
        if ( impl_p -> sock_ == SOCKET_ERROR ) 
        {
            switch (  WSAGetLastError()  )
            {
                // TODO
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
        
        if ( ::shutdown( impl() -> sock_ , SHUT_translate( how ) ) == SOCKET_ERROR )
            std::cerr << "cerr : " << __func__ << " : shutdown failed" ;
    }
    
    void CSocket::write_all ( const std::uint8_t * src , const std::size_t buff_size , std::size_t& written , std::initializer_list< EWriteFlags > flags )
    {
        if ( ! is_connected() ) 
            throw CSocketLogicException( null_error ) ;
        
        int flg_mask = FLAGS_eval( flags.begin() , flags.end() ) ;
        written = 0 ;
        // if ( flg_mask & MSG_DONTWAIT ) 
        
        do {
            written += no_checks_write( impl() -> sock_ , src + written , buff_size - written , flg_mask ) ;    
        } while ( written < buff_size ) ;
    }
    
  
    void CSocket::write_all ( const std::string& str , std::size_t& written , std::initializer_list< EWriteFlags > flags )
    {
        return write_all( reinterpret_cast< const std::uint8_t * >( str.c_str() ) , str.length() + 1 , written , flags ) ;
    }

} /* ip */ 
} /* network */
