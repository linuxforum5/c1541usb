#include "NativeFs.hpp"
#include <iostream>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
// #include <filesystem> // C++17

// namespace fs = std::filesystem

using namespace std;

NativeFs::NativeFs( Config *conf ) : Disk( conf ) {
    file = NULL;
    string path = config->get_media();
    if ( FILE* f = fopen( path.append( "/" ).append( ".label" ).c_str(), "r" ) ) {
        diskLabel = file_get_contents( f );
        fclose( f );
    }
    printf( "Directory %s mounted as a disk\n", media_path.c_str() );
}

NativeFs::~NativeFs() {
    closeFile();
}

bool NativeFs::is_dir( string filename ) {
    struct stat s;
    if ( stat( filename.c_str(), &s ) == 0 ) {
        if( s.st_mode & S_IFDIR ) {
            return true; //it's a directory
        } else if( s.st_mode & S_IFREG ) {
            return false; //it's a file
        } else {
            return false; //something else
        }
    } else {
        return false; //error
    }

}

string NativeFs::file_get_contents( FILE* f ) {
    int size = get_file_size( f );
    char buff[ size+1 ];
    buff[ size ] = 0;
    fread( buff, size, 1, f );
    return buff;
}

bool NativeFs::valid_filename( string filename ) {
    int len = filename.length();
    if ( len > 4 ) {
        return CBM::fn2cbm( filename.substr( len-4 ).c_str() ) == ".PRG";
    } else
        return false;
}

bool NativeFs::isDiskWriteProtected() { return false; }

int NativeFs::getFilesCounter() {
    int cnt = 0;
    struct dirent *entry = nullptr;
    DIR *dp = nullptr;
    dp = opendir( media_path.c_str() );
    if ( dp != nullptr ) {
        while ( ( entry = readdir(dp) ) ) {
            if ( valid_filename( entry->d_name ) ) cnt++;
        }
    }
    closedir(dp);
    return cnt;
}

Disk::FileEntry NativeFs::getCbmFileEntry( int i ) {
    FileEntry fe = { "", "", 0 };
    struct dirent *entry = nullptr;
    DIR *dp = nullptr;

    dp = opendir( media_path.c_str() );
    if ( dp != nullptr ) {
        int cnt = 0;
        while ( !fe.size && ( entry = readdir(dp) ) ) {
            if ( valid_filename( entry->d_name ) ) {
                if ( cnt++ == i ) {
                    int len = strlen( entry->d_name );
                    string basename = string(entry->d_name).substr( 0, len-4 );
                    string type = string(entry->d_name).substr( len-3 );
                    fe.filename = CBM::fn2cbm( basename.c_str() );
                    fe.type3 = CBM::fn2cbm( type.c_str() );
                    fe.size = get_file_size_from_name( media_path, entry->d_name ) - 2; // .PRG file is bigger with 2 bytes than CBM file
                }
            }
        }
    }
    closedir(dp);
    return fe;
}

bool NativeFs::filename_match( ByteArray sfilename, string cbm_name ) {
    int slen = sfilename.length();
    if ( slen > 0 ) {
        if ( sfilename.at( slen-1 ) == '*' ) {
            slen--;
            string sprefix = sfilename.mid( 0, -1 ).to_string();
            string cbm_prefix = cbm_name.substr( 0, slen );
            return sprefix == cbm_prefix;
        } else {
            return sfilename.to_string() == cbm_name;
        }
    } else {
        return false;
    }
}

ByteArray NativeFs::seek( ByteArray sfilename ) {
    string realFilename = "";
    struct dirent *entry = nullptr;
    DIR *dp = nullptr;
    dp = opendir( media_path.c_str() );
    if ( dp != nullptr ) {
        int cnt = 0;
        while ( ( realFilename.length() == 0 ) && ( entry = readdir(dp) ) ) {
            if ( valid_filename( entry->d_name ) ) {
                int len = strlen( entry->d_name );
                string cbm_name = CBM::fn2cbm( string(entry->d_name).substr( 0, len-4 ).c_str() );
                if ( filename_match( sfilename, cbm_name ) ) {
                    realFilename = string( entry->d_name);
                }
            }
        }
    }
    closedir(dp);
    return realFilename;
}

CBM::IOErrorMessage NativeFs::openFile( ByteArray sfilename, OpenMode mode ) {
    ByteArray realFilename;
    if ( mode == READ || mode == OVERWRITE ) {
        printf( "Search for file pattern '%s'\n", sfilename.c_str() );
        realFilename = seek( sfilename );
        printf( "Found file '%s' (length=%d)\n", realFilename.c_str(), realFilename.length() );
    } else {
        realFilename = sfilename;
        realFilename.append( ".PRG" );
    }
    if ( realFilename.length() ) {
	string ffn = media_path;
        const char* fn = ffn.append("/").append( realFilename.to_string() ).c_str();
        bool overwrite = false;
        switch( mode ) {
            case OVERWRITE :
                overwrite = true;
            case CREATE : return open_as( fn, "wb", realFilename ); break;
            case READ : return open_as( fn, "rb", realFilename ); break;
        }
        return CBM::ErrUnknownError;
    } else {
        return CBM::ErrFileNotFound;
    }
}

CBM::IOErrorMessage NativeFs::open_as( const char* fn, const char* mode, ByteArray realFilename ) {
    if ( file = fopen( fn, mode ) ) {
        opened.filename = CBM::fn2cbm( realFilename.mid( 0, -4 ).c_str() );
        opened.type3 = CBM::fn2cbm( realFilename.mid( -3 ).c_str() );
        opened.size = get_file_size( file );
        return CBM::ErrOK;
    } else
        return CBM::ErrUnknownError;
}

unsigned short NativeFs::get_file_size( FILE* file ) {
    unsigned short size = 0;
    fseek( file, 0L, SEEK_END );
    size = ftell( file );
    fseek( file, 0L, SEEK_SET );
    return size;
}

unsigned short NativeFs::get_file_size_from_name( string path, string name ) {
    if ( FILE *f = fopen( path.append("/").append( name ).c_str(), "rb" ) ) {
        unsigned short size = get_file_size( f );
        fclose( f );
        return size;
    } else {
        return 0;
    }
}

void NativeFs::write( ByteArray data ) {
    const unsigned char* d = data.uc_str();
    printf( "**** Write %d bytes into file\n", data.length() );
    fwrite( d, data.length(), 1, file );
}

void NativeFs::closeFile() {
    Disk::closeFile();
    if ( file != NULL ) {
        fclose( file );
        file = NULL;
    }
}

int NativeFs::openedFilePosPercent() {
    return 100 * ftell( file ) / opened.size;
}

unsigned char NativeFs::getc() {
    if ( isEOF() ) {
        return 0;
    } else {
        return (unsigned char)fgetc( file );
    }
}

bool NativeFs::isEOF() { return ftell( file ) >= opened.size; }

Disk::FileEntry NativeFs::getOpenedFileEntry() { return opened; }
