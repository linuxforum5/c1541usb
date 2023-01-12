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
#include "comm/Config.hpp"

using namespace std;

Config config;

// const unsigned char* response = "OK>8|5|4|3|7|2|2022-12-28.17:00:29\r";
bool handshake_begin = false; // Ha megkezdtük a kapcsolódást
bool connection_established = false; // Ha sikerült is

void processLine( Serial serial, char* data, int length, Interface &m_iface ) {
    if ( length ) {
        if ( connection_established ) {
            if ( config.debug ) {
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
            if ( config.debug ) printf( "---> Request ( length=%d ) %s\n", length, data );
            if ( length == 43 ) { // Last init response
                if ( !strcmp( data, "DIMArduino time set to: 2022-12-28.17:00:29") ) {
                    printf( "\tConnection established\n" );
                    connection_established = true;
                }
            }
        } else if ( length == 17 ) { // without '\r'
            if ( config.debug ) printf( "---> Request ( length=%d ) %s\n", length, data );
            if ( !strcmp( data, "connect_arduino:2" ) ) {
                printf( "\tConnection string found!\n" );
                QByteArray response( "OK>8|5|4|3|7|2|2022-12-28.17:00:29\r" );
                serial.Send( response );
                handshake_begin = true;
            }
        }
    }
}

/* ????????????????????
void processAddNewFacility( const string& str ) {
    //<---->Log("MAIN", QString("Got facility: %1").arg(str.mid(2)), success);
    m_clientFacilities[ str.at(1) ] = str.mid(2);
} // processAddNewFacility
*/

void manage_cmds( Serial serial, Buffer &buffer, Interface &m_iface ) {
    string message;
    if ( buffer.length() > 0 ) {
        char cmd1 = buffer.getFirstChar();
        unsigned char data_length;
        unsigned char channel;
        if ( config.debug ) buffer.show_content(); // For debug only
        switch( cmd1 ) {
/*
            case '!': // register facility string.
                if ( buffer.isCompleteFirstLine() ) {
                    message = buffer.getToCr();
                    processAddNewFacility( message );
                }
                break;
*/
            case 'D' : // 44 45 46
                if ( buffer.isCompleteFirstLine() ) {
                    if ( serial.getDebug() ) printf( "---> D to CR. Buffer size: %d firstEOL=%d.\n", buffer.length(), buffer.firstEOL );
// buffer.show_content();
                    message = buffer.getToCr();
                    printf( "\n\tArduino Debug (%d): '%s'\n", message.length(), message.c_str() );
                }
                break;
            case 'S': // request for file size in bytes before sending file to CBM^M
                buffer.getFirstChars( 1 );
                if ( serial.getDebug() ) printf( "---> S (1 byte)\n" );
                m_iface.processGetOpenFileSize( serial );
                break;
            case 'O' : // 4F 04 00 24
                data_length = buffer.getSecondChar();
                if ( data_length >= 4 ) {
                    if ( serial.getDebug() ) printf( "---> O (4 byte)\n" );
                    if ( buffer.length() >= data_length ) { // Minden adat már a buffer-ban van
                        QByteArray command = buffer.getFirstChars( data_length );
                        channel = command.at( 2 );
                        QByteArray filename = command.mid( 3, data_length-3 );
                        m_iface.processOpenCommand( serial, channel, filename );
                    }
                }
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
            case 'W': // write characters to file in current file system mode.^M
                if ( buffer.length() > 1 ) {
                    data_length = buffer.getSecondChar();
                    if ( serial.getDebug() ) printf( "---> W (%d bytes)\n", data_length );
                    if ( buffer.length() >= data_length ) {
                        if ( config.debug ) printf( "Command W length=%d\n", data_length );
                        buffer.getFirstChars( 2 );
                        m_iface.processWriteFileRequest( buffer.getFirstChars( data_length - 2 ) );
                        // discard all processed (written) bytes from buffer.
                    }
                }
                break;
            case 'L' : // 4C
                buffer.getFirstChars( 1 );
                if ( serial.getDebug() ) printf( "---> L (1 byte)\n" );
                m_iface.processLineRequest( serial );
                break;
            case 'C' :
                buffer.getFirstChars( 1 );
                if ( serial.getDebug() ) printf( "---> C (1 byte)\n" );
                m_iface.processCloseCommand( serial );
                break;
            case 'E': // Ask for translation of error string from error code
                if ( buffer.length() >= 2 ) {
                    if ( serial.getDebug() ) printf( "---> E (2 bytes)\n" );
                    m_iface.processErrorStringRequest( serial, static_cast<CBM::IOErrorMessage>( buffer.getSecondChar() ) );
                    buffer.getFirstChars( 2 );
                }
                break;
            default:
                buffer.show_content(); // For debug only
                printf( "Invalid command: '%c'. Buffer length=%d\n", cmd1, buffer.length() );
                buffer.show_content();
                exit(1);
                break;
        }
    }
}

void processBufferData( Serial serial, Buffer &buffer, Interface &m_iface ) {
    if ( !connection_established && buffer.isCompleteFirstLine() ) {
        if ( buffer.isCompleteFirstLine() ) {
            char* line = buffer.readFirstLine();
            if ( line != NULL ) { // Sikerült a bufferből az első sort kiolvasni
                if ( buffer.firstLineLength() > 0 )
                    processLine( serial, line, buffer.firstLineLength(), m_iface );
                buffer.dropFirstLine();
            }
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

void startC1541() {

    bool connection_established = false;

    int rdlen = 0;
    char data[ 256 ];
    Buffer buffer = Buffer();

    Serial serial( config.serialDevice, config.debug );

    Interface m_iface( config.media, config.cbmDeviceNumber );
//    m_iface.setPrg( prgname );
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
                    if ( config.debug ) printf( "\tTimeout from read... (Ctrl+C to quit) Buffer length = %d\n", buffer.length() );
                    processing = false;
                }
            }
        }
    } while ( 1 );
    serial.Close();
}

/*
void startC1541check( string prgname ) {
    if ( FILE* f = fopen( prgname.c_str(), "rb" ) ) {
        fclose( f );
        startC1541( prgname );
    } else {
        printf( "A file nem lézezik\n" );
        exit(1);
    }
}
*/

void showHelp() {
    printf( "c1541 Vesion 0.01\n" );
    printf( "Commodore 1541 emulator over USB connected Arduino.\n" );
    printf( "Usage:\n" );
    printf( "c1541 [ options ] [ mediaFile ]\n" );
    printf( "Possible media files:\n" );
    printf( "  - Directory. Each file in directory is a file on a disk.\n" );
    printf( "  - PRG file. The disk conatins only this one PRG file.\n" );
    printf( "  - D64 file. The D64 file as a disk content.\n" );
    printf( "If media file no define,9 the default value is the current directory.\n" );
    printf( "Options:\n" );
    printf( "  -d n        Commodore device number. Default is 8.\n" );
    printf( "  -c filename Config file.\n" );
    printf( "  -v          Verbose mode (debug).\n" );
    printf( "  -s device   Serial device name. Default is /dev/ttyUSB0\n" );
    printf( "  -h          This help screen.\n" );
    exit(1);
}

int main( int argc, char *argv[] ) {
/*
    QByteArray data;
    data = data.append( 'C' ).append( 8 );
    printf( "L=%d\n", data.length() );
exit(1);
*/
    int opt;
    // put ':' in the starting of the
    // string so that program can 
    //distinguish between '?' and ':' 
    signal(SIGINT, signal_callback_handler);
    while( ( opt = getopt( argc, argv, ":vh?d:c:s:" ) ) != -1 )  {
        switch( opt ) {
            case 'v':
                config.debug = true;
                break;
            case 'h': 
            case '?': 
                showHelp();
                break;
            case 'd': // deviceNumber
                config.cbmDeviceNumber = atoi( optarg );
                break;
            case 's': // serial device
                config.serialDevice = atoi( optarg );
                break;
            case 'c': // config file
                config.loadFromFile( optarg );
                break; 
            case ':': 
                printf( "option needs a value\n" ); 
                break; 
        } 
    }
    // optind is for the extra arguments
    // which are not parsed
    if ( argc - optind > 1 ) {
        printf( "Only one media enabled!\n" );
        showHelp();
    } else {
        for(; optind < argc; optind++){
            config.media = argv[ optind ]; // Egyelőre csak 1
        }
        startC1541();
    }
    return 0;
}
