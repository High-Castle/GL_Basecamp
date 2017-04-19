#ifndef NETWORK_IP_CIPADDRESS_HXX
#define NETWORK_IP_CIPADDRESS_HXX

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
            CIPAddress () : is_empty_( true ) { } ;
            
            CIPAddress ( EAddressFamily family , const std::string& addr_str , port_type port ) noexcept
                : port_( port ) , family_( family ) , is_valid_( true ) , is_empty_( false )
            { 
                if( ! to_binary_address( family_ , addr_ , addr_str.c_str() ) )
                    is_valid_ = false ;
            }
            
            std::string address () const 
            { 
                char strarr [ IP_ADDRESS_STRING_MAX_LEN + 1 ] ;
                if( ! to_string_address( family_ , addr_ , strarr , IP_ADDRESS_STRING_MAX_LEN + 1 ) )
                    throw CBadIPAddress( "Bad Address" ) ;
                return strarr ; 
            } 
            
            port_type      port     () const noexcept { return port_   ; } // port in host byte order
            EAddressFamily family   () const noexcept { return family_ ; } 
            bool           empty    () const noexcept { return is_empty_ ; }
            bool           is_valid () const noexcept { return is_valid_ ; }
            
            
            private :
                static bool to_string_address( EAddressFamily , const unsigned char * , char * , std::size_t ) ;  // implementation 
                static bool to_binary_address( EAddressFamily , unsigned char * , const char * ) ;  // implementation
                
                port_type        port_   ; // in host byte order
                EAddressFamily   family_ ; 
                
                std::uint8_t is_valid_ : 1 , 
                             is_empty_ : 1 ;
                
                std::uint8_t addr_[ IP_ADDRESS_MAX_LEN ] ; // DO NOT TOUCH, implementation-defined byte order
            

            friend bool operator == ( const CIPAddress& op0 , const CIPAddress& op1 ) 
            {
                if ( op0.family_ != op1.family_ || op0.port_ != op1.port_ ) return false ;
                const std::size_t bound = op0.family_ == EAddressFamily::IPv4 ? IPv4_ADDRESS_LEN : IPv6_ADDRESS_LEN ;
                return std::memcmp( op0.addr_ , op1.addr_ , bound ) == 0 ;
            }
            
            friend bool operator != ( const CIPAddress& op0 , const CIPAddress& op1 ) /*  */
            {
                return ! operator == ( op0 , op1 ) ;
            }
        } ; 
        
    } // ip
} // network
#endif // NETWORK_IP_CIPADDRESS_HXX
