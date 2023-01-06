#ifndef DISKBASE_HPP
#define DISKBASE_HPP

#include <iostream>
#include "Prg.hpp"

using namespace std;

class DiskBase {
public:
    DiskBase();
    bool setPrg( string prgFilename );
    unsigned char getDeviceNumber();
    string getLabelLine();
    int getFilesCounter();
    Prg getFile( int i );
    Prg* getOpenedFile();
    bool openFile( string filename );
    unsigned short openedFileSize();
    int openedFilePosPercent();
private:
    unsigned char deviceNumber;
    string diskLabel;
    string openedFilename; // empty if not open
    Prg openedFile;
    string prg;
};

#endif // DISKBASE_HPP
