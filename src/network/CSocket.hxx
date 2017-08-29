#ifndef NETWORK_IP_CSOCKET_HXX
#define NETWORK_IP_CSOCKET_HXX

#include <memory>
#include <cstring>
#include <chrono>

#include <initializer_list>

#include "Exception.hxx" 
#include "Types.hxx"
#include "CIPAddress.hxx"

// NOTE : the behaviour is implementation-defined if ( for current version ) :
//              * passed addresses contain leading zeros or whitespaces ;
//              * ipv6 socket attempted to accept ipv4 connection ;

// TODO : select , keepalive , O_ASYNC and way to map signal to each socket. ( select( , , 0 ) in handler or so , as platform specific feature )

// FIXME / TODO (?) : on some Linuxes, SO_RCVTIMEO can affect accept also. The behaviour is implementation-defined in current version, 
//                    just do not set the option on socket, that is inteded to be an acceptor.
//                    ( can't think of any cases where it does matter, but that should be explored more carefully )
//                    one among the ways to standardize this is to use select with timeout.  

// TODO / NOTE : for nonblocking sockets use connect + select ( with optional timeout ), ( EALREADY - previous attempt to connect is not completed yet )

namespace tools
{
        template < class T , class Deleter >
        struct C_UniquePtr final
        {
            C_UniquePtr() noexcept : ptr_( nullptr ) { }
            
            explicit C_UniquePtr( T * ptr ) noexcept 
                : ptr_( ptr )
            {
                static_assert( noexcept( Deleter::delete_ptr( nullptr ) ) , "Deleter::delete_ptr has to be noexcept" ) ;
            }
            
            C_UniquePtr( const C_UniquePtr& ) = delete ;
            C_UniquePtr& operator = ( const C_UniquePtr& holder ) = delete ;
            
            C_UniquePtr( C_UniquePtr&& holder ) noexcept 
                : ptr_( holder.ptr_ ) 
            {
                holder.ptr_ = nullptr ;
            }
            
            C_UniquePtr& operator = ( C_UniquePtr&& holder ) noexcept
            {
                ptr_ = holder.ptr_ ;
                holder.ptr_ = nullptr ;
                return * this ;
            }
            
            ~ C_UniquePtr () { Deleter::delete_ptr( ptr_ ) ; }
            
            void reset( T * ptr = nullptr ) noexcept
            {
                Deleter::delete_ptr( ptr_ ) ;
                ptr_ = ptr ;
            }
            
            T       * operator -> ()       noexcept { return ptr_ ; }
            T const * operator -> () const noexcept { return ptr_ ; }
            
            const T * get () const noexcept { return ptr_ ; }
            T *       get () noexcept { return ptr_ ; }
            
            bool empty() const noexcept { return ptr_ == nullptr ; }
            
            private :
                T * ptr_ ;
        } ;   
}

namespace network 
{
    std::uint32_t hton_l( std::uint32_t ) ;
    std::uint16_t hton_s( std::uint16_t ) ;
    std::uint32_t ntoh_l( std::uint32_t ) ;
    std::uint16_t ntoh_s( std::uint16_t ) ;
    
    std::string resolve( const std::string& ) ; // optional
    

    
    namespace ip 
    {
        enum class EShutdown : unsigned short
        {
           READ ,
           WRITE ,
           ALL ,
        ENUM_END
        } ;
        
        enum class EWriteFlags : unsigned short
        {
            OUT_OF_BAND , // TODO : handlers for SIGURG
        ENUM_END      
        } ;
        
        enum class EReadFlags : unsigned short
        {
            WHAIT_ALL ,
            OUT_OF_BAND , // TODO : handlers for SIGURG
            PEEK ,
        ENUM_END     
        } ;
        
        struct CSocket final
        {
            CSocket ( EAddressFamily addr_family , ESocketType type , EProtocol proto = EProtocol::IP ) ;
            
            CSocket( const CSocket& sock ) = delete ;
            CSocket& operator = ( const CSocket& sock ) = delete ;
            
            CSocket( CSocket&& sock ) = default ;
            CSocket& operator = ( CSocket&& sock ) noexcept ;
            
            ~ CSocket () ;
            
            bool is_empty     () const noexcept ;
            bool is_connected () const noexcept ;
            bool is_bound     () const noexcept ;
                        
            CIPAddress remote_endpoint () const ; /* throws nothing if ! is_empty() */
            CIPAddress bound_address   () const ; /* throws nothing if ! is_empty() */
            
            EProtocol      protocol () const ; /* throws nothing if ! is_empty() */
            EAddressFamily domain   () const ; /* throws nothing if ! is_empty() */
            ESocketType    type     () const ; /* throws nothing if ! is_empty() */     
            
            void bind ( const std::string& address , port_type port ) ;
            
            void listen ( std::size_t queue_length ) ;
            CSocket accept () ;

            void connect ( const std::string& address , port_type port ) ;
            
            std::size_t write ( const unsigned char * src , std::size_t sz , std::initializer_list< EWriteFlags > = {} ) ;
            std::size_t read  ( unsigned char * dst , std::size_t sz , std::initializer_list< EReadFlags > = {} ) ;
            
            // guaranties, that all data will be sent.
            void write_all ( const unsigned char * , std::size_t , std::size_t& , std::initializer_list< EWriteFlags > = {} ) ;
            void write_all ( const std::string& , std::size_t& , std::initializer_list< EWriteFlags > flags = {} ) ;
            
            void set_option( const struct ISocketOption& option ) ,
                 get_option( ISocketOption& option ) const ;
            
            CSocket duplicate () ; // dup() socket file descriptor ( or WSADuplicateSocket for windows handlers )
            void shutdown ( EShutdown ) ;
            
            private :
                struct CImplementation ; 
                
                struct Deleter { static void delete_ptr( CImplementation * ) noexcept ; } ;
                using ImplHolder = tools::C_UniquePtr< CImplementation , Deleter > ;
                
                ImplHolder impl_ ;  

                explicit CSocket ( ImplHolder ) noexcept ;
                
                CImplementation * impl () ; 
                const CImplementation * impl () const ; 
        } ;
        
        struct ISocketOption
        {
            friend void CSocket::set_option( const ISocketOption& option ) ,
                        CSocket::get_option( ISocketOption& option ) const ;
            
            virtual ~ ISocketOption () = default ;
            
            protected :
               
               struct COptionParams ; // inteded to public inherit OptionParamsBase
               
               struct OptionParamsBase
               {
                   virtual ~ OptionParamsBase() = default ; 
               } ;
               
               virtual const COptionParams& parameters () const = 0 ;
               virtual COptionParams& parameters () = 0 ;
        } ;
        
        // TODO : timeout options
        
        struct CReuseAddress final : ISocketOption
        {
            CReuseAddress( bool = false ) ; 
            bool value () const ;
            private :
               struct CImplParams ; // intended to implement abstract part of COptionParams
               const COptionParams& parameters () const override ;
               COptionParams& parameters () override ;
               std::unique_ptr< OptionParamsBase > params_ ; // for most of implementations it has to be polymorphic, so nevermind,
        } ;
        
        // SO_RCVTIMEO and SO_SNDTIMEO
        struct CTimeout : ISocketOption
        {
            std::chrono::microseconds value () const ;
            protected :
               struct CImplParams ;
               CTimeout( std::unique_ptr< OptionParamsBase > ) ;
            private :
               const COptionParams& parameters () const override ;
               COptionParams& parameters () override ;
               std::unique_ptr< OptionParamsBase > params_ ; 
        } ;
        
        struct CWriteTimeout final : CTimeout
        {
            CWriteTimeout( std::chrono::microseconds = {} ) ;
            private :
                struct CImplParams ; // optional
        } ;
        
        struct CReadTimeout final : CTimeout
        {
            CReadTimeout( std::chrono::microseconds = {} ) ; 
            private :
                struct CImplParams ; // optional
        } ;
        
        struct CBlock final : ISocketOption
        {
            CBlock( bool = false ) ; 
            bool value () const ;
            private :
               struct CImplParams ; // intended to implement abstract part of COptionParams
               const COptionParams& parameters () const override ;
               COptionParams& parameters () override ;
               std::unique_ptr< OptionParamsBase > params_ ; 
        } ;
        
        
        
    } // ip
} // network

// ( http://stackoverflow.com/questions/27609522/extern-template-for-template-parametrized-with-incompete-type )
// inlined functions templates such as std::unique_ptr< ... >::~unique_ptr< ... > will be instantiated 
// even if i do prevent it with `extern template`.
// extern template class std::unique_ptr< network::ip::CSocket::CImplementation > ;
// extern template class std::unique_ptr< network::ip::ISocketOption::COptionParams > ;
// 
#endif // NETWORK_IP_CSOCKET_HXX
