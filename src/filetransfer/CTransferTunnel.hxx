#ifndef CTransferTunnel
#define CTransferTunnel

#include <istream>
#include <string>
#include <cstdint>
#include <functional>
#include <limits>

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
        enum : unsigned { TIMEOUT_SEC = 5 , ADDITIONAL_TRANSFER_ATTEMPTS = 3 } ;
        
        struct CDataHeaderPOD  // | h | p | p | p | s | s | s | s | c | c | c | c | e | p | p | p |     
        {
            CHeaderPOD header ;
            // unsigned char padding0_[ 3 ] ;
            std::uint32_t size , checksum ;
            std::uint8_t eof ;
            // unsigned char padding1_[ 3 ] ;
        } ;
        
        // TODO : bool -> size_t ( bytes written )
        static bool recv_amount_to_stream( std::ostream& , network::ip::CSocket& , 
                                           std::size_t , std::size_t , unsigned = ADDITIONAL_TRANSFER_ATTEMPTS ) ;
        static bool send_amount_from_stream( std::istream& , network::ip::CSocket& , 
                                             std::size_t , std::size_t , unsigned = ADDITIONAL_TRANSFER_ATTEMPTS ) ;
        
        static void recv_stream( std::ostream& , network::ip::CSocket& , std::size_t , unsigned = ADDITIONAL_TRANSFER_ATTEMPTS ) ;
        static void send_stream( std::istream& , network::ip::CSocket& , std::size_t , unsigned = ADDITIONAL_TRANSFER_ATTEMPTS ) ;
        
        static void to_network( CDataHeaderPOD * , std::uint32_t , std::uint8_t ) ;
        static void from_network( const CDataHeaderPOD * , std::uint32_t& , std::uint8_t& ) ; 
               
        static void encrypt( unsigned char * , std::size_t , unsigned char * , std::size_t ) ;
        static void decrypt( unsigned char * , std::size_t , unsigned char * , std::size_t ) ;
        
        private :
            constexpr CTransferTunnel_TCP () { static_assert( alignof( CDataHeaderPOD ) == 4 
                                                && sizeof( std::uint32_t ) == 4 
                                                && sizeof( CDataHeaderPOD ) == 16
                                                && alignof( std::max_align_t ) >= 4 , "" ) ; }
     } ;
     std::uint32_t calc_checksum_hton( const unsigned char * data , std::size_t sz ) ;
}

#endif
