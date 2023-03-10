#ifndef NATIVEFS_HPP
#define NATIVEFS_HPP

/**
 * Egy mappát kezel egy commodore lemezként.
 */

#include "Disk.hpp"

using namespace std;

class NativeFs : public Disk {
public:
    NativeFs( Config *conf );
    virtual ~NativeFs();
    int getFilesCounter(); // Files in disk
    FileEntry getCbmFileEntry( int index ); // A CBM szerinti fájlmérettel!
    bool isDiskWriteProtected();

    CBM::IOErrorMessage openFile( ByteArray filename, OpenMode mode );
    FileEntry getOpenedFileEntry();
    void write( ByteArray data ); // Error if file opened as read
    unsigned char getc(); // read from opened file, if opened as read else error
    bool isEOF(); // File open and eof

    int openedFilePosPercent();
    void closeFile();

    static bool is_dir( string filename );

private:
    FILE* file; // to create
    unsigned short get_file_size( FILE* file );
    unsigned short get_file_size_from_name( string path, string name );
    ByteArray seek( ByteArray filename );
    CBM::IOErrorMessage open_as( const char* fn, const char* mode, ByteArray realFilename );
    string cbm_filename_format( const char* filename );
    bool valid_filename( string filename );
    string file_get_contents( FILE* f );
    bool filename_match( ByteArray sfilename, string cbm_name );
};

#endif // NATIVEFS_HPP
