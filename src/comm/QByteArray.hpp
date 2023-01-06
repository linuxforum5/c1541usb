#ifndef QBYTEARRAY_HPP
#define QBYTEARRAY_HPP

#include <iostream>

using namespace std;

class QByteArray {

    public:
        QByteArray();
        QByteArray( char* bytes, int length );
        QByteArray( const string str );
        unsigned char at( int index );
        QByteArray mid( int from, int length );
        void prepend( char c );
        QByteArray append( char c );
        string to_string();
        const char* c_str();
        bool eq( const string str );
        bool eq( const char c );
    private:
        static const int max_size = 65536;
        char buf[ max_size ];
        int buf_length;
};

#endif // QBYTEARRAY_HPP
