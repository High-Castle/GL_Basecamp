#ifndef CTransferTunnel
#define CTransferTunnel

#include <istream>
#include <string>
#include <cstdint>
#include <functional>

#include "CSocket.hxx"

namespace transfer_protocol
{
    struct CTransferException : network::CBasicException  
    { 
         CTransferException( const char * message ) noexcept : 
                CBasicException ( message ) {}
    } ;
    
/* TODO : Encryption */

    

    enum PackageType : std::uint8_t
    { 
        DATA , 
        JUMP_PACKAGE ,
        APPROVE , 
        PACK_CANCEL ,
        TRANSFER_CANCEL ,
    } ;
            
    struct CHeaderPOD
    {
         PackageType type ; 
    } ;
    
    struct CTransferTunnel_TCP final
    {
        enum : unsigned { PACKAGE_DATA_SIZE = 1024 * 1024 /* */ , TIMEOUT_SEC = 5 ,
                          BIND_ATTEMPTS = 3 , ADDITIONAL_TRANSFER_ATTEMPTS = 3 , CLIENT_QUEUE = 1 , } ;
        
        struct CDataPackagePOD
        {
            CHeaderPOD header ;
            unsigned char padding_[ 3 ] ;
            unsigned char size [ 4 ] alignas( 4 ) ; 
            unsigned char checksum [ 4 ] ;
            unsigned char eof ;
            unsigned char data [ PACKAGE_DATA_SIZE ] ;
            // padding
        } ;
                
        // TODO : bool -> size_t ( bytes written )
        static bool recv_amount_to_stream( std::ostream& , network::ip::CSocket& , std::size_t ) ;
        static bool send_amount_from_stream( std::istream& , network::ip::CSocket& , std::size_t ) ;
        
        static void recv_stream( std::ostream& , network::ip::CSocket& ) ;
        static void send_stream( std::istream& , network::ip::CSocket& ) ;
        
        static void to_network( CDataPackagePOD& , std::uint32_t , std::uint8_t ) ;
        static void from_network( const CDataPackagePOD& , std::uint32_t& , std::uint8_t& ) ; 
               
        static void encrypt( std::uint8_t * , std::size_t , std::uint8_t * , std::size_t ) ;
        static void decrypt( std::uint8_t * , std::size_t , std::uint8_t * , std::size_t ) ;
        
        private :
            constexpr CTransferTunnel_TCP () { static_assert( alignof( CDataPackagePOD ) == 4 , "" ) ; }
     } ;
     std::uint32_t calc_checksum_hton( const std::uint8_t * data , std::size_t sz ) ;
}

#endif
