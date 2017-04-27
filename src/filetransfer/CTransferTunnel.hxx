#ifndef CTransferTunnel
#define CTransferTunnel

#include <istream>
#include <string>
#include <cstdint>

#include "CSocket.hxx"

namespace transfer_protocol
{

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
        enum : unsigned { PACKAGE_DATA_SIZE = 1024 * 2 /* 2KB */ , TIMEOUT_SEC = 5 ,
                          BIND_ATTEMPTS = 3 , CLIENT_QUEUE = 1 , } ;
        
        struct CDataPackagePOD
        {
            CHeaderPOD header ;
            unsigned char padding_[ 3 ] ;
            unsigned char size [ 4 ] alignas( 4 ) ; 
            unsigned char eof ;
            unsigned char data [ PACKAGE_DATA_SIZE ] ;
            // padding
        } ;
        
        struct CJumpPackagePOD 
        {
            CHeaderPOD header ;
            unsigned char padding_[ 1 ] ;
            unsigned char port [ 2 ] alignas( 2 ) ;
        } ;
        
        void send( std::istream& stream ,
                   const std::string& addr , // address to connect receive to
                   network::ip::port_type port ,
                   const std::size_t chunck_size ) ;
                                
        static void receive( std::ostream& ,
                             const std::string& addr , // address to start receive on
                             network::ip::port_type port ,
                             const std::size_t chunck_size ,
                             unsigned short delta_port ) ;
                    // TODO : bool -> size_t ( bytes written )
        static bool recv_to_stream( std::ostream& , network::ip::CSocket& , std::size_t ) ;
        static bool send_from_stream( std::istream& , network::ip::CSocket& , std::size_t ) ;
        
        private :
            constexpr CTransferTunnel_TCP () 
            { static_assert( alignof( CDataPackagePOD ) == 4 , "" ) ; } ;
            
  
            static void encrypt( std::uint8_t * , std::size_t , std::uint8_t * , std::size_t ) ;
            static void decrypt( std::uint8_t * , std::size_t , std::uint8_t * , std::size_t ) ;
            

                                   
            static void to_network( CDataPackagePOD& , std::uint32_t , std::uint8_t ) ;
            static void from_network( const CDataPackagePOD& , std::uint32_t& , std::uint8_t& ) ;
    
            static void to_network( CJumpPackagePOD& jump , network::ip::port_type port ) ;
            static void from_network( const CJumpPackagePOD& jump , network::ip::port_type& port ) ;
     } ;
}

#endif
