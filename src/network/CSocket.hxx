#ifndef NETWORK_IP_CSOCKET_HXX
#define NETWORK_IP_CSOCKET_HXX

#include <memory>
#include <cstring>
#include <vector>

#include <initializer_list>

#include "Exception.hxx" 
#include "Types.hxx"
#include "CIPAddress.hxx"

// NOTE : the behaviour is implementation-defined if ( for current version ) :
//              * passed addresses contain leading zeros or whitespaces ;
//              * ipv6 socket attempted to accept ipv4 connection ;


namespace network 
{
    std::uint32_t htonl( std::uint32_t ) ;
    std::uint16_t htons( std::uint16_t ) ;
    std::uint32_t ntohl( std::uint32_t ) ;
    std::uint16_t ntohs( std::uint16_t ) ;
    
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
            DONT_WAIT ,
        ENUM_END      
        } ;
        
        enum class EReadFlags : unsigned short
        {
            DONT_WAIT ,
            WHAIT_ALL ,
        ENUM_END     
        } ;
        
        struct CSocket final
        {
            CSocket ( EAddressFamily addr_family , ESocketType type , EProtocol proto = EProtocol::IP ) ;
            
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
            
            std::size_t write ( const std::uint8_t * src , std::size_t sz , std::initializer_list< EWriteFlags > = {} ) ;
            std::size_t write ( const std::string& str , std::initializer_list< EWriteFlags > = {} ) ;
            std::size_t read  ( std::uint8_t * dst , std::size_t sz , std::initializer_list< EReadFlags > = {} ) ;
            
            // guaranties, that all data will be sent. (Espessially when DONT_WAIT is set)
            // if exception is thrown, ... ( todo documentation )
            void write_all ( std::uint8_t * dst , std::size_t bucket_size , std::initializer_list< EWriteFlags > = {} ) ;
            
            void read_all ( std::vector< std::uint8_t >& , std::initializer_list< EReadFlags > = {} ) ; // optional
            void set_option( const struct ISocketOption& option ) ; 
            
            CSocket duplicate () ; // dup() socket file descriptor ( or WSADuplicateSocket for windows handlers )
            void shutdown ( EShutdown ) ;
            
            private :
                struct CImplementation ;
                
                struct ImplementationBase // TODO : i will somehow cleanup this shame in further updates.
                {
                    virtual ~ ImplementationBase () = default ;
                } ;
                
                std::unique_ptr< ImplementationBase > impl_ ;  
                explicit CSocket ( std::unique_ptr< ImplementationBase > ) noexcept ;
                
                CImplementation * impl () ; 
                const CImplementation * impl () const ; 
        } ;
        
        struct ISocketOption
        {
            friend void CSocket::set_option( const ISocketOption& option ) ;
            
            virtual ~ ISocketOption () = default ;
            protected :
            
               struct COptionParams ;
               
               struct OptionParamsBase
               {
                   COptionParams * get () ; 
                   const COptionParams * get () const ;
                   virtual ~ OptionParamsBase() = default ; 
               } ;
               
               virtual std::unique_ptr< OptionParamsBase > parameters () const = 0 ;
        } ;
        
        // TODO : timeout options
        
        struct CReuseAddress final : ISocketOption
        {
            CReuseAddress( bool ) ; 
            private :
               std::unique_ptr< OptionParamsBase > parameters () const override ;
               bool value_ ; 
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
