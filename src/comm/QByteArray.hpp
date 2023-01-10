#ifndef QBYTEARRAY_HPP
#define QBYTEARRAY_HPP

#include <iostream>

using namespace std;

class QByteArray {

    public:
        QByteArray();
        QByteArray( unsigned char* bytes, int length );
        QByteArray( const string str );
        QByteArray( int size, unsigned char c );
        unsigned char at( int index );
        QByteArray mid( int from, int length = 0 );
        void prepend( unsigned char c );
        QByteArray append( unsigned char c );
        QByteArray append( const char* str );
        string to_string();
        const char* c_str();
        const unsigned char* uc_str();
        bool eq( const string str );
        bool eq( const unsigned char c );
        bool isEmpty();
        int length();
    private:
        static const int max_size = 65536;
        unsigned char buf[ max_size ];
        int buf_length;
};

#endif // QBYTEARRAY_HPP
