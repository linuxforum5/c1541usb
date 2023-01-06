#ifndef FILE_HPP
#define FILE_HPP

#include <iostream>

using namespace std;

class File {
public:
    File();
    File( string name );
    File( string name, unsigned short size );
    virtual string getFilename();
    unsigned short getSizeKb();
    virtual unsigned short getSizeB();
    virtual char getc(); // get next byte from file
    bool isEOF();
    bool exists();
    string getType3();
private:
    string name;
    unsigned short sizeB;
    string type3; // PRG | DIR | SEQ
    unsigned short pos;
    const char* data;
};

#endif // FILE_HPP
