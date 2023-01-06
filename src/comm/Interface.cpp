#include "Interface.hpp"
#include<iostream>
#include<cmath>

Interface::Interface() {
    m_dirListing = {};
    m_openState = O_NOTHING; // O_DIR, O_FILE
    disk = DiskBase();
    deviceNumber = 8;
    m_currReadLength = MAX_BYTES_PER_REQUEST; // Maximum ennyi bájtot küldhetünk vissza egyszerre
}

bool Interface::setPrg( string prgFilename ) {
    return disk.setPrg( prgFilename );
}

void Interface::processOpenCommand( Serial serial, unsigned char channel, QByteArray &filename /*, bool localImageSelectionMode = false */ ) {
//    string line;
//    string fn = filename.to_string();
    printf( "Start open command. Channel: %d, fn: '%s'\n", channel, filename.c_str() );
    switch( channel ) {
        case READPRG_CHANNEL :
            if ( filename.eq( CBM_DOLLAR_SIGN ) ) {
                printf( "\tCommodore requested directory list...\n" );
                m_openState = O_DIR;
                createDirectoryListing();
            } else {
                printf( "\tCommodore requested file...\n" );
                if ( disk.openFile( filename.to_string() ) ) {
                    m_openState = O_FILE;
                } else { // File not found???
                    printf( "File open error!\n" );
                    exit(1);
                    m_openState = O_FILE_ERR;
                }
            }
            sendOpenResponse( serial, m_openState );
            break;
//        case WRITEPRG_CHANNEL : 
//        case CMD_CHANNEL : 
        default:
            printf( "Invalid channel: %d\n", channel );
            break;
    }
}

void Interface::processCloseCommand( Serial serial ) {
//    string name = m_currFileDriver->openedFileName();
    QByteArray data;
    if ( m_openState == O_SAVE or m_openState == O_SAVE_REPLACE or m_openState == O_FILE ) {
/*
        // Small 'n' means last operation was a save operation.
        data.append(m_openState == O_SAVE or m_openState == O_SAVE_REPLACE ? 'n' : 'N').append((char)name.length()).append(name);
        if ( 0 not_eq m_pListener ) // notify UI listener of change.
            m_pListener->fileClosed(name);
        Log(FAC_IFACE, info, QString("Close: Returning last opened file name: %1").arg(name));
        if ( not m_currFileDriver->close() ) {
            m_currFileDriver = &m_native;
            if ( 0 not_eq m_pListener )
                m_pListener->imageUnmounted();
	}
*/
    } else { // Means CLOSED and the drive number (that MAY have changed due to a comamnd).
        data.append('C').append( deviceNumber );
    }
    serial.Send( data.to_string() );
    m_openState = O_NOTHING;
}

void Interface::sendOpenResponse( Serial serial, char code ) const {
    // Response: ><code><CR>
    // send back response / result code to uno.
    string response = QByteArray().append('>').append(code).append('\r').to_string();
    printf( "\tresponse open command received\n" );
    serial.Send( response );
} // sendOpenResponse

void Interface::createDirectoryListing() {
    send_line( 0, disk.getLabelLine() );
    for( int i=0; i<disk.getFilesCounter(); i++ ) {
        File file = disk.getFile( i );
        string line = createFilenameLine( file.getFilename(), file.getType3() );
        unsigned short fileSize = file.getSizeKb();
        // Send initial spaces (offset) according to file size
        send_line( fileSize, line.substr((int)log10((double)fileSize))); // A méret mérete karakterekben
    }
} // sendListing

string Interface::createFilenameLine( string name, string type3 ) {
    string line = "   \"";
    line.append( name );
    line.append("\" ");
    int spaceFill = 16 - name.length();
    while ( spaceFill-- > 0 )
        line.append( " " );
    line.append( type3 ); // PrgType. PRG/DIR
    return line;
}

void Interface::send_line( short lineNo, const string& text ) {
// printf( "SENDLINE1 (%d): '%s'\n", text.length(), text.c_str() );
    QByteArray line( text/*.toLocal8Bit()*/ );
    // the line number is included with the line itself. It goes in with lobyte,hibyte.
    line.prepend( (unsigned char)((lineNo bitand 0xFF00) >> 8) );
    line.prepend( (unsigned char)(lineNo bitand 0xFF) );
    // length of it all is the first byte.
    line.prepend( (unsigned char)text.size() + 2 );
    // add the response byte.
    line.prepend( 'L' );
    // add it to the total dirlisting array.
// printf( "List item counter before = %d\n", m_dirListing.size() );
    m_dirListing.push_back( line.to_string() );
// printf( "List item counter after = %d\n", m_dirListing.size() );
} // send

void Interface::processLineRequest( Serial serial ) {
//    if( O_INFO == m_openState or O_DIR == m_openState ) {
    if ( m_dirListing.empty() ) {
        printf( "\tResponse content is empty.\n" );
        string response = "l";
        serial.Send( response );
    } else {
        printf( "\tSend respone line for file content\n" );
        string line = m_dirListing.front();
        serial.Send( line );
        m_dirListing.pop_front();
    }
}

void Interface::processGetOpenFileSize( Serial serial ) {
    unsigned short size = disk.openedFileSize();
    QByteArray data;
    char high = size >> 8, low = size bitand 0xff;
    serial.Send( data.append( 'S' ).append( high ).append( low).to_string() );
    printf( "\tGetOpenFileSize: Returning file size: %d (%02X %02X)\n", size, high, low );
} // processGetOpenedFileSize

/**
 * Ha megvan adva a length (R), akkor annál 2-vel kevesebb bájtot kell visszaküldenünk, mivel még 2 bájtot a protokol kér.
 * Ha a length nincs megadva (N), azaz 0, akkor a visszaküldenő 
 */
int Interface::processReadFileRequest( Serial serial, unsigned short length ) {
    QByteArray data;
    unsigned char count;
    bool atEOF = false;
    if ( length )
        m_currReadLength = length;
    // NOTE: -2 here because we need two bytes for the protocol.
    for ( count = 0; count < m_currReadLength - 2 and not atEOF; count++ ) {
        data.append( disk.getOpenedFile()->getc() );
        atEOF = disk.getOpenedFile()->isEOF();
    }
// progress meter:
//    if ( 0 not_eq m_pListener )
//        m_pListener->bytesRead(data.size());
    // prepend whatever count we got.
    data.prepend( count );
    // If we reached end of file, head byte in answer indicates with 'E' instead of 'B'.
    data.prepend( atEOF ? 'E' : 'B' );
    serial.Send( data.to_string() );
    return disk.openedFilePosPercent();
}

/*
void Interface::fileLoading( const string& fileName, unsigned short fileSize ) {
    m_loadSaveName = fileName;
    // ui->progressInfoText->clear();
    // ui->progressInfoText->setEnabled(true);
    // ui->loadProgress->setEnabled(true);
    // ui->loadProgress->setRange(0, fileSize);
    // ui->loadProgress->show();
    m_totalReadWritten = 0;
    // ui->loadProgress->setValue(m_totalReadWritten);
    // writeTextToDirList( QString("LOAD\"%1\",%2\n\nSEARCHING FOR %1\nLOADING").arg(fileName, QString::number(m_appSettings.deviceNumber)));
    // cbmCursorVisible(false);
} // fileLoading
*/
