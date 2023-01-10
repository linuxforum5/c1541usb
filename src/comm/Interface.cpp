#include "Interface.hpp"
#include<iostream>
#include<cmath>

const list<string> s_IOErrorMessages = { "00,OK"
                                       , "01,FILES SCRATCHED"
                                       , "20,READ ERROR"         // Block header not found
                                       , "21,READ ERROR"         // no sync character
                                       , "22,READ ERROR"         // data block not present
                                       , "23,READ ERROR"         // checksum error in data block
                                       , "24,READ ERROR"         // byte decoding error
                                       , "25,WRITE ERROR"        // write-verify error
                                       , "26,WRITE PROTECT ON"
                                       , "27,READ ERROR"         // checksum error in header
                                       , "28,WRITE ERROR"        // long data block
                                       , "29,DISK ID MISMATCH"
                                       , "30,SYNTAX ERROR"       // general syntax Forexample, two file names may appear on the left side of the COPY command.
                                       , "31,SYNTAX ERROR"       // The DOS does not recognize the command. The command must start in the first position.
                                       , "32,SYNTAX ERROR"       // The command sent is longer than 58 characters.
                                       , "33,SYNTAX ERROR"       // Pattern matching is invalidly used in the OPEN or SAVE command.
                                       , "34,SYNTAX ERROR"       // The file name was left out of a command or the DOS does not recognize it as such.
                                       , "39,SYNTAX ERROR"       // This error may result if the command sent to command channel (secondary address 15) is unrecognized by the DOS.
                                       , "50,RECORD NOT PRESENT"
                                       , "51,OVERFLOW IN RECORD"
                                       , "52,FILE TOO LARGE"
                                       , "60,WRITE FILE OPEN"
                                       , "61,FILE NOT OPEN"
                                       , "62,FILE NOT FOUND"
                                       , "63,FILE EXISTS"
                                       , "64,FILE TYPE MISMATCH"
                                       , "65,NO BLOCK"
                                       , "66,ILLEGAL TRACK AND SECTOR"
                                       , "67,ILLEGAL SYSTEM T OR S"
                                       , "70,NO CHANNEL"
                                       , "71,DIRECTORY ERROR"
                                       , "72,DISK FULL"
                                       , "73,UNO2IEC DOS V0.2"
                                       , "74,DRIVE NOT READY"
                                       , "97,UNO SERIAL ERR."    // Specific error to this emulated device, serial communication has gone out of sync.
                                       , "98,NOT IMPLEMENTED" };
const string s_unknownMessage = "99,UNKNOWN ERROR";
const string s_errorEnding = ",00,00";

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
/*
        case CBM::CMD_CHANNEL: // cmd = filename
            // command channel command, or request for status if empty.
            if ( cmd.isEmpty() or ( cmd.length() == 1 and cmd.at(0) == '\r' ) ) {
                // Response: ><code><CR>
                // The code return is according to the values of the IOErrorMessage enum.
                // send back m_queuedError to uno.
                sendOpenResponse( (char)m_queuedError );
//                Log( FAC_IFACE, info, QString("CmdChannel Status Response code: %1 = '%2'").arg( string::number( m_queuedError ) ).arg( errorStringFromCode( m_queuedError ) ) );
                // go back to OK state, we have dispatched the error to IEC host now. Error will only show once.
                m_queuedError = CBM::ErrOK;
            } else {
                // it's a DOS command, so execute it.
                m_queuedError = CBMDos::Command::execute( cmd, *this );
//                Log(FAC_IFACE, m_queuedError == CBM::ErrOK ? success : error, QString("CmdChannel_Response code: %1 = '%2'").arg(QString::number(m_queuedError)).arg(errorStringFromCode(m_queuedError)));
            }
            // Note: This MAY be actually an OPEN file instead so that close should return file name in this case too.
            m_openState = O_CMD;
            break;
*/
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
        case WRITEPRG_CHANNEL : // it was an open file for writing (save) command.
            {
                CBM::IOErrorMessage m_queuedError = CBM::ErrOK;
                m_openState = O_NOTHING;
 //            if ( disk != NULL ) {
                bool overwrite = ( filename.at( 0 ) == '@' );
                if ( overwrite ) filename = filename.mid( 1 ); // Drop '@'
                if ( filename.isEmpty() )
                    m_queuedError = CBM::ErrNoFileGiven;
                else if ( disk.isDiskWriteProtected() )
                    m_queuedError = CBM::ErrWriteProtectOn;
                else {
                    m_queuedError = disk.fopenWrite( filename, overwrite );
                    if ( CBM::ErrOK == m_queuedError ) {
                        // if ( 0 not_eq m_pListener ) m_pListener->fileSaving(fileName);
                        m_openState = overwrite ? O_SAVE_REPLACE : O_SAVE;
                    }
                }
 //            } else
 //                m_queuedError = ErrDriveNotReady;
                if ( CBM::ErrOK != m_queuedError ) m_openState = O_FILE_ERR;
                // The code return is according to the values of the IOErrorMessage enum.
                sendOpenResponse( serial, (char)m_queuedError );
                if ( serial.getDebug() ) {
                    printf( "Create file %s\n", ( m_queuedError == CBM::ErrOK ) ? "OK" : "Error" );
                }
            }
            break;
//        case CMD_CHANNEL : 
        default:
            printf( "Invalid channel for open: %d\n", channel );
            break;
    }
}

void Interface::processWriteFileRequest( const QByteArray& theBytes ) {
    disk.write( theBytes );
} // processWriteFileRequest

void Interface::processCloseCommand( Serial serial ) {
    QByteArray data;
    if ( m_openState == O_SAVE or m_openState == O_SAVE_REPLACE or m_openState == O_FILE ) {
        string name = disk.getOpenedFilename();
        // Small 'n' means last operation was a save operation.
        data.append( m_openState == O_SAVE or m_openState == O_SAVE_REPLACE ? 'n' : 'N' ).append( (char)name.length() ).append( name.c_str() );
        disk.closeFile();
    } else { // Means CLOSED and the drive number (that MAY have changed due to a comamnd).
        data.append('C').append( deviceNumber );
    }
    serial.Send( data.to_string() );
    m_openState = O_NOTHING;
}

// For a specific error code, we are supposed to return the corresponding error string.
void Interface::processErrorStringRequest( Serial serial, CBM::IOErrorMessage code ) {
    // the return message begins with ':' for sync.
    QByteArray retStr( 1, ':' );
    // append message and the common ending and terminate with CR.
    string str = errorStringFromCode( code ) + s_errorEnding + '\r';
    serial.Send( retStr.append( str.c_str() ).to_string() );
} // processErrorStringRequest

string Interface::errorStringFromCode( CBM::IOErrorMessage code ) const {
    // Assume not found by pre-assigning the unknown message.
    for ( list<string>::const_iterator msg = s_IOErrorMessages.begin(); msg != s_IOErrorMessages.end(); msg++ ) {
//    foreach( const string& msg, s_IOErrorMessages )
        if ( code == stoi( msg->substr( 0, msg->find( ',' ) ) ) ) // found it!
            return *msg;
    }
    return s_unknownMessage;
} // errorStringFromCode

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
    unsigned char high = size >> 8, low = size bitand 0xff;
    serial.Send( data.append( 'S' ).append( high ).append( low).to_string() );
    printf( "<--- Send file size: %u = %04X (H:%02X L:%02X)\n", size, size, high, low );
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
