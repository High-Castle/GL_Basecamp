#include <iostream>
#include <string>
#include <cintdef>

#include "network/CSocket.hxx"

namespace transfer_protocol
{
/* Inteded to work with UDP, optionally with TCP / SEQPACK ( then order_number doesn't play ) */
/* todo : crc32 */
    struct CFileTransfer final
    {
        static const size_t package_data_size = 1024 ;
        enum PackageType : unsigned char { DATA , APPROVE , PACK_CANCEL } ;
            
        struct CHeaderPOD
        {
            PackageType type ; 
        } ;
        
        struct CDataPackagePOD
        {
            struct CDataInfoPOD 
            {
                unsigned char checksum     [ 4 ] , // htonl
			      data_size    [ 4 ] , 
			      order_number [ 4 ] ; 
            } ;
            
            CHeaderPOD   pack_header ;
            CDataInfoPOD data_header ;
            
            unsigned char data [ package_data_size ] ;
        } ;
        
        
        static send( std::istream& , const std::string& addr , network::ip::port_type port ,
		      std::size_t file_size , std::uint8_t split_percent ) ;
    
        static recieve( std::ostream& ,   , network::ip::port_type port ) ;
        
        public :
            CFileTransfer() ;
    } ;
}
