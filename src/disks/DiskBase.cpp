#include "DiskBase.hpp"
#include<iostream>
#include "Prg.hpp"

using namespace std;

DiskBase::DiskBase() {
    deviceNumber = 8;
    diskLabel = "DISK BASE";
    openedFilename = "";
    prg = "";
    file = NULL;
}

bool DiskBase::setPrg( string prgFilename ) {
    prg = prgFilename;
    return true;
}

unsigned char DiskBase::getDeviceNumber() {
    return deviceNumber;
}

bool DiskBase::isDiskWriteProtected() { return false; }

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

CBM::IOErrorMessage DiskBase::fopenWrite( QByteArray filename, bool overwrite ) {
    file = fopen( filename.append(".prg").c_str(), "wb" );
    openedFilename = filename.to_string();
    printf( "**** Open to %s '%s' file\n", overwrite ? "overwrite" : "create", filename.c_str() );
    return CBM::ErrOK;
}

void DiskBase::write( QByteArray data ) {
    const unsigned char* d = data.uc_str();
    printf( "**** Write %d bytes into file\n", data.length() );
    fwrite( d, data.length(), 1, file );
}

void DiskBase::closeFile() {
    if ( file != NULL ) {
        fclose( file );
        file = NULL;
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

string DiskBase::getOpenedFilename() { return openedFilename; }
