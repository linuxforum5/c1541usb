#include "Config.hpp"
#include <iostream>
#include <ctime>

Config::Config() {
    media = ".";
    serialDevice = "/dev/ttyUSB0";
    cbmDeviceNumber = 8;
    pin_ATN = 5;
    pin_CLOCK = 4;
    pin_DATA = 3;
    pin_RESET = 7;
    pin_SRQ_IN = 2;
    debug = false;
    getNoOkResponseString = "NOK>\r";
}

void Config::set_media( string filename_with_path ) {
    while( filename_with_path.length() && filename_with_path.at( filename_with_path.length()-1 ) == '/' )
        filename_with_path.pop_back();
    media = filename_with_path;
}

string Config::get_media() { return media; }

string Config::getOkResponseString() {
    time_t curr_time;
    tm * curr_tm;
    time(&curr_time);
    curr_tm = localtime(&curr_time);
    char dtm[100];
    strftime( dtm, 50, "%Y-%m-%d.%H:%M:%S", curr_tm );

    char response[100] = "";
    sprintf( response, "OK>%d|%d|%d|%d|%d|%d|%s\r", cbmDeviceNumber, pin_ATN, pin_CLOCK, pin_DATA, pin_RESET, pin_SRQ_IN, dtm );
    return response;
}

void Config::loadFromFile( const char *filename ) {}
