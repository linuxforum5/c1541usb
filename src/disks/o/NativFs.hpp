#ifndef DISKBASE_HPP
#define DISKBASE_HPP

#include <iostream>
#include "DiskBase.hpp"

using namespace std;

class NativeFs : DiskBase {
    public:
        NativeFs();
        

    bool setPrg( string prgFilename );
    unsigned char getDeviceNumber();
    string getLabelLine();
    int getFilesCounter();
    Prg getFile( int i );
    Prg* getOpenedFile();
    CBM::IOErrorMessage fopenWrite( QByteArray filename, bool overwrite );
    void write( QByteArray data );
    bool openFile( string filename );
    bool isDiskWriteProtected();
    unsigned short openedFileSize();
    int openedFilePosPercent();
    string getOpenedFilename();
    void closeFile();
private:
    unsigned char deviceNumber;
    string diskLabel;
    string openedFilename; // empty if not open
    Prg openedFile;
    string prg;
    FILE* file; // to create
};

#endif // DISKBASE_HPP
