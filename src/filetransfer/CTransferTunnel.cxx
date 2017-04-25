namespace transfer_protocol 
{
    // static member
    void CTransferTunnel::send( std::istream& stream ,
                                const std::string& addr ,
                                network::ip::port_type port ,
                                std::size_t file_size ,
                                std::size_t chunck_size ) 
    {
        using namespace network::ip ;
                
        for ( ; ; )  
        {
            CSocket peer { EAddressFamily::IPv4 , ESocketType::STERAM } ;

            peer.set_option( CWriteTimeout{ seconds{ TIMEOUT } } ) ;
            peer.set_option( CReadTimeout{ seconds{ TIMEOUT } } ) ;
            
            peer.connect( addr , current_port ) ;
            
            CDataPackagePOD package ;
            stream.read( package.data , PACKAGE_DATA_SIZE ) ;  
            
            // prepeare package
            package.pack_header.type = DATA ;
            package.data_header.
            to_network( &package ) ;
            
            // send package ( arg - num of attempts )
            for ( ; ; ) 
            {
                CHeaderPOD control_package ;
                try 
                { 
                    peer.write_all( ( std::uint8_t * ) package , sizeof package ) ;   
                    peer.read( ( std::uint8_t * ) control_package , sizeof control_package ) ; // reply
                } 
                catch ( const CSocketIOAttemptException& ) 
                {
                    if ( ! attempts ) 
                        throw CTransferException( "Peer is not responding" )  ;
                }
                        
                switch ( control_package.type )
                {
                    case APROOVE : 
                        
                    case PACK_CANCEL : 
                        if ( ! attempts )
                            throw CTransferException( "Peer rejected data" ) ;
                        -- attempts ;
                        continue ;
                    default :
                        throw CTransferException( "Protocol Error" ) ;
                }
                
                break ;
            } // attempt to send part
            
        } // sending all file.
    }
    
    // static member
    void CTransferTunnel::receive(  )
    
    
    
}
