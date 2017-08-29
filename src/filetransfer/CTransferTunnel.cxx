#include <istream>
#include <chrono>
#include <exception>
#include <memory>
#include <utility>
#include <limits>
#include <functional>

#include <iostream> // cerr, debug

#include "Types.hxx"
#include "CSocket.hxx"
#include "CTransferTunnel.hxx"

namespace transfer_protocol 
{
    namespace 
    {
       
        

        // note : http://stackoverflow.com/questions/8720425/array-placement-new-requires-unspecified-overhead-in-the-buffer
        //        ( that is why i did not use operator new directly ).
        
        // note : uint8_t * -> unsigned char * = OK , but unsigned char * -> uint8_t * = UB in general, 
        // since CHAR_BIT is not always 8 ( 8 == CHAR_BIT is required by POSIX, though ); let non-POSIX world and other exotics to be area for experiments.
        
        class CPackageBuffer final  // to avoid UB, since accessing char via non char pointer breaks strict aliasing rules.
        {
            using CDataHeaderPOD = CTransferTunnel_TCP::CDataHeaderPOD ;
            public :
                CPackageBuffer( std::size_t data_size ) 
                    : pack_sz_( sizeof( CDataHeaderPOD ) + data_size )
                {
                    /* 
                    * [5.3.4/10] "... For `arrays` of `char and unsigned char`, the difference between the result of the new-expression and the address 
                    * returned by the allocation function shall be an integral multiple of the strictest fundamental alignment requirement (3.9) of 
                    * any object type whose size is no greater than the size of the array being created. 
                    */
                    auto buffer = new unsigned char[ pack_sz_ ] ; 
                    header_ = new ( buffer ) CDataHeaderPOD ; 
                }
                
                CPackageBuffer ( const CPackageBuffer& ) = delete ; 
                CPackageBuffer ( CPackageBuffer&& ) = delete ;
                CPackageBuffer& operator = ( const CPackageBuffer& ) = delete ;
                CPackageBuffer& operator = ( CPackageBuffer&& ) = delete ;
                
                CDataHeaderPOD * header () noexcept { return header_ ; }
                unsigned char * data () noexcept { return reinterpret_cast< unsigned char * >( header_ + 1 ) ; }
                unsigned char * package () noexcept { return reinterpret_cast< unsigned char * >( header_ ) ; }
                
                CDataHeaderPOD const * header () const noexcept { return header_ ; }
                unsigned char const * data () const noexcept { return reinterpret_cast< unsigned char const * >( header_ + 1 ) ; }
                unsigned char const * package () const noexcept { return reinterpret_cast< unsigned char const * >( header_ ) ; }
                
                std::size_t size () const noexcept { return pack_sz_ ; }
                std::size_t data_size () const noexcept { return pack_sz_ - sizeof( CDataHeaderPOD ) ; }
                
                ~ CPackageBuffer ()
                {
                    header() -> ~CDataHeaderPOD () ;  // trivial
                    delete [] ( unsigned char * ) header_ ; // non-array placement, does not perform any padding, header_ == buffer
                }
                
            private :
                CDataHeaderPOD * header_ ;
                std::size_t const pack_sz_ ;
        } ;
    } 
    
    void CTransferTunnel_TCP::to_network( CDataHeaderPOD * data , std::uint32_t data_sz , std::uint8_t eof ) 
    {
        data -> header.type = DATA ;
        data -> size = network::hton_l( data_sz ) ;
        data -> eof  = eof ; 
    }
    
    void CTransferTunnel_TCP::from_network( const CDataHeaderPOD * data , std::uint32_t& data_sz , std::uint8_t& eof ) 
    {
        data_sz = network::ntoh_l( data -> size ) ;
        eof     = data -> eof ; 
    }

    
    namespace
    {
        enum Polynom32 : std::uint32_t
        { 
            ETHERNET = 0x04C11DB7
        } ;

        // references : [1] http://www.zlib.net/crc_v3.txt, 
        //              [2] http://create.stephan-brumme.com/crc32/ 
        //              [3] https://www.cs.jhu.edu/~scheideler/courses/600.344_S02/CRC.html 
        //              [4] https://en.wikipedia.org/wiki/Mathematics_of_cyclic_redundancy_checks
        std::uint32_t crc32 ( const unsigned char * const data , std::size_t sz )
        { // note it doesn't implement any standard, just for simple check
            enum { BITS_IN_BYTE = std::numeric_limits< unsigned char >::digits } ; 
            enum { POLY = ETHERNET } ;
            
            std::uint32_t acc = 0 ; 
            
            // division [1]
            for ( unsigned char const * current = data ; 
                current < data + sz ; 
                ++ current )
            {
                acc ^= * current ; // XOR-add/sub 
                
                for ( unsigned char each_bit = BITS_IN_BYTE ; each_bit ; -- each_bit ) // TODO : table approach
                {
                    acc = ( acc >> 1 ) /* reversed bit order, really doesn't matter here; both recv and send are doing same thing on the same level */ 
                                    ^ ( ( acc & 1 ) * POLY ) ; /*  if ( reversed >= poly_part ) then ( reversed - poly )
                                                                    ( note that >= and - are operators of non-carry arithmetic [1] ) ) */ 
                }
            }
            return acc ;
        }

        
        void send_data_pack( CPackageBuffer& buff , network::ip::CSocket& peer , unsigned attempts )
        {
            using namespace network::ip ;
            buff.header() -> checksum = 0 ;
            buff.header() -> checksum = calc_checksum_hton( buff.package() , buff.size() ) ;
            
            for ( ; ; -- attempts ) 
            {
                std::size_t written ;
                peer.write_all( buff.package() , buff.size() , written ) ;  
                CHeaderPOD remote_is_done ; // rename
                peer.read( ( unsigned char * ) &remote_is_done , sizeof( CHeaderPOD ) ) ; // 1 byte pack
                
                if ( remote_is_done.type == APPROVE ) break ;
                if ( ! attempts ) 
                    throw CTransferException( "Error while transfering, checksum doesn't match" ) ;
                std::cerr << "\npackage has been dropped" ; 
            } 
        }
        
        void recv_data_pack( CPackageBuffer& buff , network::ip::CSocket& peer , unsigned attempts )
        {
            using namespace network::ip ;
            for ( ; ; -- attempts ) 
            {
                peer.read( buff.package() , buff.size() , { EReadFlags::WHAIT_ALL } ) ;
                
                std::uint32_t remote_sum = buff.header() -> checksum ;
                buff.header() -> checksum = 0 ;
                std::uint32_t sum = calc_checksum_hton( buff.package() , buff.size() ) ;
                

                CHeaderPOD im_ready { remote_sum == sum ? APPROVE : PACK_CANCEL } ;
                std::size_t written ;
                peer.write_all( ( unsigned char * ) &im_ready , sizeof( CHeaderPOD ) , written ) ; // 1 byte pack
                
                if ( im_ready.type == APPROVE ) break ;
                if ( ! attempts )
                    throw CTransferException( "Error while transfering, checksum doesn't match" ) ;
                std::cerr << "\npackage has been dropped" ; 
            }
        }
    }
    
    std::uint32_t calc_checksum_hton( const unsigned char * data , std::size_t sz )
    {
        return network::hton_l( crc32( data , sz ) ) ;
    }
    
    void CTransferTunnel_TCP::send_stream ( std::istream& stream , network::ip::CSocket& peer ,
                                            std::size_t pack_data_sz , 
                                            unsigned attempts )
    {
        using namespace network::ip ;
        
        if ( pack_data_sz == 0 )
            throw std::invalid_argument( "size of data in package cannot be zero" ) ;
        
        CPackageBuffer buff { pack_data_sz } ;
        
        for ( ; stream.good() ; ) 
        { 
            std::streamsize bytes_to_wirte = stream.read( ( char * ) buff.data() , pack_data_sz ).gcount() ;
            to_network( buff.header() , ( std::uint32_t ) bytes_to_wirte , stream.eof() ) ; 
            send_data_pack( buff , peer , attempts ) ;
        } 
    }
    
    void CTransferTunnel_TCP::recv_stream( std::ostream& stream , network::ip::CSocket& peer , 
                                           std::size_t pack_data_sz ,  
                                           unsigned attempts )
    {
        using namespace network::ip ;
        
        if ( pack_data_sz == 0 )
            throw std::invalid_argument( "size of data in package cannot be zero" ) ;
        
        CPackageBuffer buff { pack_data_sz } ;

        for ( ; ; ) 
        {
            std::uint32_t dsize ; std::uint8_t eof ;
            recv_data_pack( buff , peer , attempts ) ;
            from_network( buff.header() , dsize , eof ) ;
            stream.write( ( char * ) buff.data() , dsize ) ;
            if ( eof ) break ; 
        }
    }
    
    bool CTransferTunnel_TCP::send_amount_from_stream( std::istream& stream , 
                                                       network::ip::CSocket& peer ,
                                                       std::size_t data_size ,
                                                       std::size_t pack_data_sz ,
                                                       unsigned attempts )
    {        
        if ( pack_data_sz == 0 )
            throw std::invalid_argument( "size of data in package cannot be zero" ) ;
        
        CPackageBuffer buff { pack_data_sz } ;
        
        std::size_t remainder = data_size % pack_data_sz ;
        unsigned long num_of_packages = data_size / pack_data_sz ;
        unsigned long remainder_num_of_packs = remainder ? 1 : 0 ;
        for ( const auto& loop : { std::make_pair( num_of_packages , pack_data_sz ) ,
                                   std::make_pair( remainder_num_of_packs , remainder ) } )
        {
            for ( unsigned long packs = 0 ; packs != loop.first ; ++ packs ) 
            { 
                std::streamsize bytes_to_wirte = stream.read( ( char * ) buff.data() , loop.second ).gcount() ;

                to_network( buff.header() , ( std::uint32_t ) bytes_to_wirte , stream.eof() ) ;
                send_data_pack( buff , peer , attempts ) ;

                if ( ! stream.good() ) return data_size == 0 ; 
            } // send packages
        }
        return true ;
    }
    
    // TODO : make themselfs streams.
    
    // false if eof received
    bool CTransferTunnel_TCP::recv_amount_to_stream( std::ostream& stream , 
                                                     network::ip::CSocket& peer ,
                                                     std::size_t const data_sz ,
                                                     std::size_t pack_data_sz ,
                                                     unsigned attempts )
    {
        using namespace network::ip ;
        
        if ( pack_data_sz == 0 )
            throw std::invalid_argument( "size of data in package cannot be zero" ) ;
        
        CPackageBuffer buff { pack_data_sz } ;
        
        std::size_t totally_received = 0 ;
        for ( ; totally_received < data_sz ; ) 
        {
            std::uint32_t dsize ; std::uint8_t eof ;
            
            recv_data_pack( buff , peer , attempts ) ;
            
            from_network( buff.header() , dsize , eof ) ;
            stream.write( ( char * ) buff.data() , dsize ) ;
            
            totally_received += dsize ;
            
            if ( eof ) return false ; 
        }
        return true ;
    }
}

