#ifndef PRGFILE_HPP
#define PRGFILE_HPP

#include "Disk.hpp"

using namespace std;

class PrgFile : public Disk {
public:
    PrgFile( Config *conf );
    virtual ~PrgFile();
    string getLabel(); // disk label
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

private:
    FILE* file; // to create
    unsigned short get_file_size( FILE* file );
    unsigned short get_file_size_from_name( string path, string name = "" );
    ByteArray seek( ByteArray filename );
    CBM::IOErrorMessage open_as( const char* fn, const char* mode, ByteArray realFilename );
    string cbm_filename_format( const char* filename );
    bool valid_filename( string filename );
    string file_get_contents( FILE* f );
    bool filename_match( ByteArray sfilename, string cbm_name );
    int file_counter; // 1 vagy 0
};

#endif // PRGFILE_HPP
