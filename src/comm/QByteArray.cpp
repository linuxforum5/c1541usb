#include "QByteArray.hpp"
#include<iostream>

QByteArray::QByteArray( ) {
    buf_length = 0;
}

QByteArray::QByteArray( char* bytes, int length ) {
    buf_length = length;
    for( int i=0; i<length; i++ ) buf[i] = bytes[i];
}

QByteArray::QByteArray( const string str ) {
    buf_length = str.length();
    for( int i=0; i<buf_length; i++ ) buf[i] = str.at(i);
}

unsigned char QByteArray::at( int index ) { return buf[ index ]; }

void QByteArray::prepend( char c ) {
    for( int i = buf_length; i > 0; i-- )
        buf[ i ] = buf[ i-1 ];
    buf[ 0 ] = c;
    buf_length++;
}

QByteArray QByteArray::append( char c ) {
    buf[ buf_length ] = c;
    buf_length++;
    return *this;
}

bool QByteArray::eq( const string str ) {
    bool equ = false;
    if ( str.length() == buf_length ) {
        equ = true;
        for( int i=0; i<buf_length && equ; i++ )
            equ = buf[i] == str.at(i);
    }
    return equ;
}

bool QByteArray::eq( const char c ) {
    return buf_length==1 && buf[ 0 ] == c;
}

string QByteArray::to_string() {
    buf[ buf_length ] = 0;
    string str = "";
    for( int i=0; i<buf_length; i++ )
        str.append( 1, buf[ i ] );
// printf( "STR (%d):", buf_length );
// for(int i=0; i<buf_length; i++)
//     printf( " %02X", buf[i] );
// printf( "\n" );
    return str;
}

const char* QByteArray::c_str() {
    buf[ buf_length ] = 0;
    return buf;
}

QByteArray QByteArray::mid( int from, int length ) {
    char sub[ length ];
    for( int i=0; i<length; i++ )
        sub[i]=buf[from+i];
    return QByteArray( sub, length );
}
