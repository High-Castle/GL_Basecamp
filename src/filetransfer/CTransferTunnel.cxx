namespace transfer_protocol 
{
    using aligned_header = std::aligned_storage< sizeof( CTransferTunnel::CHeaderPOD ) , std::max_align_t > ;
    
    void CTransferTunnel::send( std::istream& stream ,
                                const std::string& addr , 
                                network::ip::port_type port ,
                                std::size_t file_size ,
                                const std::size_t chunck_size ,
                                const std::size_t port_jump_delta ) 
    {
         using namespace network::ip ;
         
         for ( port_type current_port = port ;  
                ; 
               current_port += port_jump_delta )
        {
            // encrypt , checksum each part
            CSocket peer { EAddressFamily::IPv4 , ESocketType::STERAM } ;
            peer.set_option( CWriteTimeout{ seconds{ TIMEOUT } } ) ;
            peer.set_option( CReadTimeout{ seconds{ TIMEOUT } } ) ;
            peer.connect( addr , current_port ) ;
            
            for (  ; ;  ) // send splitted on packages
            {
                for ( unsigned attempt = ATTEMPTS ; ;  )
                {
                    std::streamsize bytes_to_wirte = stream.read( package.data , PACKAGE_DATA_SIZE ).gcount() ;
                    
                }
            }
        }
    }
    
    
    
    
    
    
    
    
    
    
    
    
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