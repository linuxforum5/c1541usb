#ifndef NATIVEFS_HPP
#define NATIVEFS_HPP

#include <iostream>
// #include "Prg.hpp"
#include "../comm/QByteArray.hpp"
#include "../comm/CBM.hpp"

using namespace std;

struct FileEntry {
    string filename;
    string type3;
    unsigned short size;
};

class NativeFs {
public:
    enum OpenMode {
        READ = 0,
        CREATE = 1,
        OVERWRITE = 2
    };

    NativeFs();
    void setPath( string path );
    unsigned char getDeviceNumber(); // Disk device number
    string getLabel(); // disk label
    int getFilesCounter(); // Files in disk
    FileEntry getCbmFileEntry( int index ); // A CBM szerinti fájlmérettel!
    bool isDiskWriteProtected();

    CBM::IOErrorMessage openFile( QByteArray filename, OpenMode mode );
    FileEntry getOpenedFileEntry();
    void write( QByteArray data ); // Error if file opened as read
    unsigned char getc(); // read from opened file, if opened as read else error
    bool isEOF(); // File open and eof
    
    int openedFilePosPercent();
    void closeFile();
    

private:
    string diskLabel;
    FileEntry opened;
    FILE* file; // to create
    unsigned short get_file_size( FILE* file );
    unsigned short get_file_size_from_name( string path, string name );
    QByteArray seek( QByteArray filename );
    string cbm_filename_format( string filename );
    bool valid_filename( string filename );
    string file_get_contents( FILE* f );
    bool filename_match( QByteArray sfilename, string cbm_name );
    string base_path;
};

#endif // NATIVEFS_HPP
