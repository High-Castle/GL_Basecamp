#include <iostream>
#include <string>
#include <cstdint>

#include "network/CSocket.hxx"

namespace transfer_protocol
{
/* Inteded to work with UDP, optionally with TCP / SEQPACK ( then order_number doesn't play ) */
/* TODO : crc32 */
/* TODO : Encryption */
    
    struct CTransferTunnel_TCP final
    {
        struct CHeaderPOD
        {
            unsigned char checksum [ 4 ] ;
            unsigned char size [ 8 ]  ;
        } ;
        
        void send( std::istream& stream ,
                   const std::string& addr , // address to connect receive to
                   network::ip::port_type port ,
                   std::size_t file_size ,
                   const std::size_t chunck_size ,
                   const std::size_t port_jump_delta ) ;
                                
        static void recieve( std::ostream& ,
                             const std::string& addr , // address to start receive on
                             network::ip::port_type port ,
                             std::size_t file_size ,
                             std::uint8_t split_percent ) ;
        
        private :
            constexpr CTransferTunnel() { static_assert( alignof( CHeaderPOD ) == 1 , "CDataPackagePOD's alignament is not 1" };
            
            static unsigned long check( CHeaderPOD * ) ;
            
            static void encrypt( uint8_t * , std::size_t , unsigned char * key ) ;
            static void decrypt( uint8_t * , std::size_t , unsigned char * key ) ;
            
            static void to_network( CHeaderPOD * ) ;
            static void from_network( CHeaderPOD * ) ;
    } ;

}






 /*
        
        
        enum PackageType : unsigned char { DATA , APPROVE ,  , PACK_CANCEL } ;
            
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
        constexpr void alignment_assertion ( ) { static_assert( alignof( CDataPackagePOD ) == 1 , "CDataPackagePOD's alignament is not 1" ) ; }
        */
