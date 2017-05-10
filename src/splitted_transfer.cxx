#include <iostream>
#include <chrono>
#include <unordered_map>
#include <string>
#include <sstream>
#include <exception>
#include <algorithm>
#include <fstream>
#include <iterator>

#include "CTransferTunnel.hxx"
#include "CSocket.hxx"
#include "filesystem_utility.hxx"

using namespace std::chrono ;
using namespace transfer_protocol ;
using namespace network::ip ;

namespace 
{    
    enum 
    { 
        MSG_PACK_SIZE = 256 ,
        DATA_PACK_SIZE = 1024 * 1024 * 1 ,
        CLIENT_QUEUE = 1 ,
        
    } ;

    enum 
    {
       BIND_ATTEMPTS = 3 ,
       TIMEOUT_SEC = 10 
    } ;
    
    template < class T >
    void unpack_stream( std::istream& stream , T& curr ) {
        stream >> curr ;
        if ( stream.fail() )
            throw std::invalid_argument( "Error while unpacking stream" ) ;
    }
    
    void unpack_stream( std::istream& stream ) {}
    
    template < class T , class... Args > 
    void unpack_stream ( std::istream& stream , T& curr , Args&... args ) {
        stream >> curr ; 
        if ( stream.fail() )
            throw std::invalid_argument( "Error while unpacking stream" ) ;
        unpack_stream( stream , args... ) ;
    }
    
    void msg_to_peer ( CSocket& peer , const std::string& line ) {
        std::istringstream sstream ( line ) ;
        CTransferTunnel_TCP::send_stream( sstream , peer , MSG_PACK_SIZE ) ;
    }
    
    template< class... Args >
    void msg_from_peer( CSocket& peer , Args&... out_args ) {
        std::stringstream sstream ;
        CTransferTunnel_TCP::recv_stream( sstream , peer , MSG_PACK_SIZE ) ;
        unpack_stream( sstream , out_args... ) ;
    }
    
    void trim ( std::string& str )
    {
        str.erase( str.begin() , std::find_if( str.begin() , str.end() , [] ( char ch ) { return ! std::isspace( ch ) ; } ) ) ;
        str.erase( std::find_if( str.rbegin() , str.rend() , [] ( char ch ) { return ! std::isspace( ch ) ; } ).base() , str.end() ) ;
    }
    
    struct CSessionEnd final : std::exception 
    { 
    } ;
    
    struct ClientScript final
    {    
        void invite_and_play_with_server ( const std::string& addr , port_type port ) 
        {
            CSocket cmd_socket { EAddressFamily::IPv4 , ESocketType::STREAM } ;
            cmd_socket.connect( addr , port ) ;
            cmd_socket.set_option( CWriteTimeout{ seconds{ TIMEOUT_SEC } } ) ;
            
            std::stringstream line ;
            
            while ( true ) try
            {
                line.clear() ;
                line.str( "" ) ;
                CTransferTunnel_TCP::recv_stream( line , cmd_socket , MSG_PACK_SIZE ) ;
                interpret( line , cmd_socket ) ;
            }
            catch ( const CSessionEnd& ) {
                std::cout << "\nsession end" ;
                break ;
            }
        }
        
        private :

            void interpret ( std::stringstream& line_stream , CSocket& cmd_socket )  
            {    
                std::string cmd ; 
                unpack_stream( line_stream , cmd ) ;   
                try { interpreter_operations.at( cmd )( line_stream , cmd_socket ) ; } 
                catch ( const std::out_of_range& e ) {
                    msg_to_peer( cmd_socket , "Error : no such a command : " + cmd ) ; 
                    std::cout << "Error : no such a command : " << cmd << std::flush ;
                }
                catch ( const std::invalid_argument& e ) {
                    msg_to_peer( cmd_socket , e.what() ) ; 
                    std::cout << "\n" << e.what() << std::flush ;
                }
                catch( filesystem_utility::CFilesystemException const& e ) { 
                    msg_to_peer( cmd_socket , e.what() ) ; 
                    std::cerr << "\n" << e.what() << std::flush ; 
                }
            }
            
            const std::unordered_map< std::string , void (*) ( std::stringstream&  , CSocket& ) > 
                interpreter_operations { { "cd" , cd } ,
                                         { "ls" , ls } ,
                                         { "getfile" , getfile } ,
                                         { "disconnect" , disconnect } } ; 
            
            static void cd ( std::stringstream& args , CSocket& peer )
            {
                std::string path ;
                unpack_stream( args , path ) ;
                filesystem_utility::cd( path.c_str() ) ;
                msg_to_peer( peer , "directory changed" ) ; 
            }
                                         
            static void ls ( std::stringstream& args , CSocket& peer )
            {
                std::string path ;
                unpack_stream( args , path ) ;
                std::cerr << "\ndoing ls " << path ; 
                auto entries = filesystem_utility::ls( path.c_str() ) ;
                std::cerr << "\nsending..." ;
                msg_to_peer( peer , entries ) ; 
                std::cerr << "\nls sent" ; 
            }
            
            static void disconnect ( std::stringstream& , CSocket& ) 
            { 
                throw CSessionEnd () ; 
            }
            
            static void getfile( std::stringstream& args_stream , CSocket& peer_cmd ) 
            {
                using namespace std::chrono ;
                using namespace network::ip ;
                std::cerr << "\nstarting " << __func__ ;
                std::string fname ;
                port_type current_port ;
                std::size_t chunk_sz ;
                
                unpack_stream( args_stream , current_port ) ;
                
                fname = std::string( std::istreambuf_iterator< char >{ args_stream } ,  
                                     std::istreambuf_iterator< char >{ } ) ;
                trim( fname ) ;
                
                std::size_t file_sz = filesystem_utility::file_size( fname.c_str() ) ;
                std::ifstream infile ( fname , std::ios_base::binary ) ;
                
                msg_to_peer( peer_cmd , std::to_string( infile.good() ) + " " + std::to_string( file_sz ) ) ;
                
                if ( ! infile.good() )
                    throw std::invalid_argument( "cannot open " + fname + " locally" ) ;
                
                msg_from_peer( peer_cmd , chunk_sz ) ;
                
                if ( ! chunk_sz )
                    throw std::invalid_argument( "chunk size cannot be zero" ) ;
                
                std::string addr = peer_cmd.remote_endpoint().address() ; 
                
                for ( ; ; ) 
                {
                    CSocket peer { EAddressFamily::IPv4 , ESocketType::STREAM } ;
                
                    peer.set_option( CWriteTimeout{ seconds{ TIMEOUT_SEC } } ) ;
                    peer.set_option( CReadTimeout { seconds{ TIMEOUT_SEC } } ) ;
                    
                    peer.connect( addr , current_port ) ; // assert( there is a listening current_port at addr ) ;
                    
                    if ( ! CTransferTunnel_TCP::send_amount_from_stream( infile , peer , chunk_sz , DATA_PACK_SIZE ) ) 
                    {
                        break ;
                    }
                    
                    msg_from_peer( peer , current_port ) ;
                    std::cerr << "\nnext port is " << current_port ;
                }
                std::cerr << "\nfile \"" << fname << "\" sent" ;
            }
            
    } ;
    
    struct ServerScript final
    {
        void serve ( const std::string& addr , port_type port ) const
        {    
            CSocket server { EAddressFamily::IPv4 , ESocketType::STREAM } ;
            
            server.bind( addr , port ) ;
            server.listen( 1 ) ;
            
            std::stringstream sstream( std::ios_base::in | std::ios_base::out | std::ios_base::app ) ;
           // sstream.exceptions( std::ifstream::failbit | std::ifstream::badbit ) ;
            
            while ( true ) try  // add mth.
            {
                std::cin.clear() ;
                auto cmd_socket = server.accept() ; // ctrl C
                cmd_socket.set_option( CReadTimeout{ seconds{ 5 } } ) ;
                
                auto addr = cmd_socket.remote_endpoint() ;
                std::cout << addr.address() << ":" << addr.port() << " connected" << std::endl ;
                
                std::string cmd ;
                std::cout << "\n>: " << std::flush ;
                while ( std::getline( std::cin , cmd ) )
                {
                    sstream.clear() ;
                    sstream.str( cmd ) ;
                    interpret( sstream , cmd_socket ) ;
                    std::cout << "\n>: " << std::flush ;
                }
            }   
                catch ( const CSocketException& e ) 
                {
                    std::cerr << "Client disconnected unexpectfully.\nReason : \"" << e.what() << "\"" ;
                }
                catch ( const CSessionEnd& ) 
                { 
                    break ;
                }
        }
        
        private :
            
            void interpret ( std::stringstream& line_stream , CSocket& cmd_socket ) const
            {
                std::string cmd ; line_stream >> cmd ;
                try { operations.at( cmd )( line_stream , cmd_socket ) ; } 
                catch ( const filesystem_utility::CFilesystemException& e )
                {
                    std::cout << "\n" << e.what() << std::flush ; 
                }
                catch ( const std::out_of_range& ) {
                    std::cout << "Error : no such a command : " << cmd << std::endl ;
                }
                catch ( const std::invalid_argument& e ) {
                    std::cout << "Argument Error : " << e.what() ;
                }
            }
            
            static void getfile( std::stringstream& args_stream , CSocket& peer_cmd ) 
            {
                using namespace network::ip ;
                std::cerr << "\nstarting \"getfile\"" ;
                
                char tmp_suffix [ L_tmpnam ] = "temporary" ;
                
                port_type start_port   ; std::string fname ; 
                std::size_t chunk_size ; float percents    ;
                std::size_t file_size  ; bool status       ;
                port_type port_delta   ;
                
                unpack_stream( args_stream , start_port , port_delta , percents ) ;
                
                fname = std::string( std::istreambuf_iterator< char >{ args_stream } ,  
                                     std::istreambuf_iterator< char >{ } ) ;
                trim( fname ) ;
                
                if ( percents > 100 ) 
                    throw std::invalid_argument( "bad percent : " + std::to_string( percents ) ) ;
                
                //std::tmpnam( tmp_suffix ) ;
                const std::string tmp_name = fname + "_" + tmp_suffix ;
                
                std::cerr << "\ntriggering client to send operation..." ;
                msg_to_peer( peer_cmd , "getfile " + std::to_string( start_port ) + " " + fname ) ;
                std::cerr << " ok\ngetting requested file size..." ;
                msg_from_peer( peer_cmd , status , file_size ) ;
                std::cerr << " " << file_size ;
                
                if ( ! status )
                    throw std::invalid_argument( "client declined request to send" ) ;
                
                std::cerr << "\nport delta is " << port_delta ;
                std::cerr << "\npercents of whole size are " << percents ;
                
                chunk_size = ( file_size ? file_size * ( percents / 100.f ) : 1 ) ;
                std::cerr << "\nchunk size is " << chunk_size ; 
                
                std::cerr << "\ncreate listening socket..." ; 
                CSocket connector_peer { EAddressFamily::IPv4 , ESocketType::STREAM } ;
                connector_peer.bind( peer_cmd.bound_address().address() , start_port ) ;
                connector_peer.listen( CLIENT_QUEUE ) ;
                std::cerr << " created" ;
                
                std::string addr = peer_cmd.bound_address().address() ;
                port_type current_port = peer_cmd.bound_address().port() ;
                
                std::cerr << "\nsending chunk size to client..." ;
                msg_to_peer( peer_cmd , std::to_string( chunk_size ) ) ;
                std::cerr << " sent" ;
                
                std::cerr << "\nopening file..." ;
                std::ofstream fileout ( tmp_name , std::ios_base::binary ) ;
                std::cerr << " open" ;
                
                std::cout << "\nstarting a transfer of \"" << fname << "\"" << std::endl ; 
                std::cerr << "\nstarting transfer" ;
                
                for ( std::size_t chunk_num = 0 ; ;  )
                {
                    auto source = connector_peer.accept() ;
                    
                    source.set_option( CWriteTimeout{ seconds{ TIMEOUT_SEC } } ) ;
                    source.set_option( CReadTimeout { seconds{ TIMEOUT_SEC } } ) ;
                    
                    if ( ! CTransferTunnel_TCP::recv_amount_to_stream( fileout , source , chunk_size , DATA_PACK_SIZE ) ) 
                    {
                        break ;
                    }
                    std::cout << "\n" << ( ++ chunk_num * percents ) << "% "  << std::flush ;
                    
                    CSocket new_peer { EAddressFamily::IPv4 , ESocketType::STREAM } ;
                    
                    for ( unsigned attempt = 0 ; ; ) 
                    {
                        try 
                        { 
                            current_port += port_delta ;
                            new_peer.bind( addr , current_port ) ; 
                            break ;
                        }   
                        catch ( const CSocketBindException& ) {
                            ++ attempt ;
                            std::cerr << "\nattempt to bind port " << current_port << " failed" ;
                            if ( attempt == BIND_ATTEMPTS ) throw ; 
                        }
                    }
                    
                    new_peer.listen( CLIENT_QUEUE ) ;
                    
                    std::cerr << "\nsending next port to client..." ;
                    msg_to_peer( source , std::to_string( current_port ) ) ;
                    std::cerr << "sent" ;
                    
                    connector_peer = std::move( new_peer ) ;
                }
                
                int result = std::rename( tmp_name.c_str() , fname.c_str() ) ;
                std::cout << "\nfile was saved to \"" << ( ! result ? fname : tmp_name ) << "\"" << std::flush ;
                std::cerr << "\ntransfered" ;
            } 
            
            static void ls ( std::stringstream& args , CSocket& peer )
            {
                std::string path ;
                unpack_stream( args , path ) ;
                std::cout << filesystem_utility::ls( path.c_str() ) << std::flush ; 
            }
            
            static void end_session ( std::stringstream& , CSocket& ) { 
                throw CSessionEnd () ; 
            }
            
            static void client_cmd ( std::stringstream& line , CSocket& peer )
            {
                CTransferTunnel_TCP::send_stream( line , peer , MSG_PACK_SIZE ) ;
                CTransferTunnel_TCP::recv_stream( std::cout , peer , MSG_PACK_SIZE ) ;
            }
            
            static void cd ( std::stringstream& args , CSocket& )
            {
                std::string str ;
                unpack_stream( args , str ) ;
                filesystem_utility::cd( str.c_str() ) ;
            } ;
            
            static void help ( std::stringstream& args , CSocket& )
            {
                std::cout << "\nusage :"
                             "\n    >: getfile initial_port port_delta percents FILENAME"
                             "\n    >: client ls path"
                             "\n    >: ls path (local)" ;
            }
            
            const std::unordered_map< std::string , void (*) ( std::stringstream& , CSocket& ) > 
                operations { { "cd" , cd } ,
                             { "getfile" , getfile } ,
                             { "ls" , ls } ,
                             { "client" , client_cmd } ,
                             { "!q" , end_session } ,
                             { "!help" , help }
                } ;         
    } ;
}


// usage :



int main ( int args_num , char ** args ) try
{
    enum ARGS { OPERATION = 1 , ADDRESS , PORT , FNAME , DIR_PATH = FNAME , ARGS_NUM = DIR_PATH } ;
    
    std::cout.sync_with_stdio( false ) ;
    std::cin.sync_with_stdio( false ) ;
    
    if ( args_num < ARGS_NUM ) { std::cerr << "usage : splitted_transfer server address port\n"
                                               "\tsplitted_transfer client address port" ;
                                 return - 1 ; }
                
    std::string operation ( args[ OPERATION ] ) ;
    
    if ( operation == "server" )
    {
        ServerScript().serve( args[ ADDRESS ] , std::stoi( args[ PORT ] ) )  ; 
    }
    else if ( operation == "client" )
    {
        ClientScript().invite_and_play_with_server( args[ ADDRESS ] , std::stoi( args[ PORT ] ) ) ;
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
