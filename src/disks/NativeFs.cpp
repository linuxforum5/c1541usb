#include "NativeFs.hpp"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <dirent.h>

using namespace std;

NativeFs::NativeFs() {
    diskLabel = "DISK BASE";
    opened = { "", "", 0 };
    file = NULL;
    base_path = "";
}

void NativeFs::setPath( string path ) {
    base_path = path;
    closeFile();
    if ( FILE* f = fopen( path.append( ".label" ).c_str(), "r" ) ) {
        diskLabel = file_get_contents( f );
        fclose( f );
    }
}

string NativeFs::file_get_contents( FILE* f ) {
    int size = get_file_size( f );
    char buff[ size+1 ];
    buff[ size ] = 0;
    fread( buff, size, 1, f );
    return buff;
}

bool NativeFs::isDiskWriteProtected() { return false; }

string NativeFs::getLabel() { return diskLabel; }

bool NativeFs::valid_filename( string filename ) {
    int len = filename.length();
    if ( len > 4 ) {
//        return filename.substr( len-4 ) == ".PRG" || filename.substr( len-4 ) == ".prg";
        return boost::to_upper_copy( filename.substr( len-4 ) ) == ".PRG";
    } else
        return false;
}

int NativeFs::getFilesCounter() {
    int cnt = 0;
    struct dirent *entry = nullptr;
    DIR *dp = nullptr;
    dp = opendir( base_path.c_str() );
    if ( dp != nullptr ) {
        while ( ( entry = readdir(dp) ) ) {
            if ( valid_filename( entry->d_name ) ) cnt++;
        }
    }
    closedir(dp);
    return cnt;
}

FileEntry NativeFs::getCbmFileEntry( int i ) {
    FileEntry fe = { "", "", 0 };
    struct dirent *entry = nullptr;
    DIR *dp = nullptr;

    dp = opendir( base_path.c_str() );
    if ( dp != nullptr ) {
//printf( "Open dir '%s'. Search for index: %d\n", base_path.c_str(), i );
        int cnt = 0;
        while ( !fe.size && ( entry = readdir(dp) ) ) {
//printf( "Entry '%s' found\n", entry->d_name );
            if ( valid_filename( entry->d_name ) ) {
//printf( "Entry '%s' is valid. It is the %d. file.\n", entry->d_name, cnt );
                if ( cnt++ == i ) {
//printf( "Found %d. item\n", i );
                    int len = strlen( entry->d_name );
//printf( "Filename length is %d\n", len );
                    string basename = string(entry->d_name).substr( 0, len-4 );
//printf( "Basename is '%s'\n", basename.c_str() );
                    string type = string(entry->d_name).substr( len-3 );
//printf( "Type3 is '%s'\n", type.c_str() );
                    fe.filename = cbm_filename_format( basename );
                    fe.type3 = cbm_filename_format( type );
                    fe.size = get_file_size_from_name( base_path, entry->d_name ) - 2; // .PRG file is bigger with 2 bytes than CBM file
//printf( "CBM size is %d\n", fe.size );
                }
            }
        }
    }
    closedir(dp);
    return fe;
}

string NativeFs::cbm_filename_format( string filename ) {
    return boost::to_upper_copy( filename ); // 160=>' ', max length=23, 
}

bool NativeFs::filename_match( QByteArray sfilename, string cbm_name ) {
    int slen = sfilename.length();
// printf( "Search length: %d\n", slen );
    if ( slen > 0 ) {
// printf( "SEEK '%s' check '%s'\n", sfilename.c_str(), cbm_name.c_str() );
        if ( sfilename.at( slen-1 ) == '*' ) {
            slen--;
            string sprefix = sfilename.mid( 0, -1 ).to_string();
            string cbm_prefix = cbm_name.substr( 0, slen );
// printf( "* PREFIX '%s' check '%s'\n", sprefix.c_str(), cbm_prefix.c_str() );
            return sprefix == cbm_prefix;
        } else {
            return sfilename.to_string() == cbm_name;
        }
    } else {
        return false;
    }
}

QByteArray NativeFs::seek( QByteArray sfilename ) {
    string realFilename = "";
    struct dirent *entry = nullptr;
    DIR *dp = nullptr;
    dp = opendir( base_path.c_str() );
    if ( dp != nullptr ) {
        int cnt = 0;
        while ( ( realFilename.length() == 0 ) && ( entry = readdir(dp) ) ) {
            if ( valid_filename( entry->d_name ) ) {
                int len = strlen( entry->d_name );
                string cbm_name = cbm_filename_format( string(entry->d_name).substr( 0, len-4 ) );
                if ( filename_match( sfilename, cbm_name ) ) {
                    realFilename = string( entry->d_name);
                }
            }
        }
    }
    closedir(dp);
    return realFilename;
}

CBM::IOErrorMessage NativeFs::openFile( QByteArray sfilename, OpenMode mode ) {
    QByteArray realFilename;
    if ( mode == READ || mode == OVERWRITE ) {
        printf( "Search for file pattern '%s'\n", sfilename.c_str() );
        realFilename = seek( sfilename );
        printf( "Found file '%s' (length=%d)\n", realFilename.c_str(), realFilename.length() );
    } else {
        realFilename = sfilename;
        realFilename.append( ".PRG" );
    }
    if ( realFilename.length() ) {
	string ffn = base_path;
        const char* fn = ffn.append("/").append( realFilename.to_string() ).c_str();
//printf( "Full filename '%s'\n", fn );
        bool overwrite = false;
        switch( mode ) {
            case OVERWRITE :
                overwrite = true;
            case CREATE :
//printf( "Mode = %s\n", overwrite ? "OVERWRITE" : "CREATE" );
                if ( file = fopen( fn, "wb" ) ) {
//printf( "Create success\n" );
                    opened.filename = cbm_filename_format( realFilename.mid( 0, -4 ).to_string() );
                    opened.type3 = cbm_filename_format( realFilename.mid( -3 ).to_string() );
                    opened.size = get_file_size( file );
//                    printf( "**** Open to %s '%s' file\n", overwrite ? "overwrite" : "create", fn );
                    return CBM::ErrOK;
                } else {
//printf( "Create error\n" );
                    return CBM::ErrUnknownError;
                }
                break;
            case READ :
//printf( "Open read '%s'\n", fn );
                if ( file = fopen( fn, "rb" ) ) {
//printf( "Open is success\n", fn );
                    opened.filename = cbm_filename_format( realFilename.mid( 0, -4 ).to_string() );
                    opened.type3 = cbm_filename_format( realFilename.mid( -3 ).to_string() );
                    opened.size = get_file_size( file );
//                    printf( "**** Open to read '%s'.'%s' file (size=%d)\n", opened.filename.c_str(), opened.type3.c_str(), opened.size );
                    return CBM::ErrOK;
                } else
                    return CBM::ErrUnknownError;
                break;
        }
        return CBM::ErrUnknownError;
    } else {
        return CBM::ErrFileNotFound;
    }
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

void NativeFs::write( QByteArray data ) {
    const unsigned char* d = data.uc_str();
    printf( "**** Write %d bytes into file\n", data.length() );
    fwrite( d, data.length(), 1, file );
}

void NativeFs::closeFile() {
    if ( file != NULL ) {
        fclose( file );
        file = NULL;
        opened = { "", "", 0 };
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

FileEntry NativeFs::getOpenedFileEntry() { return opened; }
