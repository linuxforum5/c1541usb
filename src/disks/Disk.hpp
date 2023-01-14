#ifndef DISK_HPP
#define DISK_HPP

#include <iostream>
#include "../comm/ByteArray.hpp"
#include "../comm/CBM.hpp"
#include "../comm/Config.hpp"

using namespace std;

class Disk {
public:
    struct FileEntry {
        string filename;
        string type3;
        unsigned short size;
    };
    enum OpenMode { READ, CREATE, OVERWRITE };

    Disk( Config *conf );
//    virtual ~Disk();
    string getLabel(); // disk label
    // Disk methods
    virtual int getFilesCounter() = 0; // Files in disk
    virtual bool isDiskWriteProtected() = 0;
    // File methods
    FileEntry getOpenedFileEntry();
    virtual unsigned char getc() = 0; // read from opened file, if opened as read else error
    virtual void write( ByteArray data ) = 0; // Error if file opened as read
    virtual FileEntry getCbmFileEntry( int index ) = 0; // A CBM szerinti fájlmérettel!
    virtual CBM::IOErrorMessage openFile( ByteArray filename, OpenMode mode ) = 0;
    virtual bool isEOF() = 0; // File open and eof
    virtual int openedFilePosPercent() = 0;
    virtual void closeFile();
/*
    unsigned char getDeviceNumber(); // Disk device number

*/
protected:
    string diskLabel;
    FileEntry opened;
    string media_path;
    Config *config;
};

#endif // DISK_HPP
