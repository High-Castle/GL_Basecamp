#include <iostream>
#include <unordered_map>

#include "network/CSocket.hxx"
#include "CFileSender.hxx"



struct CInterpretException : CBasicException
{
    using CBasicException::CBasicException ;
} ;

struct CInterpretUnhandledException : CBasicException
{
    using CBasicException::CBasicException ;
} ;

class CInterpreterClientSocket final
{
    CInterpreterClientSocket (  , , ) ;
    
    CInterpreterClientSocket( const CInterpreterClientSocket& )             = delete ;
    CInterpreterClientSocket( CInterpreterClientSocket&& )                  = delete ;
    CInterpreterClientSocket& operator=( const CInterpreterClientSocket& )  = delete ;
    CInterpreterClientSocket& operator=( CInterpreterClientSocket&& )       = delete ;
    
    
    std::string interpret ( const std::string& line ) ;

    private :
        
        transfer_protocol::CFileTransfer file_sender_    ;
        network::CSocket                 command_socket_ ;
        
        const std::unordered_map< std::string , std::string ( * ) ( std::istream& ) > func_map_
        {
            { "ls" , ls_ } , { "cd" , cd_ } , { "mkdir" , mkdir_ } , { "get" , get_ } , { "connect" , connect_ }
        } ;
        
        /* to unnamed namespace in .cxx file , will be not members */
        static std::string ls_ ( std::istream& in ) ;
        static std::string cd_ ( std::istream& in ) ;
        static std::string mkdir_ ( std::istream& in ) ;
        static std::string get_ ( std::istream& in ) ;
} ;

        /*
        static std::string connect_ ( std::istream& in )
        {
            std::string address , port ;
            in >> address >> port ;
            try { command_socket_.connect( address , std::stoi( port ) ) ; }
            catch ( const std::invalid_argument& e ) 
            { 
                throw CInterpretException( "Cannont recognize port number : " + port ) ; 
            }
        }
        //*/
