#include <string>
#include <sstream>
#include <memory>
#include <cstdint>
#include <cstddef>

#include "filesystem_utility.hxx" 

#include <windows.h>
#include <sys/types.h>

namespace filesystem_utility
{
    std::string ls ( const char * path ) 
    {
        std::stringstream str ;        
        auto path_to_list = std::string( path ) + "/\*" ;
        WIN32_FIND_DATA found_data ;
        
        HANDLE find_handler = FindFirstFile( path_to_list.c_str() , &found_data ) ;
        
        if ( find_handler != INVALID_HANDLE_VALUE )
        {
            do
            {
                if ( found_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                {
                    str << found_data.cFileName << "/ " ;
                }
                else
                {
                    str << found_data.cFileName << " " ;
                }
            }
            while ( FindNextFile( find_handler , &found_data ) != 0 ) ;
            
            return str.str() ;
        }
        
        switch ( GetLastError() )
        {

        }
        throw CFilesystemException( "ls : cannot list the directory" ) ;
    }
        
    std::size_t file_size ( const char * fname ) 
    {
        struct _stat64 the_stat ;
        
        if ( _stat64( fname , &the_stat ) != -1 )
        {
            return the_stat.st_size  ;
        }
   
        switch ( GetLastError() )
        {

        }
        throw CFilesystemException( "file_size : cannot get file's size" ) ;
    }

    void cd ( const char * path )
    {
       if ( ! SetCurrentDirectory( path ) )
       {
           switch( GetLastError() )
           {

           }
           throw CFilesystemException( "cd : Cannot change dir" ) ;
       }
    }
}
