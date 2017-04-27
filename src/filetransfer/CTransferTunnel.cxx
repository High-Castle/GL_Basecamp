#include <istream>
#include <chrono>
#include <exception>
#include <memory>
#include <utility>


#include "Types.hxx"
#include "CSocket.hxx"
#include "CTransferTunnel.hxx"



namespace transfer_protocol 
{
    
    void CTransferTunnel_TCP::to_network( CDataPackagePOD& data , std::uint32_t data_sz , std::uint8_t eof ) 
    {
        data.header.type = DATA ;
        reinterpret_cast< std::uint32_t& >( data.size ) = network::htonl( data_sz ) ;
        data.eof  = eof ; 
    }
    
    void CTransferTunnel_TCP::from_network( const CDataPackagePOD& data , std::uint32_t& data_sz , std::uint8_t& eof ) 
    {
        data_sz = network::ntohl( reinterpret_cast< const std::uint32_t& >( data.size ) ) ;
        eof     = data.eof ; 
    }
    
    void CTransferTunnel_TCP::to_network( CJumpPackagePOD& jump , network::ip::port_type port ) 
    {
        jump.header.type = JUMP_PACKAGE ;
        reinterpret_cast< std::uint16_t& >( jump.port ) = network::htons( port ) ;
    }
    
    void CTransferTunnel_TCP::from_network( const CJumpPackagePOD& jump , network::ip::port_type& port ) 
    {
        port = network::ntohs( reinterpret_cast< const std::uint16_t& >( jump.port ) ) ;
    }
    
    void CTransferTunnel_TCP::send_stream ( std::istream& stream , network::ip::CSocket& peer )
    {
        auto buffer = std::unique_ptr< CDataPackagePOD >{ new CDataPackagePOD } ;
        for ( ; stream.good() ;  ) 
        { 
            std::streamsize bytes_to_wirte = stream.read( ( char * ) buffer -> data , PACKAGE_DATA_SIZE ).gcount() ;
            to_network( * buffer , ( std::uint32_t ) bytes_to_wirte , stream.eof() ) ; 
            std::size_t written ;
            peer.write_all( ( const std::uint8_t * ) buffer.get() , sizeof * buffer , written ) ;
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
            peer.read( ( std::uint8_t * ) buffer.get() , sizeof * buffer , { EReadFlags::WHAIT_ALL } ) ;
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
            std::size_t written ;
            peer.write_all( ( const std::uint8_t * ) buffer.get() , sizeof * buffer , written ) ;

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
            
            peer.read( ( std::uint8_t * ) buffer.get() , sizeof * buffer , { EReadFlags::WHAIT_ALL } ) ;
            
            from_network( * buffer , dsize , eof ) ;
            stream.write( ( const char * ) buffer -> data , dsize ) ;
            
            totally_received += dsize ;
            
            if ( eof ) return false ; 
        }
        return true ;
    }

    
    void CTransferTunnel_TCP::send( std::istream& stream ,
                                const std::string& addr , 
                                const network::ip::port_type port ,
                                const std::size_t chunk_sz ) 
    {
         using namespace std::chrono ;
         using namespace network::ip ;
         
         if ( ! chunk_sz )
             throw std::invalid_argument( "chunk size cannot be zero" ) ;
         
         port_type current_port = port ;
         
         for ( ; ; ) 
         {
            CSocket peer { EAddressFamily::IPv4 , ESocketType::STREAM } ;
         
            peer.set_option( CWriteTimeout{ seconds{ TIMEOUT_SEC } } ) ;
            peer.set_option( CReadTimeout { seconds{ TIMEOUT_SEC } } ) ;
            
            peer.connect( addr , current_port ) ; // assert( there is a listening current_port at addr ) ;
            
            if ( ! send_amount_from_stream( stream , peer , chunk_sz ) ) 
            {
                return ;
            }
            
            CJumpPackagePOD jump ;
            peer.read( ( std::uint8_t * )  &jump , sizeof jump , { EReadFlags::WHAIT_ALL } ) ;
            from_network( jump , current_port ) ;  // TODO : next_addr
         }
    }
    
    void CTransferTunnel_TCP::receive( std::ostream& stream ,
                                   const std::string& addr , 
                                   const network::ip::port_type port ,
                                   const std::size_t chunk_sz , 
                                   unsigned short const delta_port ) 
    {
        using namespace std::chrono ;
        using namespace network::ip ;
        
        CSocket peer { EAddressFamily::IPv4 , ESocketType::STREAM } ;
        peer.bind( addr , port ) ;
        peer.listen( CLIENT_QUEUE ) ;
        
        for ( port_type current_port = port ; ; )
        {
            auto source = peer.accept() ;
            
            source.set_option( CWriteTimeout{ seconds{ TIMEOUT_SEC } } ) ;
            source.set_option( CReadTimeout { seconds{ TIMEOUT_SEC } } ) ;
            
            if ( ! recv_amount_to_stream( stream , source , chunk_sz ) ) 
            {
                return ;
            }
            
            auto new_peer = CSocket{ EAddressFamily::IPv4 , ESocketType::STREAM } ;
            
            for ( unsigned attempt = 0 ; attempt < BIND_ATTEMPTS ; ++ attempt ) 
                try { new_peer.bind( addr , current_port += delta_port ) ; }   
                catch ( const CSocketBindException& ) {
                    if ( attempt == BIND_ATTEMPTS ) throw ; 
                    continue ;
                }
           
           peer.listen( CLIENT_QUEUE ) ;
           
           CJumpPackagePOD jump ;
           to_network( jump , current_port ) ;
           std::size_t written ;
           peer.write_all( ( std::uint8_t * ) &jump , sizeof jump , written ) ;
           
           peer = std::move( new_peer ) ;
        }
    }
}

