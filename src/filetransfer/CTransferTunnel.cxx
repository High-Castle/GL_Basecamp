#include <chrono>
#include <exception>

#include "network/Types.hxx"
#include "network/CSocket.hxx"

namespace transfer_protocol 
{
    namespace
    {
        // returns false if not all requested data_sz was written to a socket
        bool send_data_from_stream( std::istream& stream , 
                                    network::ip::CSocket& peer ,
                                    std::size_t data_size )
        {
            using CTransferTunnel::PACKAGE_DATA_SIZE ;
            
            unsigned long num_of_full_packages = data_size / PACKAGE_DATA_SIZE ;
            std::size_t remainder = data_size % PACKAGE_DATA_SIZE ;
            unsigned long remainder_pack_num = remainder ? 1 : 0 ;
            
            for ( auto loop : { std::make_pair( num_of_full_packages , PACKAGE_DATA_SIZE ) , 
                                std::make_pair( remainder_pack_num   , remainder ) } ) 
            {
                for ( unsigned long packs = 0 ; packs < loop.first ; ++ packs ) 
                { 
                    std::size_t const bytes_to_read = loop.second ;
                    CTransferTunnel::CDataPackagePOD package ;
                    
                    if ( stream.eof() ) return false ;
                    
                    std::streamsize bytes_to_wirte = stream.read( package.data , bytes_to_read ).gcount() ;
                    
                    std::uint32_t checksum = 0 ; // calc_crc32 ;
                    to_network( &package , ( std::uint32_t ) bytes_to_wirte , checksum ) ;
                    
                    std::size_t written = 0 ;
                    for ( unsigned attempt = ATTEMPTS ; ; ) // attempt to send package
                    {
                        CHeaderPOD control_header ;
                        
                        peer.write_all( &package , sizeof package , written ) ;
                        peer.read( &control_header , sizeof control_header ) ;
                        
                        switch ( control_header.type )
                        {
                            APROOVE : break ;
                            
                            PACKAGE_CANCEL :
                                if ( ! attempts )
                                    throw CTransferException( "PACKAGE_CANCEL" ) ;
                                -- attempts ;
                                continue ;
                                
                            TRANSFER_CANCEL :
                                throw CTransferException( "TRANSFER_CANCEL" ) ;
                        }
                        
                        break ; // all is OK.
                    } // attempt to send package
                } // send packages
            }
            return true ;
        }
    }
    
    void CTransferTunnel::send( std::istream& stream ,
                                const std::string& addr , 
                                const network::ip::port_type port ,
                                const std::size_t chunk_size ) 
    {
         using namespace std::chrono ;
         using namespace network::ip ;
         
         if ( ! chunk_size )
             throw std::invalid_argument( "chunk size cannot be zero" ) ;
         
         port_type current_port = port ;
         
         for ( ; ; ) 
         {
            CSocket peer { EAddressFamily::IPv4 , ESocketType::STERAM } ;
         
            peer.set_option( CWriteTimeout{ seconds{ TIMEOUT } } ) ;
            peer.set_option( CReadTimeout { seconds{ TIMEOUT } } ) ;
            
            peer.connect( addr , current_port ) ; // assert( there is a listening current_port at addr ) ;
            
            if ( ! send_data_from_stream( stream , peer , chunk_size ) ) 
            {
                CHeaderPOD eof { END_OF_STERAM } ;
                peer.write_all( &eof , sizeof eof ) ;
                return ;
            }
            
            CHeaderPOD ok { DATA_AVAILABLE } ;
            peer.write_all( &ok , sizeof ok ) ;
            
            CEndOfChunkPackagePOD eoc_package ;
            std::uint16_t next_port ;
            peer.read( &eoc_package , sizeof eoc_package ) ;
            from_network( eoc_package , &next_port ) ;
            
            current_port = next_port ; // TODO : next_addr
         }
    }
    
    
    void CTransferTunnel::receive( std::istream& stream ,
                                   const std::string& addr , // from
                                   network::ip::port_type port ,
                                   std::size_t chunck_size )
    {
        
    }
    
    
    
                    // encrypt( ( std::uint8_t * ) package.data , bytes_to_wirte , key ) ; // htonsl if alg use > char
                // checksum( (  ) ) ;
    
    
/*
    void CTransferTunnel::send( std::istream& stream ,
                                const std::string& addr ,
                                network::ip::port_type port ,
                                std::size_t file_size ,
                                std::size_t chunck_size ) 
    {
        using namespace network::ip ;
        

        for ( port_type current_port = port ;  
                ; 
              current_port += PORT_JUMP_DELTA )
        {
            
            CSocket peer { EAddressFamily::IPv4 , ESocketType::STERAM } ;
            peer.set_option( CWriteTimeout{ seconds{ TIMEOUT } } ) ;
            peer.set_option( CReadTimeout{ seconds{ TIMEOUT } } ) ;
            peer.connect( addr , current_port ) ;
            
            for ( ; ; ) // send current chunck splitted on packages  
            {
                CDataPackagePOD package ;
                std::streamsize bytes_to_wirte = stream.read( package.data , PACKAGE_DATA_SIZE ).gcount() ;  
                
                // prepeare package
                package.pack_header.type = DATA ;
                package.data_header.size = 
                to_network( &package ) ;
                
                std::size_t bytes_written = 0 ;
                
                for ( std::uint8_t attepts = SEND_ATTEMPTS ; ; ) // attempt to send part
                {
                    CHeaderPOD control_package ;
                    
                    try 
                    { 
                        peer.write_all( ( std::uint8_t * ) package , sizeof package - bytes_written , bytes_written ) ; // send package  
                        peer.read( ( std::uint8_t * ) control_package , sizeof control_package ) ; // reply
                    } 
                    catch ( const CSocketIOAttemptException& ) 
                    {
                        if ( ! attempts ) 
                            throw CTransferException( "Peer is not responding" )  ;
                        -- attempts ; 
                        continue ;
                    }
                            
                    switch ( control_package.type )
                    {
                        case APROOVE : 
                            break ; 
                            
                        case PACK_CANCEL : 
                            if ( ! attempts )
                                throw CTransferException( "Peer rejected data" ) ;
                            -- attempts ;
                            continue ; 
                        default :
                            throw CTransferException( "Protocol Error" ) ;
                    }
                    
                    break ; // all sent.
                } // attempt to send part
            
            } // sending all file.
        }
        
    }
    
    // static member
    void CTransferTunnel::receive( std::istream& stream ,
                                   const std::string& addr ,
                                   network::ip::port_type port ,
                                   std::size_t file_size ,
                                   std::size_t chunck_size )
    {
        
    }
*/




}
