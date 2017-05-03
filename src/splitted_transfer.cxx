#include <iostream>
#include <chrono>
#include <unordered_map>
#include <string>
#include <exception>
#include <fstream>

#include "CTransferTunnel.hxx"

using namespace std::chrono ;
using namespace transfer_protocol ;
using namespace network::ip ;

// 
// usage :
// general_test operation address port filename
//

namespace 
{
    
    
    struct Client final
    {
        struct ResposeFileRequest
        {
           unsigned char size [ 8 ] alignas( 8 ) ;
           enum { OK , CANCEL } Status status ;
        } ;
        
        void invite_and_play_with_server ( const std::string& addr , port_type port )
        {
            CSocket cmd_socket { EAddressFamily::IPv4 , ESocketType::STREAM } ;
            cmd_socket.connect( addr , port ) ;
            cmd_socket.set_option( CWriteTimeout{ seconds{ 5 } } ) ;
            
            std::stringstream line ;
            while ( true ) try
            {
                CTransferTunnel_TCP::recv_stream( line , cmd_socket ) ;
                interpret( line ,  ) ;
                
            }
            catch ( const CSocketException& ) {
                
            }
            catch ( const CSessionEnd& ) {
                
            }
        }
        
        private :
            void interpret ( std::istringstream& line_stream , std::ostream& out , CSocket& cmd_socket )
            {
                std::string cmd ; line_stream >> cmd ;
                try { operations.at( line_stream , out , cmd_socket ) ; } 
                catch ( const std::out_of_range& ) {
                    out << "Error : no such a command : " << cmd << std::flush ;
                }
            }
            
            const std::unordered_map< std::string , void (*) ( std::istringstream& , std::ostream& , CSocket& ) > 
                interpreter_operations { { "ls" , ls } ,
                                         { "getfile" , getfile } ,
                                         { "ls" , text_cmd } } ; 
    } ;
    
    struct CJumpPackagePOD 
    {
        CHeaderPOD header ;
        unsigned char padding_[ 1 ] ;
        unsigned char port [ 2 ] alignas( 2 ) ;
    } ;
    
    void to_network( CJumpPackagePOD& jump , network::ip::port_type port ) ;
    void from_network( const CJumpPackagePOD& jump , network::ip::port_type& port ) ;
    
    void send( std::istream& stream ,
               const std::string& addr , 
               const network::ip::port_type port ,
               const std::size_t chunk_sz ,
               std::function< void( std::size_t ) > chunk_sent_callback ) 
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
                chunk_sent_callback() ;
                return ;
            }
            
            chunk_sent_callback() ;
            CJumpPackagePOD jump ;
            peer.read( ( std::uint8_t * )  &jump , sizeof jump , { EReadFlags::WHAIT_ALL } ) ;
            from_network( jump , current_port ) ;  // TODO : next_addr
         }
    }
    
    void receive( std::ostream& stream ,
                  network::ip::CSocket peer ,
                  const std::size_t chunk_sz , 
                  std::function< network::ip::port_type( network::ip::port_type ) > next_port
                  std::function< void( std::size_t ) > chunk_recveived_callback ) 
    {
        using namespace std::chrono ;
        using namespace network::ip ;
        
        std::string addr = peer.bound_address().address() ;
        port_type current_port = peer.bound_address().port() ;
        
        for ( ; ; )
        {
            auto source = peer.accept() ;
            
            source.set_option( CWriteTimeout{ seconds{ TIMEOUT_SEC } } ) ;
            source.set_option( CReadTimeout { seconds{ TIMEOUT_SEC } } ) ;
            
            if ( ! recv_amount_to_stream( stream , source , chunk_sz ) ) 
            {
                chunk_recveived_callback() ;
                return ;
            }
            
            chunck_received_callback() ;
            
            auto new_peer = CSocket{ EAddressFamily::IPv4 , ESocketType::STREAM } ;
            
            for ( unsigned attempt = 0 ; attempt < BIND_ATTEMPTS ; ++ attempt ) 
                try { new_peer.bind( addr , current_port = next_port( current_port ) ) ; }   
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
    
    
    
    struct Server final
    {
        void serve ( const std::string& addr , port_type port )
        {    
            CSocket server { EAddressFamily::IPv4 , ESocketType::STREAM } ;
            
            server.bind( addr , port ) ;
            server.listen( 1 ) ;
            
            std::istringstream sstream ;
            sstream.exceptions( std::ifstream::failbit | std::ifstream::badbit ) ;
            
            while ( true ) try  // add mth.
            {
                auto cmd_socket = server.accept() ; // ctrl C
                cmd_socket.set_option( CReadTimeout{ seconds{ 5 } } ) ;
                
                { auto addr = cmd_socket.remote_endpoint() ;
                  std::cout << addr.address() << ":" << addr.port() << " connected\n" << std::flush ; }
                
                std::string cmd ;
                while ( std::getline( cmd , std::cin ) )
                {
                    sstream.str( cmd ) ;
                    interpret( sstream , std::cout , cmd_socket ) ;
                    sstream.clear() ;
                }
                
                sstream.clear() ;
                std::cin.clear() ; // for EOF and other different 
            }
            catch ( const CSocketException& e )
            {
                std::cerr << "Client disconnected unexpectfully.\nReason : \"" << e.what() << "\"" ;
            }
        }
        
        private :
            
            void interpret ( std::istringstream& line_stream , std::ostream& out , CSocket& cmd_socket )
            {
                std::string cmd ; line_stream >> cmd ;
                try { operations.at( line_stream , out , cmd_socket ) ; } 
                catch ( const std::out_of_range& ) {
                    out << "Error : no such a command : " << cmd ;
                }
                catch ( const std::ios_base::failure& e ) { 
                    out << "bad arguments to " + cmd ;
                }
            }
            
            static void getfile( std::istringstream& args_stream , std::ostream& out , CSocket& peer_cmd ) 
            {
                char tmp_suffix [ L_tmpnam ] = "temporary" ;
                std::tmpnam( tmp_suffix ) ;
                
                network::ip::port_type start_port ;
                std::string fname ; 
                
                args_stream >> fname >> start_port >> port_delta ;
                const std::string tmp_name = fname + "_" + tmp_suffix ;
                
                std::size_t chunk_size ;

                {   
                    std::size_t file_size ;
                    bool status ;

                    { std::istringstream request ( "getfile " + fname + " " + std::to_string( start_port ) ) ;
                      CTransferTunnel_TCP::send_stream( request , peer_cmd ) ; }

                    { std::stringstream response ;
                      CTransferTunnel_TCP::recv_stream( response , peer_cmd ) ;
                      response >> status >> file_size ; 
                      if ( response.fail() ) ; }
                      
                    chunk_size = file_size * 0.1 ;
                }
                
                { 
                    CSocket local_peer { EAddressFamily::IPv4 , ESocketType::STREAM } ;
                    local_peer.bind( peer_cmd.bound_address().address() , start_port ) ;
                    local_peer.listen( CLIENT_QUEUE ) ;
                    
                    { std::istringstream approve ( "i'm ready to accept your sender" ) ;
                      CTransferTunnel_TCP::send_stream( approve , peer_cmd ) ; }
                    
                    auto next_port = [ port_delta ] ( port_type port ) { return port + port_delta ; } ;
                    auto print_progress = [ out& ] ( std::size_t chunk_num ) { out << ( chunk_num * 10 ) << "%" << "\r    \r"  ; } ;
                    std::ofstream fileout ( tmp_name , peer , std::ios_base::binary ) ;
                    std::cout << "starting a transfer of \"" << fname << std::endl ; 
                    CTransferTunnel_TCP::recv( fileout , std::move( local_peer ) , chunk_size , next_port , print_progress ) ; 
                }
                
                int result = std::rename( tmp_name.c_str() , fname.c_str() ) ;
                out << "\nfile was saved to " << ( ! result ? fname : tmp_name ) << std::flush ;
            } 
                
            const std::unordered_map< std::string , void (*) ( std::istringstream& , std::ostream& , CSocket& ) > 
                operations { { "cd" , { cd } } ,
                             { "getfile" , getfile } ,
                             { "ls" , ls } 
                             { "client" , client_cmd } 
                             { "disconnect" , disconnect } } ;
    } ;
}

int main ( int args_num , char ** args ) try
{
    enum ARGS { OPERATION = 1 , ADDRESS , PORT , FNAME , DIR_PATH = FNAME , ARGS_NUM } ;
    
    std::cout.sync_with_stdio( false ) ;
    std::cin.sync_with_stdio( false ) ;
    
    if ( args_num < ARGS_NUM ) { std::cerr << "usage : splitted_file_transfer server address port\n"
                                               "\tsplited_transfer client address port" ;
                                 return - 1 ; }
                
    std::string operation ( args[ 0 ] ) ;
    
    if ( operation == "server" )
    {
        Server().serve( args[ ADDRESS ] , std::stoi( args[ PORT ] ) ) ) ; 
    }
    else if ( operation == "client" )
    {
        Client().invite_and_play_with_server( args[ ADDRESS ] , std::stoi( args[ PORT ] ) ) ;
    }
    else 
    {
        std::cerr << "invalid functionality" ;
        return - 1 ;
    }
    return 0 ;
}
catch ( const std::exception& e )
{
    std::cerr << e.what() ;
    return - 1 ;
}
