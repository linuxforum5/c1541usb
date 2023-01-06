#include "Prg.hpp"

Prg::Prg( string filename ) : File() {
    size = 0;
    if ( filename.length() ) { // Van megadva fájl
        if ( file = fopen( filename.c_str(), "rb" ) ) { // Sikerült megnyitni a fájlt, tehát létezett
            name = filename;
//            type3 = "PRG";
            fseek( file, 0L, SEEK_END );
            size = ftell( file );
            fseek( file, 0L, SEEK_SET );
        } else {
            name = "";
        }
    } else {
        file = NULL;
        name = filename;
    }
}

unsigned short Prg::getSizeB() { return size; }

string Prg::getFilename() { return name; }

char Prg::getc() {
    if ( isEOF() ) {
        return 0;
    } else {
        return fgetc( file );
    }
}

bool Prg::isEOF() { return ftell( file ) >= size; }

bool Prg::exists() { return file; }

int Prg::getPosPercent() { return 100 * ftell( file ) / size; }
