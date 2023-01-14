#include "PrgFile.hpp"
#include <iostream>
#include <cstring>
#include <dirent.h>

using namespace std;

PrgFile::PrgFile( Config *conf ) : Disk( conf ) {
    file = NULL;
    file_counter = ( valid_filename( config->get_media() ) ) ? 1 : 0;
    printf( "PRG file %s mounted as a disk\n", media_path.c_str() );
}

PrgFile::~PrgFile() { closeFile(); }

string PrgFile::file_get_contents( FILE* f ) {
    int size = get_file_size( f );
    char buff[ size+1 ];
    buff[ size ] = 0;
    fread( buff, size, 1, f );
    return buff;
}

bool PrgFile::valid_filename( string filename ) {
    int len = filename.length();
    if ( len > 4 ) {
        return CBM::fn2cbm( filename.substr( len-4 ).c_str() ) == ".PRG";
    } else
        return false;
}

bool PrgFile::isDiskWriteProtected() { return true; }

int PrgFile::getFilesCounter() { return file_counter; }

Disk::FileEntry PrgFile::getCbmFileEntry( int i ) {
    FileEntry fe = { "", "", 0 };
    if ( file_counter ) { // File is available
        string media_basename = media_path.substr( media_path.find_last_of( '/' ) + 1 );
        int len = media_basename.length();
        string basename = media_basename.substr( 0, len-4 );
        string type = media_basename.substr( len-3 );
        fe.filename = CBM::fn2cbm( basename.c_str() );
        fe.type3 = CBM::fn2cbm( type.c_str() );
        fe.size = get_file_size_from_name( media_path ) - 2; // .PRG file is bigger with 2 bytes than CBM file
    }
    return fe;
}

bool PrgFile::filename_match( ByteArray sfilename, string cbm_name ) { return true; }

ByteArray PrgFile::seek( ByteArray sfilename ) { return config->get_media(); }

CBM::IOErrorMessage PrgFile::openFile( ByteArray sfilename, OpenMode mode ) {
    ByteArray realFilename;
    if ( mode == READ || mode == OVERWRITE ) {
        printf( "Search for file pattern '%s'\n", sfilename.c_str() );
        realFilename = seek( sfilename );
        printf( "Found file '%s' size=%d\n", realFilename.c_str(), get_file_size_from_name( realFilename.to_string() ) );
    } else {
        realFilename = sfilename;
        realFilename.append( ".PRG" );
    }
    if ( realFilename.length() ) {
        const char* fn = realFilename.c_str();
        bool overwrite = false;
        switch( mode ) {
            case OVERWRITE : overwrite = true;
            case CREATE : return open_as( fn, "wb", realFilename ); break;
            case READ : return open_as( fn, "rb", realFilename ); break;
        }
        return CBM::ErrUnknownError;
    } else {
        return CBM::ErrFileNotFound;
    }
}

CBM::IOErrorMessage PrgFile::open_as( const char* fn, const char* mode, ByteArray realFilename ) {
    if ( file = fopen( fn, mode ) ) {
        opened.filename = CBM::fn2cbm( realFilename.mid( 0, -4 ).c_str() );
        opened.type3 = CBM::fn2cbm( realFilename.mid( -3 ).c_str() );
        opened.size = get_file_size( file );
        return CBM::ErrOK;
    } else
        return CBM::ErrUnknownError;
}

unsigned short PrgFile::get_file_size( FILE* file ) {
    unsigned short size = 0;
    fseek( file, 0L, SEEK_END );
    size = ftell( file );
    fseek( file, 0L, SEEK_SET );
    return size;
}

unsigned short PrgFile::get_file_size_from_name( string path, string name ) {
    string fn = name.length() ? path.append("/").append( name ) : path;
    if ( FILE *f = fopen( fn.c_str(), "rb" ) ) {
        unsigned short size = get_file_size( f );
        fclose( f );
        return size;
    } else {
        return 0;
    }
}

void PrgFile::write( ByteArray data ) {
    const unsigned char* d = data.uc_str();
    printf( "**** Write %d bytes into file\n", data.length() );
    fwrite( d, data.length(), 1, file );
}

void PrgFile::closeFile() {
    Disk::closeFile();
    if ( file != NULL ) {
        fclose( file );
        file = NULL;
    }
}

int PrgFile::openedFilePosPercent() {
    return 100 * ftell( file ) / opened.size;
}

unsigned char PrgFile::getc() {
    if ( isEOF() ) {
        return 0;
    } else {
        return (unsigned char)fgetc( file );
    }
}

bool PrgFile::isEOF() { return ftell( file ) >= opened.size; }

Disk::FileEntry PrgFile::getOpenedFileEntry() { return opened; }
