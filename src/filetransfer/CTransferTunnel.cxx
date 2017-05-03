#include <istream>
#include <chrono>
#include <exception>
#include <memory>
#include <utility>
#include <functional>

#include <iostream> // cerr, debug

#include "Types.hxx"
#include "CSocket.hxx"
#include "CTransferTunnel.hxx"



namespace transfer_protocol 
{
    void CTransferTunnel_TCP::to_network( CDataPackagePOD& data , std::uint32_t data_sz , std::uint8_t eof ) 
    {
        data.header.type = DATA ;
        reinterpret_cast< std::uint32_t& >( data.size ) = network::hton_l( data_sz ) ;
        data.eof  = eof ; 
    }
    
    void CTransferTunnel_TCP::from_network( const CDataPackagePOD& data , std::uint32_t& data_sz , std::uint8_t& eof ) 
    {
        data_sz = network::ntoh_l( reinterpret_cast< const std::uint32_t& >( data.size ) ) ;
        eof     = data.eof ; 
    }

    
    namespace
    {
        enum Polynom32 : std::uint32_t
        { 
            ETHERNET = 0x04C11DB7
        } ;

        // references : [1] http://www.zlib.net/crc_v3.txt, 
        //              [2] http://create.stephan-brumme.com/crc32/ 
        //              [3] https://www.cs.jhu.edu/~scheideler/courses/600.344_S02/CRC.html 
        //              [4] https://en.wikipedia.org/wiki/Mathematics_of_cyclic_redundancy_checks
        std::uint32_t crc32 ( const std::uint8_t * const data , std::size_t sz )
        { // note it doesn't implement any standard, just for simple check
            enum { BITS_IN_BYTE = 8 } ;
            enum { POLY = ETHERNET } ;
            
            std::uint32_t acc = 0 ; 
            
            // division [1]
            for ( std::uint8_t const * current = data ; 
                current < data + sz ; 
                ++ current )
            {
                acc ^= * current ; // XOR-add/sub 
                
                for ( std::uint8_t each_bit = BITS_IN_BYTE ; each_bit ; -- each_bit ) // TODO : table approach
                {
                    acc = ( acc >> 1 ) /* reversed bit order, really doesn't matter here; both recv and send are doing same thing on the same level */ 
                                    ^ ( ( acc & 1 ) * POLY ) /*  if ( reversed >= poly_part ) then ( reversed - poly )
                                                                    ( note that >= and - are operators of non-carry arithmetic [1] ) ) */ ;
                }
            }
            return acc ;
        }

        
        void send_data_pack( transfer_protocol::CTransferTunnel_TCP::CDataPackagePOD * pack ,
                             network::ip::CSocket& peer , 
                             unsigned long attempts = transfer_protocol::CTransferTunnel_TCP::ADDITIONAL_TRANSFER_ATTEMPTS )
        {
            using namespace network::ip ;
            ( std::uint32_t& ) pack -> checksum = 0 ;
            ( std::uint32_t& ) pack -> checksum = calc_checksum_hton( ( std::uint8_t * ) pack , sizeof * pack ) ;
            
            for ( ; ; -- attempts ) 
            {
                std::size_t written ;
                peer.write_all( ( const std::uint8_t * ) pack , sizeof * pack , written ) ;  
                CHeaderPOD remote_is_done ; // rename
                peer.read( ( std::uint8_t * ) &remote_is_done , sizeof( CHeaderPOD ) ) ; // 1 byte pack
                
                if ( remote_is_done.type == APPROVE ) break ;
                if ( ! attempts ) 
                    throw CTransferException( "Error while transfering, checksum doesn't match" ) ;
                std::cerr << "\npackage has been dropped" ; 
            } 
        }
        
        void recv_data_pack( transfer_protocol::CTransferTunnel_TCP::CDataPackagePOD * pack ,
                             network::ip::CSocket& peer, 
                             unsigned long attempts = transfer_protocol::CTransferTunnel_TCP::ADDITIONAL_TRANSFER_ATTEMPTS )
        {
            using namespace network::ip ;
            for ( ; ; -- attempts ) 
            {
                peer.read( ( std::uint8_t * ) pack , sizeof * pack , { EReadFlags::WHAIT_ALL } ) ;
                
                std::uint32_t remote_sum = reinterpret_cast< std::uint32_t& >( pack -> checksum ) ;
                reinterpret_cast< std::uint32_t& >( pack -> checksum ) = 0 ;
                std::uint32_t sum = calc_checksum_hton( ( std::uint8_t * ) pack , sizeof * pack ) ;
                

                CHeaderPOD im_ready { remote_sum == sum ? APPROVE : PACK_CANCEL } ;
                std::size_t written ;
                peer.write_all( ( const std::uint8_t * ) &im_ready , sizeof( CHeaderPOD ) , written ) ; // 1 byte pack
                
                if ( im_ready.type == APPROVE ) break ;
                if ( ! attempts )
                    throw CTransferException( "Error while transfering, checksum doesn't match" ) ;
                std::cerr << "\npackage has been dropped" ; 
            }
        }
    }
    
    std::uint32_t calc_checksum_hton( const std::uint8_t * data , std::size_t sz )
    {
        return network::hton_l( crc32( data , sz ) ) ;
    }
    
    void CTransferTunnel_TCP::send_stream ( std::istream& stream , network::ip::CSocket& peer )
    {
        using namespace network::ip ;
        auto buffer = std::unique_ptr< CDataPackagePOD >{ new CDataPackagePOD } ;
        for ( ; stream.good() ;  ) 
        { 
            std::streamsize bytes_to_wirte = stream.read( ( char * ) buffer -> data , PACKAGE_DATA_SIZE ).gcount() ;
            to_network( * buffer , ( std::uint32_t ) bytes_to_wirte , stream.eof() ) ; // NOTE : signed -> unsigned 
            send_data_pack( buffer.get() , peer ) ;
        } 
    }
    
    void CTransferTunnel_TCP::recv_stream( std::ostream& stream , network::ip::CSocket& peer )
    {
        using namespace network::ip ;
        using CDataPackagePOD = CTransferTunnel_TCP::CDataPackagePOD ;
        
        auto buffer = std::unique_ptr< CDataPackagePOD >{ new CDataPackagePOD } ;

        for ( ; ; ) 
        {
            std::uint32_t dsize ; std::uint8_t eof ;
            recv_data_pack( buffer.get() , peer ) ;
            from_network( * buffer , dsize , eof ) ;
            stream.write( ( const char * ) buffer -> data , dsize ) ;
            if ( eof ) break ; 
        }
    }
    
    bool CTransferTunnel_TCP::send_amount_from_stream( std::istream& stream , 
                                                       network::ip::CSocket& peer ,
                                                       std::size_t data_size )
    {
        std::size_t const PACKAGE_DATA_SIZE = CTransferTunnel_TCP::PACKAGE_DATA_SIZE ;
        using CDataPackagePOD = CTransferTunnel_TCP::CDataPackagePOD ;
        
        auto buffer = std::unique_ptr< CDataPackagePOD >{ new CDataPackagePOD } ;
        
        std::size_t remainder = data_size % PACKAGE_DATA_SIZE ;
        unsigned long num_of_packages = data_size / PACKAGE_DATA_SIZE + ( remainder ? 1 : 0 ) ;

        for ( unsigned long packs = 0 ; packs != num_of_packages ; ++ packs ) 
        { 
            std::streamsize bytes_to_wirte = stream.read( ( char * ) buffer -> data , PACKAGE_DATA_SIZE ).gcount() ;

            to_network( * buffer , ( std::uint32_t ) bytes_to_wirte , stream.eof() ) ;
            send_data_pack( buffer.get() , peer ) ;

            if ( ! stream.good() ) return data_size == 0 ; 
        } // send packages
        return true ;
    }
    
    // TODO : make themselfs streams.
    
    // false if eof received
    bool CTransferTunnel_TCP::recv_amount_to_stream( std::ostream& stream , 
                                                     network::ip::CSocket& peer ,
                                                     std::size_t const data_sz )
    {
        using namespace network::ip ;
        using CDataPackagePOD = CTransferTunnel_TCP::CDataPackagePOD ;
        
        auto buffer = std::unique_ptr< CDataPackagePOD >{ new CDataPackagePOD } ;
        std::size_t totally_received = 0 ;
        for ( ; totally_received < data_sz ; ) 
        {
            std::uint32_t dsize ; std::uint8_t eof ;
            
            recv_data_pack( buffer.get() , peer ) ;
            
            from_network( * buffer , dsize , eof ) ;
            stream.write( ( const char * ) buffer -> data , dsize ) ;
            
            totally_received += dsize ;
            
            if ( eof ) return false ; 
        }
        return true ;
    }
}

