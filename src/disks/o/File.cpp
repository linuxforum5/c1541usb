#include "File.hpp"

File::File() {
    name = "";
    sizeB = 15;
    type3 = "";
    pos = 0;
    data = "\x01\x08\x0C\x08\x0A\x00\x9E\x20\x32\x30\x36\x34\x00\x00\x00";
}

File::File( string nm ) {
    name = nm;
    sizeB = 15;
    type3 = "PRG";
    pos = 0;
    data = "\x01\x08\x0C\x08\x0A\x00\x9E\x20\x32\x30\x36\x34\x00\x00\x00";
}

File::File( string nm, unsigned short sz ) {
    name = nm;
    sizeB = sz;
    type3 = "PRG";
    pos = 0;
    data = "\x01\x08\x0C\x08\x0A\x00\x9E\x20\x32\x30\x36\x34\x00\x00\x00";
}

unsigned short File::getSizeKb() { return sizeB/1024; }

unsigned short File::getSizeB() { return sizeB; }

bool File::exists() { return true; }

string File::getFilename() { return name; }
string File::getType3() { return type3; }

char File::getc() {
printf( "----------- GET FILE CHAR -------------\n" );
    if ( isEOF() ) {
        return 0;
    } else {
        return data[pos++];
    }
}

bool File::isEOF() { return pos >= sizeB; }
