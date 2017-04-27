#include <istream>
#include <chrono>
#include <exception>
#include <memory>
#include <utility>

#include "network/Types.hxx"
#include "network/CSocket.hxx"


namespace transfer_protocol 
{
    
    void CTransferTunnel::to_network( CDataPackagePOD& data , std::uint32_t data_sz , std::uint8_t eof ) 
    {
        data.header.type = DATA ;
        data.size = network::ip::htonl( data_sz ) ;
        data.eof  = eof ; 
    }
    
    void CTransferTunnel::from_network( const CDataPackagePOD& data , std::uint32_t& data_sz , std::uint8_t& eof ) 
    {
        data_sz = network::ip::ntohl( data.size ) ;
        eof     = data.eof ; 
    }
    
    void CTransferTunnel::to_network( CJumpPackagePOD& jump , network::ip::port_type port ) 
    {
        jump.header.type = JUMP_PACKAGE ;
        jump.port        = network::ip::htons( port ) ;
    }
    
    void CTransferTunnel::from_network( const CJumpPackagePOD& jump , network::ip::port_type& port ) 
    {
        port = network::ip::ntohs( jump.port ) ;
    }
    

    bool CTransferTunnel::send_from_stream( std::istream& stream , 
                                            network::ip::CSocket& peer ,
                                            std::size_t data_size )
    {
        using CTransferTunnel::PACKAGE_DATA_SIZE ;
        using CTransferTunnel::CDataPackagePOD ;

        auto buffer = std::unique_ptr< CDataPackagePOD >{ new CDataPackagePOD } ;
        
        unsigned long num_of_full_packages = data_size / PACKAGE_DATA_SIZE ;
        std::size_t remainder = data_size % PACKAGE_DATA_SIZE ;
        unsigned long remainder_pack_num = remainder ? 1 : 0 ;
        
        for ( auto loop : { std::make_pair( num_of_full_packages , PACKAGE_DATA_SIZE ) , 
                            std::make_pair( remainder_pack_num   , remainder ) } ) 
        {
            for ( unsigned long packs = 0 ; packs != loop.first ; ++ packs ) 
            { 
                std::size_t const bytes_to_read = loop.second ;
                std::streamsize bytes_to_wirte = stream.read( buffer -> data , bytes_to_read ).gcount() ;
                
                to_network( * buffer , ( std::uint32_t ) bytes_to_wirte , stream.eof() ) ;
                std::size_t written ;
                peer.write_all( buffer.get() , sizeof * buffer , written ) ;
                
                if ( stream.eof() ) return data_size == 0 ;
            } // send packages
        }
        return true ;
    }
        
    bool CTransferTunnel::recv_to_stream( std::ostream& stream , 
                                          network::ip::CSocket& peer ,
                                          std::size_t const data_sz )
    {
        using CTransferTunnel::CDataPackagePOD ;
        
        auto buffer = std::unique_ptr< CDataPackagePOD >{ new CDataPackagePOD } ;
        
        for ( std::size_t totally_received = 0 ; 
                totally_received < data_sz ; ) 
        {
            std::uint32_t dsize ; std::uint8_t eof ;
            peer.read( buffer.get() , sizeof * buffer ) ;
            from_network( * buffer , dsize , eof ) ;
            stream.write( buffer -> data , dsize ) ;
            totally_received += size ;
            if ( eof ) 
                return false ;
        }
        
        return true ;
    }

    
    void CTransferTunnel::send( std::istream& stream ,
                                const std::string& addr , 
                                const network::ip::port_type port ,
                                const std::size_t chunk_sz ) 
    {
         using namespace std::chrono ;
         using namespace network::ip ;
         
         if ( ! chunk_size )
             throw std::invalid_argument( "chunk size cannot be zero" ) ;
         
         port_type current_port = port ;
         
         for ( ; ; ) 
         {
            CSocket peer { EAddressFamily::IPv4 , ESocketType::STERAM } ;
         
            peer.set_option( CWriteTimeout{ seconds{ TIMEOUT_SEC } } ) ;
            peer.set_option( CReadTimeout { seconds{ TIMEOUT_SEC } } ) ;
            
            peer.connect( addr , current_port ) ; // assert( there is a listening current_port at addr ) ;
            
            if ( ! send_from_stream( stream , peer , chunk_sz ) ) 
            {
                return ;
            }
            
            CJumpPackagePOD jump ;
            std::uint16_t next_port ;
            peer.read( &jump , sizeof eoc_package ) ;
            from_network( jump , current_port ) ;  // TODO : next_addr
         }
    }
    
    void CTransferTunnel::receive( std::istream& stream ,
                                   const std::string& addr , 
                                   const network::ip::port_type port ,
                                   const std::size_t chunk_sz , 
                                   unsigned short const delta_port ) 
    {
        using namespace std::chrono ;
        using namespace network::ip ;
        
        CSocket peer { EAddressFamily::IPv4 , ESocketType::STERAM } ;
        peer.bind( addr , port ) ;
        peer.listen( CLIENT_QUEUE ) ;
        
        for ( port_type current_port = port ; ; )
        {
            
            auto source = peer.accept() ;
            
            if ( ! recv_to_stream( stream , source , chunk_sz ) ) 
            {
                return ;
            }
            
            auto new_peer = CSocket{ EAddressFamily::IPv4 , ESocketType::STERAM } ;
            
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
           peer.write_all(  ,  , written ) ;
           
        }
    }
}

