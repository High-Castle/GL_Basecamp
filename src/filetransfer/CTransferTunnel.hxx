#include <iostream>
#include <string>
#include <cstdint>

#include "network/CSocket.hxx"

namespace transfer_protocol
{

/* TODO : Encryption */
    
    enum PackageType : std::uint32_t
    { 
        DATA , 
        END_OF_CHUNK ,
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
        enum : unsigned { PACKAGE_DATA_SIZE = 1024 * 2 /* 2KB */ , TIMEOUT_SEC = 5 ,
                          BIND_ATTEMPTS = 3 , CLIENT_QUEUE = 1 , } ;
        
        struct CDataInfoPackagePOD
        {
            CHeaderPOD header ;
            std::uint32_t size ;
            unsigned char eof ;
            unsigned char data [ PACKAGE_DATA_SIZE ] ;
        } ;
        
        struct CJumpPackagePOD 
        {
            CHeaderPOD header ;
            unsigned short port ;
        } ;
        
        void send( std::istream& stream ,
                   const std::string& addr , // address to connect receive to
                   network::ip::port_type port ,
                   const std::size_t chunck_size ) ;
                                
        static void recieve( std::ostream& ,
                             const std::string& addr , // address to start receive on
                             network::ip::port_type port ,
                             const std::size_t chunck_size ,
                             unsigned short delta_port ) ;
        
        private :
            constexpr CTransferTunnel() { static_assert( sizeof( CDataInfoPackagePOD ) == sizeof( std::uint32_t ) + sizeof( CHeaderPOD ) + 1 + PACKAGE_DATA_SIZE , "" };
            
            // static unsigned long check( CHeaderPOD * ) ;
            
            static void encrypt( std::uint8_t * , std::size_t , std::uint8_t * , std::size_t ) ;
            static void decrypt( std::uint8_t * , std::size_t , std::uint8_t * , std::size_t ) ;
            
            // TODO : bool -> size_t ( bytes written )
            static bool recv_to_stream( std::ostream& , network::ip::CSocket& , std::size_t ) ;
            static bool send_from_stream( std::istream& , network::ip::CSocket& , std::size_t ) ;
                                   
            static void to_network( CDataPackagePOD& , std::uint32_t , std::uint8_t ) ;
            static void from_network( const CDataPackagePOD& , std::uint32_t& , std::uint8_t& ) ;
    
            static void to_network( CJumpPackagePOD& jump , network::ip::port_type port ) ;
            static void from_network( const CJumpPackagePOD& jump , network::ip::port_type& port ) ;
     } ;

}
