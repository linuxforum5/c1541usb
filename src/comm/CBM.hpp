#ifndef CBM_HPP
#define CBM_HPP

class CBM {
public:
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
    const char DOLLAR_SIGN = '$';

    typedef enum {
        ErrOK = 0,
        ErrFilesScratched,	        // Files scratched response, not an error condition.^M
        ErrBlockHeaderNotFound = 20,
        ErrSyncCharNotFound,
        ErrDataBlockNotFound,
        ErrChecksumInData,
        ErrByteDecoding,
        ErrWriteVerify,
        ErrWriteProtectOn,
        ErrChecksumInHeader,
        ErrDataExtendsNextBlock,
        ErrDiskIdMismatch,
        ErrSyntaxError,
        ErrInvalidCommand,
        ErrLongLine,
        ErrInvalidFilename,
        ErrNoFileGiven,                 // The file name was left out of a command or the DOS does not recognize it as such.
                                        // Typically, a colon or equal character has been left out of the command
        ErrCommandNotFound = 39,        // This error may result if the command sent to command channel (secondary address 15) is unrecognizedby the DOS.
        ErrRecordNotPresent = 50,
        ErrOverflowInRecord,
        ErrFileTooLarge,
        ErrFileOpenForWrite = 60,
        ErrFileNotOpen,
        ErrFileNotFound,
        ErrFileExists,
        ErrFileTypeMismatch,
        ErrNoBlock,
        ErrIllegalTrackOrSector,
        ErrIllegalSystemTrackOrSector,
        ErrNoChannelAvailable = 70,
        ErrDirectoryError,
        ErrDiskFullOrDirectoryFull,
        ErrIntro,                       // power up message or write attempt with DOS mismatch
        ErrDriveNotReady,               // typically in this emulation could also mean: not supported on this file system.
        ErrSerialComm = 97,             // something went sideways with serial communication to the file server.
        ErrNotImplemented = 98,         // The command or specific operation is not yet implemented in this device.
        ErrUnknownError = 99,
        ErrCount
    } IOErrorMessage;

};

#endif // CBM_HPP
