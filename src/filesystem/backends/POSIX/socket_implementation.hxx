#ifndef SOCKET_IMPLEMENTATION_HXX
#define SOCKET_IMPLEMENTATION_HXX

// in .cxx file

#include "newtork/backends/impl.hxx"

/* note : noexcept : basic_string( basic_string&& other ) noexcept ; (8) (since C++11) */
/* note : not noexcept : basic_string( const Allocator& = Allocator() ) ; */
/* todo : all to utf8 */

class CBasicException : public std::exception
{
    public :
        const char * what () const noexcept final { return message_.c_str() ; }
        const char * native_what () const noexcept { return native_message_.c_str() ; }
    
    protected :
        
        CInterpretException( std::string message , std::string native_message ) 
            : message_( std::move( message ) ) , 
              native_message_( std::move( native_message ) )
        { }
        
        CInterpretException( std::string message ) :
            : message_( std::move( message ) )
        { }
    
    private :
        const std::string message_ , native_message_ ;
} ;


namespace network /* Another file */
{
    struct CSocketException : CBasicException
    {
        using CBasicException::CBasicException ;
    } ;
    
    struct CIPAddress 
    {
        CIPAddress ( std::string addr , int port ) noexcept
            : addr_( std::move( addr ) ) , 
              port_( port )      
        { }
        
        /* return const ref or val ? */
        std::string address () const { return addr_ ; }
        int         port    () const { return port_ ; }
        bool        empty   () const { return addr_.empty() ; }
        private :
            std::string addr_ ;
            int         port_ ;
    } ;
}

// to .cxx file

namespace network
{    
    using impl::EAddressFamily ; // newtork::CSocket sock ( newtork::IPv4 , newtork::STREAM , network::IP ) ;
    using impl::ESocketType    ;
    using impl::EProtocol      ;
    using impl::ESocketOption  ;
    
    struct CSocket final 
    {
        /* public methods : */
        CSocket ( EAddressFamily addr_family , ESocketType type , EProtocol proto ) 
            : info_{ addr_family , type , proto }
        {
            socket_descriptor_ = impl::socket( addr_family , type , proto ) ;
            if ( impl::is_descriptor_valid( socket_descriptor_ ) )
                throw CSocketException( "Socket initialization failed" ) ;
        }
        
        CSocket( const CSocket& sock ) = delete ;
        CSocket& operator = ( const CSocket& sock ) = delete ;
        
        CSocket( CSocket&& sock ) noexcept 
            : socket_descriptor_( sock.socket_descriptor_ ) , 
              addr_( std::move( sock.addr_ ) ) ,
              info_( sock.info_ )
        {
            set_released_( sock.socket_descriptor_ ) ;
            assert( ! sock.is_valid() ) ;
        }
        
        CSocket& operator = ( CSocket&& sock ) noexcept 
        {
            addr_ = std::move( sock.addr_ ) ;
            info_ = sock_.info_ ;
            if ( is_valid() ) impl::close( socket_descriptor_ ) ;
            socket_descriptor_ = sock.socket_descriptor_ ;
            impl::set_descriptor_released( sock.socket_descriptor_ ) ;
            assert( ! sock.is_valid() ) ;
        }
        
        ~ CSocket () 
        { 
            if ( ! is_valid() ) return ; 
            if ( impl::close( socket_descriptor_ ) == impl::ERROR_CODE ) 
                std::cerr << "attempt to close socket is not successul" ;
        }    
        
        bool is_valid     () const noexcept { return impl::is_descriptor_valid( socket_descriptor_ ) ; }
        bool is_connected () const noexcept { return ! addr_.connected_.empty() ; }
        bool is_binded    () const noexcept { return ! addr_.binded_.empty() ; }
        
        EProtocol       protocol () const noexcept { return info_.protocol_    ; }
        EAddressFamily  domain   () const noexcept { return info_.addr_family_ ; }
        ESocketType     type     () const noexcept { return info_.socket_type_ ; }
        
        /* return const ref or val ? */
        CIPAddress connected_to () const { return addr_.connected_ ; }
        CIPAddress binded_to    () const { return addr_.binded_    ; }
        
        
        /* berckley socket interface : */
        
        void bind ( std::string address , int port ) 
        { 
            impl::bind( socket_descriptor_ , address.c_str() , port ) ;
            // noexcept :
            addr_.binded_ = CIPAddress{ std::move( address ) , port } ; 
        }
        
        CSocket accept() 
        {
            // pair, first - descriptor , second - address
            CIPAddress empty_addr ; // stands here to provide exception safe end of the function
            auto accepted_socket = impl::accept( socket_descriptor_ , &addr ) ;
            // noexcept :
            return CSocket( desc , accepted_socket.first , socket.second , std::move( empty_addr ) , info_ ) ;
        }
        
        
        void connect ( std::string addr , int port )
        {
            impl::connect( socket_descriptor_ , addr.c_str() , port ) ;
            // noexcept :
            addr_.connected_ = CIPAddress{ std::move( addr ) , port } ;    
        }

        void listen ( std::size_t queue_length )
        {
            impl::listen( socket_descriptor_ , queue_length ) ;
        }
        
        void write ( std::uint8_t * src , std::size_t sz )
        {
            impl::write( socket_descriptor_ , src , sz , impl::BLOCK ) ;
        }
        
        void write ( const std::string& str )
        {
            impl::write( socket_descriptor_ , str.c_str() , str.length() + 1 , impl::BLOCK ) ;
        }
        
        void read ( std::uint8_t * dst , std::size_t sz )
        {
            impl::read( socket_descriptor_ , dst , sz , impl::BLOCK ) ;
        }
        
        std::string readall ()
        {
            
        }
        
        void shutdown ( EShutdown how )
        {
            impl::shutdown( socket_descriptor_ , option ) ;
        {
        
        void set_option( ESocketOption option )
        {
            impl::set_option( socket_descriptor_ , option ) ;
        }
            
        /* todo : async read / writes */
        
        private :
            impl::descriptor_type socket_descriptor_ ;
            
            struct SockInfo 
            { 
                EAddressFamily addr_family_ ; 
                ESocketType    socket_type_ ; 
                EProtocol      protocol_    ; 
            } info_ ;
            
            struct AddressPair { 
                CIPAddress connected_ , binded_ ; 
                AddressPair() = default ;
                AddressPair( CIPAddress connected , CIPAddress binded ) 
                    : connected_( std::move( connected ) ) , binded_( std::move( binded ) ) { }
            } addr_ ;
            
            CSocket ( impl::socket_descriptor_type desc , CIPAddress connected_to , CIPAddress binded_to ,  SockInfo sock_info ) 
                : addr_( connected_to , binded_to ) , info_( sock_info )
            { }
            

    } ;
}

#endif // SOCKET_IMPLEMENTATION_HXX
