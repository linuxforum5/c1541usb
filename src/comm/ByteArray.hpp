#ifndef QBYTEARRAY_HPP
#define QBYTEARRAY_HPP

#include <iostream>

using namespace std;

class ByteArray {

    public:
        ByteArray();
        ByteArray( unsigned char* bytes, int length );
        ByteArray( const string str );
        ByteArray( int size, unsigned char c );
        unsigned char at( int index );
        ByteArray mid( int from, int length = 0 );
        void prepend( unsigned char c );
        ByteArray append( unsigned char c );
        ByteArray append( const char* str );
        ByteArray append( string str );
        string to_string();
        const char* c_str();
        const unsigned char* uc_str();
        bool eq( const string str );
        bool eq( const unsigned char c );
        bool isEmpty();
        char strcmpLeft( const char* data ); // -1,0,1 strcmp like
        int length();
    private:
        static const int max_size = 65536;
        unsigned char buf[ max_size ];
        int buf_length;
};

#endif // QBYTEARRAY_HPP
