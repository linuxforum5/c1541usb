#include "Serial.hpp"

#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <cstdlib>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>

Serial::Serial( const string devName ) {
    debug = false;
    serial_fd = Serial::Open( devName.c_str(), B115200 );
    if ( serial_fd == -1 ) {
        printf( "Error opening the serial device: %s\n", devName.c_str() );
        perror( "OPEN" );
        exit( 1 );
    }
    char data[500];
    while ( Read( data, sizeof( data ), 10000 ) > 0 ); // Clear tty buffer
    printf( "SERIAL OPEN:%s (%d)\n", devName.c_str(), serial_fd );
}

int Serial::Open( const char* serial_name, speed_t baud ) {
    struct termios newtermios;
    int fd;
    fd = open( serial_name, O_RDWR | O_NOCTTY );

//    cfsetospeed(&tty, (speed_t)speed);
//    cfsetispeed(&tty, (speed_t)speed);
//
//    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
//    tty.c_cflag &= ~CSIZE;
//    tty.c_cflag |= CS8;         /* 8-bit characters */
//    tty.c_cflag &= ~PARENB;     /* no parity bit */
//    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
//    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */
//
//    /* setup for non-canonical mode */
//    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
//    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
//    tty.c_oflag

    newtermios.c_cflag = CBAUD | CS8 | CLOCAL | CREAD; // | ~CRTSCTS | ~CSTOPB | ~PARENB | ~CSIZE;
    newtermios.c_iflag = IGNPAR;
    newtermios.c_oflag = 0;
    newtermios.c_lflag = 0;
    newtermios.c_cc[ VMIN ] = 1;
    newtermios.c_cc[ VTIME ] = 0;
    cfsetospeed( &newtermios, baud );
    cfsetispeed( &newtermios, baud );
    if ( tcflush( fd, TCIFLUSH ) == -1 ) return -1;
    if ( tcflush( fd, TCOFLUSH ) == -1 ) return -1;
    if ( tcsetattr( fd,TCSANOW, &newtermios ) == -1 ) return -1;
    return fd;
}

void Serial::Send( const string data ) {
    write( serial_fd, data.c_str(), data.length() );
    if ( debug ) {
        printf( "<--- Response sent (%d):", data.length() );
        for( int i=0; i<data.length(); i++ )
            printf( " %02X", data.at(i) );
        printf( "\n" );
    }
}

int Serial::Read( char *data, int size, int timeout_usec ) {
    fd_set fds;
    struct timeval timeout;
    int count=0;
    int ret;
    int n;
    do {
        FD_ZERO( &fds );
        FD_SET ( serial_fd, &fds );
        timeout.tv_sec = 0;
        timeout.tv_usec = timeout_usec;
        ret = select ( FD_SETSIZE, &fds, NULL, NULL, &timeout );
        if ( ret == 1 ) {
            n = read( serial_fd, &data[count], size-count );
            count += n;
            data[count] = 0;
        }
    } while ( count<size && ret==1 );
    return count;
}

void Serial::Close() {
    close( serial_fd );
}
