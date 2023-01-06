#ifndef PRG_HPP
#define PRG_HPP

#include <iostream>
#include "File.hpp"

using namespace std;

class Prg : public File {
public:
    Prg( string name = "" );
    string getFilename();
    unsigned short getSizeKb();
    unsigned short getSizeB();
    char getc(); // get next byte from file
    bool isEOF();
    bool exists();
    int getPosPercent();
private:
    string name;
//    string type3; // PRG | DIR | SEQ
    int size;
    FILE* file;
};

#endif // PRG_HPP
