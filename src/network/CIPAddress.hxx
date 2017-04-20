#ifndef NETWORK_IP_CIPADDRESS_HXX
#define NETWORK_IP_CIPADDRESS_HXX

#include <cstring>

#include "Types.hxx"

/* 
 * todo : store as strings ( format them manually, to deal with trailing namespaces, 
 *        192.168.067.01 != 192.168.67.1 , etc. ) 
 *        note, that behaviour 192.168.067.01 == 192.168.67.1 is widespread ( ping, host ... ) 
 */
/* todo : IPv6 support */

/* NOTE : only xxx.xxx.xxx.xxx format without leading zeros is allowed in current version. */
// NOTE : this file is a temporary implementation of CIPAddress ; 
//        further versions are free to contain different signatures of constructors
//        ( but decl statement formed as "CIPAddress addr { (l/r)value , (l/r)value , (l/r)value }" ( or so ) should remain valid ) . 

namespace network 
{
    namespace ip
    { 
        
        struct CIPAddress final
        {
            CIPAddress ()  
            { 
                addr_[ 0 ] = '\0' ;
            } ;
            
            CIPAddress ( EAddressFamily family , const std::string& addr_str , port_type port ) noexcept
                : port_( port ) , family_( family )
            { 
                // todo : check address manually  
                addr_[ 0 ] = '\0' ;
                if ( addr_str.length () <= IP_ADDRESS_STRING_MAX_LEN )
                    std::strcpy( addr_ , addr_str.c_str() ) ;
            }
            
            CIPAddress ( const CIPAddress& adr ) noexcept
                : port_( adr.port_ ) ,
                  family_( adr.family_ )
            {
                std::strcpy( addr_ , adr.addr_ ) ;
            }
            
            CIPAddress& operator = ( const CIPAddress& adr ) noexcept
            {
                port_ = adr.port_ ;
                family_ = adr.family_ ;
                std::strcpy( addr_ , adr.addr_ ) ;
            }
            
            std::string    address () const { return addr_ ; } 
            port_type      port    () const noexcept { return port_   ; } // port in host byte order
            EAddressFamily family  () const noexcept { return family_ ; } 
            bool           empty   () const noexcept { return addr_[ 0 ] == '\0' ; }
            void           release () noexcept { addr_[ 0 ] = '\0' ; }
            
            bool is_valid_string() const noexcept
            {
                for ( const char * it = addr_ ; it != addr_ + sizeof addr_ + 1  ; ++ it )
                    if ( * it == '\0' ) return true ;
                return false ;
            }
            
            private :                            
                port_type      port_     ; 
                EAddressFamily family_   ; 
                char addr_[ IP_ADDRESS_STRING_MAX_LEN + 1 ] ; // 
                
                
            friend class CSocket ; // sad, but true, todo it ( used in both address getters in CSocket )
            
            friend bool operator == ( const CIPAddress& op0 , const CIPAddress& op1 ) 
            { 
                if ( op0.family_ != op1.family_ || op0.port_ != op1.port_ ) return false ;
                return std::strcmp( op0.addr_ , op1.addr_ ) == 0 ;
            }
            
            friend bool operator != ( const CIPAddress& op0 , const CIPAddress& op1 ) /*  */
            {
                return ! operator == ( op0 , op1 ) ;
            }
        } ; 
    } // ip
} // network
#endif // NETWORK_IP_CIPADDRESS_HXX
