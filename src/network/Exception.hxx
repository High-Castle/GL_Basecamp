#ifndef NETWORK_EXCEPTION_HXX
#define NETWORK_EXCEPTION_HXX

#include <exception>
#include <cstddef>
#include <cstring>
#include <string>

namespace network
{    
    class CBasicException : public std::exception
    {
        public :
            static const std::size_t MAX_MESSAGE_LEN = 255 ;
            const char * what () const noexcept final { return message_ ; }
        protected :
            CBasicException( const char * message ) noexcept 
            { 
                std::size_t len = std::string::traits_type::length( message ) ;
                std::size_t min = len < MAX_MESSAGE_LEN ? len : MAX_MESSAGE_LEN ;
                std::memcpy( message_ , message , min ) ;
                message_[ min ] = '\0' ; 
            }
            
            
        private :
            char message_ [ MAX_MESSAGE_LEN + 1 ] ;
    } ;
    
    namespace ip
    {
        struct CSocketException : CBasicException  
        { 
            CSocketException( const char * message ) noexcept : 
                    CBasicException ( message ) {}
        } ;
        
        struct CBadIPAddress : CBasicException  
        { 
            CBadIPAddress( const char * message ) noexcept : 
                    CBasicException ( message ) {}
        } ;
        
        // do not confuse CSocketConnectionException with CSocketConnectException
        // CSocketConnectException is thrown only on attempt to establish connection ( connect member )
        struct CSocketConnectionException : CSocketException { using CSocketException::CSocketException ; } ;
        struct CSocketInitException       : CSocketException { using CSocketException::CSocketException ; } ;
        struct CSocketReadException       : CSocketException { using CSocketException::CSocketException ; } ;
        struct CSocketWriteException      : CSocketException { using CSocketException::CSocketException ; } ;
        
        struct CSocketReadAttemptException   : CSocketException { using CSocketException::CSocketException ; } ;
        struct CSocketWriteAttemptException  : CSocketException { using CSocketException::CSocketException ; } ;
        
        struct CSocketListenException     : CSocketException { using CSocketException::CSocketException ; } ;
        struct CSocketAcceptException     : CSocketException { using CSocketException::CSocketException ; } ;
        struct CSocketConnectException    : CSocketException { using CSocketException::CSocketException ; } ;
        struct CSocketBindException       : CSocketException { using CSocketException::CSocketException ; } ; 
        struct CSocketSetOptionException  : CSocketException { using CSocketException::CSocketException ; } ;
        struct CSocketLogicException      : CSocketException { using CSocketException::CSocketException ; } ;
    } // ip
} // network

#endif // NETWORK_EXCEPTION_HXX
