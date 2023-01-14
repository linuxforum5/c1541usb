#ifndef BUFFER_HPP
#define BUFFER_HPP

#include "ByteArray.hpp"

// class MainWindow;

class Buffer {

    public:
        Buffer();
        int length();
        bool isCompleteFirstLine();
        int appendData( const char* data, int length );
        char* readFirstLine();
        char getFirstChar();
        char getSecondChar();
        int firstLineLength();
        void dropFirstLine();
        void show_content(); // For debug only
        ByteArray getFirstChars( int n );
        string getToCr();
        int firstEOL; // A teljes első sor hossza EOL nélkül, ha van teljes első sor, különben -1, azaz az első EOL indexe
    private:
        void setFirstEOL();
        void shift( int n ); // Balra léptet n karaktert, a baloldaliak kihullanak.
        static const int max_size = 1000;
        static const char EOL = '\r';

        char buf[ max_size ];
        int top; // Index of first free space
};

#endif // BUFFER_HPP
