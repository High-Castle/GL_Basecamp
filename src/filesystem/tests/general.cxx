#include <iostream>
#include <cassert>

#include "filesystem_utility.hxx"

int main ( int args_n , char ** args ) 
{
    std::cout << filesystem_utility::ls( "./" ) << std::endl ;
    
    std::cout << filesystem_utility::file_size( args[ 0 ] ) << std::endl ;
    
    filesystem_utility::cd( "../" ) ;
    
    std::cout << filesystem_utility::ls( "./" ) << std::endl ;
}
