#ifndef FILE_TRANSFER_FILESYSTEM_HXX
#define FILE_TRANSFER_FILESYSTEM_HXX

#include <string>
#include <exception>


namespace filesystem_utility
{     
    std::string ls ( const char * ) ;
        
    // parameter : 
    //      pointer to char array with path to the file.
    // return : 
    //      size of specified file in bytes.
    // exceptions :
    //      CFilesystemException if an error occured.
    std::size_t file_size ( const char * ) ; 
    std::size_t free_disk_mem () ;

    // parameter : 
    //       pointer to char array representing the path.
    // return : 
    //      changes current directory to specified
    // exceptions :
    //      CFilesystemException if an error occured
    void cd ( const char * ) ;             
    void lock_file ( const char * ) ;
    int  unlock_file ( const char * ) noexcept ;

    void mkdir ( const char * ) ;
    
    struct CFilesystemException : std::runtime_error // TODO
    {
        using runtime_error::runtime_error ;
    } ;
}



#endif
