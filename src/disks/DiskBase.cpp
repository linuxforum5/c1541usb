#include "DiskBase.hpp"
#include<iostream>
#include "Prg.hpp"

using namespace std;

DiskBase::DiskBase() {
    deviceNumber = 8;
    diskLabel = "DISK BASE";
    openedFilename = "";
    prg = "";
}

bool DiskBase::setPrg( string prgFilename ) {
    prg = prgFilename;
    return true;
}

unsigned char DiskBase::getDeviceNumber() {
    return deviceNumber;
}

string DiskBase::getLabelLine() {
    int space_counter = 28 - diskLabel.length();
    // line length 23
    string line = "\x12\"";
    line.append( space_counter, ' ' );
    line.append( 1, '\"' );
    line.append( diskLabel );
    line.append( 1, '\"' ); // Invert face, "
    return line;
}

int DiskBase::getFilesCounter() {
    return 3;
}

Prg DiskBase::getFile( int i ) {
    string fn = "FILE NAME ";
    fn.append( 1, 49+i );
    return Prg( fn );
}

Prg* DiskBase::getOpenedFile() {
    if ( openedFilename.length() ) {
        return &openedFile;
    } else {
        printf( "File not open!\n" );
        exit(1);
    }
}

bool DiskBase::openFile( string filename ) {
    Prg prgfile( prg );
    if ( prgfile.exists() ) {
        openedFilename = filename;
        openedFile = prgfile;
        return true;
    } else {
        return false;
    }
}

unsigned short DiskBase::openedFileSize() {
    return openedFile.getSizeB();
}

int DiskBase::openedFilePosPercent() {
    return openedFile.getPosPercent();
}
