#ifndef NETWORK_IP_TYPES_HXX
#define NETWORK_IP_TYPES_HXX

#include <cstddef>

namespace network 
{
    namespace ip
    {   
        enum : std::size_t
        {
            IPv4_ADDRESS_LEN = 4  ,
            IPv6_ADDRESS_LEN = 16 ,
            IP_ADDRESS_MAX_LEN = IPv6_ADDRESS_LEN ,
            IPv4_ADDRESS_STRING_MAX_LEN = 15 ,
            IPv6_ADDRESS_STRING_MAX_LEN = 45 ,
            IP_ADDRESS_STRING_MAX_LEN = IPv6_ADDRESS_STRING_MAX_LEN ,
        } ;
            
        enum class EAddressFamily : unsigned short
        {   
            IPv4 , 
            IPv6 ,
            
        ENUM_END 
        } ;

        enum class ESocketType : unsigned short   
        {
            STREAM ,
            DATAGRAM ,
            SEQPACKET ,
            
        ENUM_END 
        } ;
                
        enum class EProtocol : unsigned short
        { 
            IP ,
            UDP ,
            TCP ,
        ENUM_END
        } ;
        
        using port_type = unsigned short ;
    } // ip
} // network

#endif
