#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include "QByteArray.hpp"
#include <iostream>
#include <list>
#include "Serial.hpp"
#include "../disks/NativeFs.hpp"

using namespace std;

class Interface {
public:
    Interface( string media, unsigned char cbmDevicenumber );
//    bool setPrg( string prgFilename );
    void processOpenCommand( Serial serial, unsigned char channel, QByteArray &cmd /*, bool localImageSelectionMode = false */ );
    void processLineRequest( Serial serial );
    void processCloseCommand( Serial serial );
    void processGetOpenFileSize( Serial serial );
    void processWriteFileRequest( const QByteArray& theBytes );
    int processReadFileRequest( Serial serial, unsigned short length = 0 );
    void processErrorStringRequest( Serial serial, CBM::IOErrorMessage code );
private:
    // Device OPEN channels.
    // Special channels.
    enum IECChannels {
        READPRG_CHANNEL = 0,
        WRITEPRG_CHANNEL = 1,
        CMD_CHANNEL = 15
    };
    enum OpenState {
        O_NOTHING,	// 0	Nothing to send / File not found error
        O_INFO,		// 1	User issued a reload sd card
        O_FILE,		// 2	A program file is opened for reading
        O_DIR,		// 3	A listing is requested
        O_FILE_ERR,	// 4	Incorrect file format opened
        O_SAVE,		// 5	A program file is opened for writing
        O_SAVE_REPLACE,	// 6	"---", but Save-with-replace is requested
        O_CMD		// 7	Command channel was opened
    };
    const char CBM_DOLLAR_SIGN = '$';

    unsigned char deviceNumber; // 8, 9, ...
    string getLabelLine( string diskLabel );
    string errorStringFromCode( CBM::IOErrorMessage code ) const;
    void createDirectoryListing();
    void send_line( short lineNo, const string& text );
    void sendOpenResponse( Serial serial, char code ) const;
    string createFilenameLine( string name, string type3 );
    list<string> m_dirListing;
    OpenState m_openState;
    NativeFs disk;
    unsigned short m_currReadLength;
    const unsigned short MAX_BYTES_PER_REQUEST = 256;
};

#endif // INTERFACE_HPP
