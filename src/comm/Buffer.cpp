#include "Buffer.hpp"
#include<iostream>

Buffer::Buffer() {
    firstEOL = -1;
    top = 0;
}

void Buffer::show_content() {
    printf( "\tBUFFER (%d):", top );
    for( int i=0; i<top; i++ ) {
        printf( " %02X", buf[i] );
    }
    printf( "\n" );
}

int Buffer::appendData( char* data, int length ) {
    int data_index = 0;
    if ( top + length < max_size ) {
        while( data_index < length ) {
            buf[ top++ ] = data[ data_index++ ];
        }
        if ( firstEOL == -1 ) setFirstEOL();
    }
    return length - data_index;
}

int Buffer::length() { return top; }
int Buffer::firstLineLength() { return firstEOL; }
bool Buffer::isCompleteFirstLine() { return firstEOL > -1; }
char Buffer::getFirstChar() { return buf[ 0 ]; }
char Buffer::getSecondChar() { return buf[ 1 ]; }

QByteArray Buffer::getFirstChars( int n ) {
    if ( top >= n ) {
        unsigned char ret[ n ];
        for( int i=0; i<n; i++ )
            ret[i]=buf[i];
        shift( n );
        return QByteArray( ret, n );
    } else {
        printf( "Error: buffer data too small!\n" );
        exit(1);
    }
}

void Buffer::setFirstEOL() {
    for ( firstEOL = 0; firstEOL<top && buf[ firstEOL ] != EOL; firstEOL++ );
    if ( firstEOL == top ) firstEOL=-1;
}

char* Buffer::readFirstLine() {
    if ( isCompleteFirstLine() ) {
        buf[ firstEOL ] = 0;
        return buf;
    } else {
        return NULL;
    }
}

void Buffer::dropFirstLine() {
    if ( isCompleteFirstLine() ) {
        shift( firstEOL+1 );
        setFirstEOL();
    } else {
        printf( "Invalid drop!\n" );
        exit(1);
    }
}

void Buffer::shift( int n ) {
    int i = n;
    int j = 0;
    while( i < top ) // Van mozgatandó adat
        buf[ j++ ] = buf[ i++ ];
    top -= n;
}

string Buffer::getToCr() {
    int i = 0;
    while( i<top && buf[i] != EOL ) i++;
    if ( i < top ) { // Valid line
        QByteArray str(getFirstChars( i ));
        getFirstChars( 1 );
        return str.to_string();
    } else {
        return NULL;
    }
}
