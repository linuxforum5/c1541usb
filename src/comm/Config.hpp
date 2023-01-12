#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>

using namespace std;

class Config {
public:
    Config();
    string media;                    // ./
    string serialDevice;             // /dev/ttyUSB0
    unsigned char cbmDeviceNumber;   // 8
    unsigned char pin_ATN;           // 5
    unsigned char pin_CLOCK;         // 4
    unsigned char pin_DATA;          // 3
    unsigned char pin_RESET;         // 7
    unsigned char pin_SRQ_IN;        // 2
    string getOkResponseString();    // "OK>8|5|4|3|7|2|2022-12-28.17:00:29\r"
    const string getNoOkResponseString = "NOK>\r";
    bool debug;                      // false;
    void loadFromFile( const char *filename ); 

};

#endif // CONFIG_HPP
