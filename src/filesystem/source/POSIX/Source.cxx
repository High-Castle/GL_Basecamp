
struct DirColser
{
   void operator () ( DIR * d ) const { closedir( d ) ; }   
} ;

using dir_ptr = std::unique_ptr< DIR , DirColser > ;

namespace filesystem_utility
{

    std::string ls ( const char * path ) 
    {
        std::stringstream str ;
        auto directory = dir_ptr{ ::opendir( path ) } ;
        
        if ( directory.get() == NULL )
            throw ;
        
        for ( dirent * curr_file = ::readdir( directory ) ;
              curr_file != NULL ;
              curr_file = ::readdir( directory ) )
        {
            str << curr_file -> d_name ;
        }
        return str.str() ;
    }
        
    // parameter : 
    //      pointer to char array with path to the file.
    // return : 
    //      size of specified file in bytes.
    // exceptions :
    //      CFilesystemException if an error occured.
    std::size_t file_size ( const char * sz ) 
    {
        
    }
    
    std::size_t free_disk_mem () 
    {
        
    }

    // parameter : 
    //       pointer to char array representing the path.
    // return : 
    //      changes current directory to specified
    // exceptions :
    //      CFilesystemException if an error occured
    void cd ( const char * ) ;             
}
