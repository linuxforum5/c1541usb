#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>

#include <cstdlib>
#include <signal.h>

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include <vector>

#include "comm/QByteArray.hpp"
#include "comm/Buffer.hpp"
#include "comm/Interface.hpp"
#include "comm/Serial.hpp"

using namespace std;

bool debug = false;
// const unsigned char* response = "OK>8|5|4|3|7|2|2022-12-28.17:00:29\r";
bool handshake_begin = false; // Ha megkezdtük a kapcsolódást
bool connection_established = false; // Ha sikerült is

void processLine( Serial serial, char* data, int length, Interface &m_iface ) {
    if ( length ) {
        if ( connection_established ) {
            if ( debug ) {
                printf( "---> Request ( length=%d )", length );
                for( int i=0; i<length; i++ ) printf( " %02X", data[i] );
                printf( "\n" );
            }
            switch( data[0] ) {
                default :
                    printf( "Invalid line command after connection: '%c'!\n", data[0] );
                    exit(1);
            }
            // m.processData( data, length );
        } else if ( handshake_begin ) { // Itt várjuk a válasz
            if ( debug ) printf( "---> Request ( length=%d ) %s\n", length, data );
            if ( length == 43 ) { // Last init response
                if ( !strcmp( data, "DIMArduino time set to: 2022-12-28.17:00:29") ) {
                    printf( "\tConnection established\n" );
                    connection_established = true;
                }
            }
        } else if ( length == 17 ) { // without '\r'
            if ( debug ) printf( "---> Request ( length=%d ) %s\n", length, data );
            if ( !strcmp( data, "connect_arduino:2" ) ) {
                printf( "\tConnection string found!\n" );
                const string response = "OK>8|5|4|3|7|2|2022-12-28.17:00:29\r";
                serial.Send( response );
                handshake_begin = true;
            }
        }
    }
}

void manage_cmds( Serial serial, Buffer &buffer, Interface &m_iface ) {
    string message;
    if ( buffer.length() > 0 ) {
        char cmd1 = buffer.getFirstChar();
        unsigned char data_length;
        unsigned char channel;
        switch( cmd1 ) {
            case 'O' : // 4F 04 00 24
                data_length = buffer.getSecondChar();
                if ( data_length >= 4 ) {
                    if ( buffer.length() >= data_length ) { // Minden adat már a buffer-ban van
                        QByteArray command = buffer.getFirstChars( data_length );
                        channel = command.at( 2 );
                        QByteArray filename = command.mid( 3, data_length-3 );
                        m_iface.processOpenCommand( serial, channel, filename );
                    }
                }
                break;
            case 'L' : // 4C
                buffer.getFirstChars( 1 );
                m_iface.processLineRequest( serial );
                break;
            case 'D' : // 44 45 46
                message = buffer.getToCr();
                printf( "\n\tArduino Debug (%d): '%s'\n", message.length(), message.c_str() );
                break;
            case 'C' :
                buffer.getFirstChars( 1 );
                m_iface.processCloseCommand( serial );
                break;
            case 'S': // request for file size in bytes before sending file to CBM^M
                buffer.getFirstChars( 1 );
                m_iface.processGetOpenFileSize( serial );
                break;
            case 'R':  // LOAD ,8
                if ( buffer.length() > 0 ) {
                    buffer.getFirstChars( 1 );
                    int percent = m_iface.processReadFileRequest( serial );
                    printf( "\t%02d%%\r", percent );
                    fflush(stdout);
                }
                break;
            case 'N': // LOAD ,8,1 : same as 'N', but we are also given the expected read size. All succeeding 'R' will be with this size.^M
                if ( buffer.length() > 1 ) {
                    data_length = buffer.getSecondChar();
                    buffer.getFirstChars( 2 );
                    int percent = m_iface.processReadFileRequest( serial, data_length );
                    printf( "\t%02d%%\r", percent );
                    fflush(stdout);
                }
                break;
            default:
                buffer.show_content(); // For debug only
                printf( "Invalid command: %c. Buffer length=%d\n", cmd1, buffer.length() );
                break;
        }
    }
}

void processBufferData( Serial serial, Buffer &buffer, Interface &m_iface ) {
    if ( !connection_established && buffer.isCompleteFirstLine() ) {
        char* line = buffer.readFirstLine();
        if ( line != NULL ) { // Sikerült a bufferből az első sort kiolvasni
            if ( buffer.firstLineLength() > 0 )
                processLine( serial, line, buffer.firstLineLength(), m_iface );
            buffer.dropFirstLine();
        }
    } else if ( connection_established ) {
        manage_cmds( serial, buffer, m_iface );
    }
}

void signal_callback_handler( int signum ) {
    cout << "CTRL+C detected. Exit ... " << signum << endl;
    // serial.Close();
    // Terminate program
    exit( signum );
}

bool processing = false; // for timeout debug only

void startC1541( string prgname ) {
    bool connection_established = false;

    int rdlen = 0;
    char data[ 256 ];
    Buffer buffer = Buffer();

    const string devName = "/dev/ttyUSB0";
    Serial serial( devName );

    Interface m_iface;
    m_iface.setPrg( prgname );
    do {
        processBufferData( serial, buffer, m_iface );
        if ( rdlen > 0 ) { // Vár adat még a data tömbben, ami nem fért a buffer-be
            // printf( "Append %d new chars\n", rdlen );
            rdlen = buffer.appendData( data, rdlen ); // A data csak akkor kerül a buffer-be, ha a teljes data belefér. Ez esetben a visszatérő érték 0
        } else { // rdlen == 0 azaz nincs beolvasott adat, amit még nem tettünk a buffer-ba
            rdlen = serial.Read( data, sizeof( data ), 10000 );
            if ( rdlen > 0 ) { // SKIP
                processing = true;
            } else if ( rdlen < 0 ) {
                printf( "***\tError from read: %d: %s\n", rdlen, strerror(errno) );
                serial.Close();
                exit(1);
            } else {  /* rdlen == 0 */
                if ( processing ) {
                    if ( debug ) printf( "\tTimeout from read... (Ctrl+C to quit) Buffer length = %d\n", buffer.length() );
                    processing = false;
                }
            }
        }
    } while ( 1 );
    serial.Close();
}

void startC1541check( string prgname ) {
    if ( FILE* f = fopen( prgname.c_str(), "rb" ) ) {
        fclose( f );
        startC1541( prgname );
    } else {
        printf( "A file nem lézezik\n" );
        exit(1);
    }
}

void showHelp() {
    printf( "c1541\n" );
    printf( "Commodore 1541 emulator over USB connected Arduino.\n" );
    printf( "Usage:\n" );
    printf( "c1541 option\n" );
    printf( "Options:\n" );
    printf( "-p prgFilename Commodore program file stored in PRG format, with .PRG extension.\n" );
    exit(1);
}

int main( int argc, char *argv[] ) {
    if ( argc < 3 ) {
        showHelp();
    } else {
        signal(SIGINT, signal_callback_handler);
        int c;
        while ((c = getopt(argc, argv, "?h:p:d")) != -1) {
            switch (c) {
                case 'h':
                case '?':
                    showHelp();
                    break;
                case 'p':
                    startC1541check( optarg );
                    break;
                case 'd': 
                    break;
                default:
                    showHelp();
                    break;
            }
        }
        argc -= optind; 
        argv += optind; 
    }
    return 0;
}
