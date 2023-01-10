#ifndef SERIAL_HPP
#define SERIAL_HPP

#include <iostream>
#include <termios.h>

using namespace std;

class Serial {

    public:
//         Serial();
        Serial( const string devName, bool debub );

        void Send( const string data );
        int Read( char *data, int size, int timeout_usec );
        void Close();
        bool getDebug();
    private:
        int serial_fd;
        int Open( const char* serial_name, speed_t baud );
        bool debug;
};

#endif // SERIAL_HPP
