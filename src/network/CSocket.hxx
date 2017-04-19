#ifndef NETWORK_IP_CSOCKET_HXX
#define NETWORK_IP_CSOCKET_HXX

#include <memory>
#include <cstring>
#include <vector>

#include "Exception.hxx" 
#include "Types.hxx"
#include "CIPAddress.hxx"

/* todo : const std::string& -> std::string in paramters ( when CIPAddress get changed ) */
// NOTE : the behaviour is implementation-defined if ( for current version ) :
//              * passed addresses contain leading zeros or whitespaces ;
//              * ipv6 socket attempted to accept ipv4 connection ;


namespace network 
{
    std::uint32_t htonl( std::uint32_t ) ;
    std::uint16_t htons( std::uint16_t ) ;
    std::uint32_t ntohl( std::uint32_t ) ;
    std::uint16_t ntohs( std::uint16_t ) ;
    
    namespace ip 
    {
        enum class EShutdown : unsigned short
        {
           READ ,
           WRITE ,
           ALL ,
        ENUM_END
        } ;
        
        struct CSocket final
        {
            CSocket ( EAddressFamily addr_family , ESocketType type , EProtocol proto ) ;
            
            CSocket( const CSocket& sock ) = delete ;
            CSocket& operator = ( const CSocket& sock ) = delete ;
            
            CSocket( CSocket&& sock ) = default ;
            CSocket& operator = ( CSocket&& sock ) = default  ;
            
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
            
            std::size_t write ( const std::uint8_t * src , std::size_t sz ) ;
            std::size_t write ( const std::string& str ) ;
            std::size_t read  ( std::uint8_t * dst , std::size_t sz ) ;

            void write_all ( std::uint8_t * dst , std::size_t bucket_size ) ;
            void read_all ( std::vector< std::uint8_t >& ) ; // optional
            void set_option( const struct ISocketOption& option ) ;
            
            void shutdown ( EShutdown ) ;
            
            private :
                struct CImplementation ;
                
                struct IImplementation
                {
                    virtual ~ IImplementation () = default ;
                } ;
                
                std::unique_ptr< IImplementation > impl_ ;  
                explicit CSocket ( std::unique_ptr< CImplementation > ) noexcept ;
                
                CImplementation * impl () ; 
                const CImplementation * impl () const ; 
        } ;
        
        struct ISocketOption
        {
            friend void CSocket::set_option( const ISocketOption& option ) ;
            
            virtual ~ ISocketOption () = default ;
            protected :
            
               struct COptionParams ;
               
               struct IOptionParams
               {
                   COptionParams * get () ; 
                   const COptionParams * get () const ;
                   virtual ~ IOptionParams() = default ; 
               } ;
               
               virtual std::unique_ptr< IOptionParams > parameters () const = 0 ;
        } ;
        
        struct CReuseAddress final : ISocketOption
        {
            CReuseAddress( bool ) ; 
            private :
               std::unique_ptr< IOptionParams > parameters () const override ;
               bool value_ ; 
        } ;
    } // ip
} // network

// ( http://stackoverflow.com/questions/27609522/extern-template-for-template-parametrized-with-incompete-type )
// inlined functions templates such as std::unique_ptr< ... >::~std::unique_ptr< ... > will be instantiated 
// even if i do prevent it with `extern template`.
// extern template class std::unique_ptr< network::ip::CSocket::CImplementation > ;
// extern template class std::unique_ptr< network::ip::ISocketOption::COptionParams > ;
// 
#endif // NETWORK_IP_CSOCKET_HXX
