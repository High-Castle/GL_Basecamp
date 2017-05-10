#include <string>
#include <sstream>
#include <memory>
#include <cstdint>
#include <cstddef>

#include <sys/statvfs.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include "filesystem_utility.hxx"

namespace 
{
    struct DirColser
    {
        void operator () ( DIR * d ) const { closedir( d ) ; }   
    } ;
    using dir_ptr = std::unique_ptr< DIR , DirColser > ;
}

namespace filesystem_utility
{
    std::string ls ( const char * path ) 
    {
        std::stringstream str ;
        auto directory = dir_ptr{ opendir( path ) } ;
        
        if ( directory.get() != NULL )
        {
            for ( dirent * curr_entry = readdir( directory.get() ) ;
                curr_entry != NULL ;
                curr_entry = readdir( directory.get() ) )
            {
                str << curr_entry -> d_name 
                    << ( curr_entry -> d_type == DT_DIR ? "/ " : " " ) ;
            }
            return str.str() ;
        }
        
        switch ( errno )
        {
            case EACCES  : throw CFilesystemException( "ls : Permission denied" ) ;
            case ENOENT  : throw CFilesystemException( "ls : Directory does not exist, or name is an empty string" ) ;
            case ENOMEM  : throw CFilesystemException( "ls : Insufficient memory to complete the operation" ) ;
            case ENOTDIR : throw CFilesystemException( "ls : name is not a directory" ) ;
        }
        throw CFilesystemException( "ls : cannot list the directory" ) ;
    }
        
    // parameter : 
    //      pointer to char array with path to the file.
    // return : 
    //      size of specified file in bytes.
    // exceptions :
    //      CFilesystemException if an error occured.
    std::size_t file_size ( const char * fname ) 
    {
        struct stat file_stat ;

        if ( stat( fname , &file_stat ) == 0 )
        {
            return file_stat.st_size ;
        }
   
        switch ( errno )
        {
            case EACCES  : throw CFilesystemException( "file_size : Permission denied" ) ;
            case ENOENT  : throw CFilesystemException( "file_size : Directory does not exist, or name is an empty string" ) ;
            case ENOMEM  : throw CFilesystemException( "file_size : Insufficient memory to complete the operation" ) ;
            case ENOTDIR : throw CFilesystemException( "file_size : name is not a directory" ) ;
        }
        throw CFilesystemException( "file_size : cannot list the directory" ) ;
    }

    // parameter : 
    //       pointer to char array representing the path.
    // return : 
    //      changes current directory to specified
    // exceptions :
    //      CFilesystemException if an error occured
    void cd ( const char * path )
    {
       if ( chdir( path ) == - 1 )
       {
           switch( errno )
           {
               case EACCES : throw CFilesystemException( "cd : Search permission is denied for one of the components of path" ) ; 
               case EIO    : throw CFilesystemException( "cd : An I/O error occurred" ) ;
               case ELOOP  : throw CFilesystemException( "cd : Too many symbolic links were encountered in resolving path" ) ;
               case ENAMETOOLONG : throw CFilesystemException( "cd : path is too long" ) ;
               case ENOENT : throw CFilesystemException( "cd : The file does not exist" ) ; 
               case ENOTDIR : throw CFilesystemException( "cd : A component of path is not a directory" ) ;
           }
           throw CFilesystemException( "cd : Cannot change directory" ) ;
       }
    }
}
