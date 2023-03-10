#include "ByteArray.hpp"
#include<iostream>

ByteArray::ByteArray( ) {
    buf_length = 0;
}

ByteArray::ByteArray( unsigned char* bytes, int length ) {
    buf_length = length;
    for( int i=0; i<length; i++ ) buf[i] = bytes[i];
}

ByteArray::ByteArray( const string str ) {
    buf_length = str.length();
    for( int i=0; i<buf_length; i++ ) buf[i] = str.at(i);
}

ByteArray::ByteArray( int size, unsigned char c ) {
    for( int i=0; i<size; i++ ) buf[i] = c;
    buf_length = size;
}

unsigned char ByteArray::at( int index ) { return buf[ index ]; }

void ByteArray::prepend( unsigned char c ) {
    for( int i = buf_length; i > 0; i-- )
        buf[ i ] = buf[ i-1 ];
    buf[ 0 ] = c;
    buf_length++;
}

ByteArray ByteArray::append( unsigned char c ) {
    buf[ buf_length++ ] = c;
    return *this;
}

ByteArray ByteArray::append( string str ) {
    for( int i=0; i<str.length(); i++ ) {
        buf[ buf_length++ ] = str.at( i );
    }
    return *this;
}

ByteArray ByteArray::append( const char* str ) {
    for( int i=0; str[ i ]; i++ ) {
        buf[ buf_length++ ] = str[ i ];
    }
    return *this;
}

char ByteArray::strcmpLeft( const char* data ) {
    char cmp = 0;
    int i = 0;
    while( i < buf_length && buf[ i ] == data[ i ] ) i++;
    if ( i == buf_length ) return 0;
    else if ( buf[ i ] < data[ i ] ) return -1;
    else return 1;
}

bool ByteArray::eq( const string str ) {
    bool equ = false;
    if ( str.length() == buf_length ) {
        equ = true;
        for( int i=0; i<buf_length && equ; i++ )
            equ = buf[i] == str.at(i);
    }
    return equ;
}

bool ByteArray::eq( const unsigned char c ) {
    return buf_length==1 && buf[ 0 ] == c;
}

bool ByteArray::isEmpty() { return buf_length==0; }

int ByteArray::length() { return buf_length; }

string ByteArray::to_string() {
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

const char* ByteArray::c_str() {
    buf[ buf_length ] = 0;
    return (char*)buf;
}

const unsigned char* ByteArray::uc_str() {
    buf[ buf_length ] = 0;
    return buf;
}

ByteArray ByteArray::mid( int from, int length ) {
    if ( from < 0 ) from = buf_length + from;
    if ( length < 0 ) length = buf_length + length - from;
    else if ( length == 0 ) length = buf_length - from;
    unsigned char sub[ length ];
    for( int i=0; i<length; i++ )
        sub[i]=buf[from+i];
    return ByteArray( sub, length );
}
