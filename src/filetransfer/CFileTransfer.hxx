#include <iostream>
#include <string>
#include <cstdint>

#include "network/CSocket.hxx"

namespace transfer_protocol
{
/* Inteded to work with UDP, optionally with TCP / SEQPACK ( then order_number doesn't play ) */
/* TODO : crc32 */
/* TODO : Encryption */
    struct CTransferTunnel final
    {
        enum : std::size_t { PACKAGE_DATA_SIZE = 1024 } ;
        
        enum PackageType : unsigned char { DATA , APPROVE , PACK_CANCEL } ;
            
        struct CHeaderPOD
        {
            PackageType type ; 
        } ;
        
        struct CDataPackagePOD
        {
            struct CDataInfoPOD 
            {
                  unsigned char checksum [ 4 ] , // htonl
			      data_size    [ 4 ] , 
			      order_number [ 4 ] ; 
            } ;
            
            CHeaderPOD   pack_header ;
            CDataInfoPOD data_header ;
            
            unsigned char data [ PACKAGE_DATA_SIZE ] ;
        } ;
        
        CTransferTunnel() = delete ;
        
        static void send( std::istream& , const std::string& addr , network::ip::port_type port , std::size_t file_size , std::uint8_t split_percent ) ;
        static void recieve( std::ostream& , const std::string& addr , network::ip::port_type port , std::size_t file_size , std::uint8_t split_percent ) ;
        
        private :
            static bool check( CDataPackagePOD * ) ;
            //static void encrypt( CDataPackagePOD * , unsigned char * key ) ;
            //static void decrypt( CDataPackagePOD * , unsigned char * key ) ;
            static void to_network( CDataPackagePOD * ) ;
            static void from_network( CDataPackagePOD * ) ;
            constexpr void alignment_assertion ( ) { static_assert( alignof( CDataPackagePOD ) == 1 , "CDataPackagePOD's alignament is not 1" ) ; }
    } ;
}
