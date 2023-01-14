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
    if ( firstEOL >= top ) {
        printf( "FirstEOL error! Top=%d, firstEOL=%d\n", top, firstEOL );
        exit(1);
    }
}

int Buffer::appendData( const char* data, int length ) {
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

ByteArray Buffer::getFirstChars( int n ) { // Read and drop firs chars
    if ( top >= n ) {
        unsigned char ret[ n ];
        for( int i=0; i<n; i++ )
            ret[i]=buf[i];
        shift( n );
        return ByteArray( ret, n );
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
    if ( n > 0 ) {
        if ( n < top ) {
            int i = n;
            int j = 0;
            while( i < top ) // Van mozgatandó adat
                buf[ j++ ] = buf[ i++ ];
            top -= n;
            if ( firstEOL >= n )
                firstEOL -= n;
            else
                firstEOL = -1;
        } else {
            top = 0;
            firstEOL = -1;
        }
    }
}

string Buffer::getToCr() {
    int i = 0;
    while( i<top && buf[i] != EOL ) i++;
    if ( i < top ) { // Valid line
        ByteArray str(getFirstChars( i ));
        getFirstChars( 1 );
        return str.to_string();
    } else {
        return NULL;
    }
}
