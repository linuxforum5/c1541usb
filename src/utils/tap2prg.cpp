/**
 * https://vice-emu.sourceforge.io/vice_17.html
 */
#include<iostream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <getopt.h>

/* half-wave pulse timer values */
#define CBMPULSE_SHORT (0x30 - 1)
#define CBMPULSE_MEDIUM (0x42 - 1)
#define CBMPULSE_LONG (0x56 - 1)

uint32_t tapDataLen = 0;

struct tapHeader {
    char magic[12];
    uint8_t version;
    uint8_t platform;
    uint8_t video;
    uint8_t reserved;
    uint32_t dataLen;
};

char TAP_FILE_NAME16[16] = {'T', 'A', 'P', 'E', ' ', 'C', 'A', 'R', 'T', ' ', 'L', 'O', 'A', 'D', 'E', 'R'};
static const uint8_t default_loader[171] = {
    /* auto-generated from loader.bin on Fri Dec 31 10:33:12 2021 */
    0x6e, 0x11, 0xd0, 0x78, 0x18, 0xa0, 0x10, 0xa2, 0x00, 0x86, 0xc6, 0xa9, 0x27, 0x2e, 0xf7, 0x03, 0x90, 0x02, 0x09, 0x08, 0x85, 0x01, 0xca, 0xea, 0xd0, 0xfc, 0x29, 0xdf, 0x85,
    0x01, 0xca, 0xd0, 0xfd, 0x88, 0xd0, 0xe7, 0xa9, 0x30, 0x85, 0x01, 0xa0, 0xfa, 0x20, 0xb6, 0x03, 0x99, 0xb0, 0xff, 0xc8, 0xd0, 0xf7, 0x20, 0xb6, 0x03, 0x91, 0xae, 0xe6, 0xae,
    0xd0, 0x02, 0xe6, 0xaf, 0xa5, 0xae, 0xc5, 0xac, 0xd0, 0xef, 0xa6, 0xaf, 0xe4, 0xad, 0xd0, 0xe9, 0xa0, 0x37, 0x84, 0x01, 0x38, 0x2e, 0x11, 0xd0, 0x86, 0x2e, 0x86, 0x30, 0x86,
    0x32, 0x85, 0x2d, 0x85, 0x2f, 0x85, 0x31, 0x58, 0x20, 0x53, 0xe4, 0x6c, 0xaa, 0x00, 0xa9, 0x10, 0x24, 0x01, 0xd0, 0xfc, 0xa2, 0x38, 0xa9, 0x27, 0x86, 0x01, 0x85, 0x00, 0xea,
    0xa5, 0x01, 0x29, 0x18, 0x4a, 0x4a, 0x45, 0x01, 0x4a, 0x29, 0x0f, 0xaa, 0xa5, 0x01, 0x29, 0x18, 0x4a, 0x4a, 0x45, 0x01, 0x4a, 0x29, 0x0f, 0x1d, 0xe7, 0x03, 0xa2, 0x2f, 0x86,
    0x00, 0xe8, 0x86, 0x01, 0x60, 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 0xca, 0x00, 0x00, 0x00, 0x00

};

static uint8_t header_loader[171] = {0};
static uint8_t start_addr_low = 0x02, start_addr_high = 0x03;
static uint8_t end_addr_low = 0x04, end_addr_high = 0x03;

char tapfileName[256] = {0};
/*************************************************************************************************************
 *
 *************************************************************************************************************/
const int raw_tape_version = 1;
// https://plus4world.powweb.com/plus4encyclopedia/500247
enum PULSE_V1 {       //  V1   V2     TED KERNAL   318004-01
//    CLOCK1 = 0x00,
    SHORT1 = 0x35,    // 0x35 0x1A - 2083Hz         2500Hz       01 - 4F
    LONG1 = 0x6A,     // 0x6A 0x35 - 1042Hz         1250Hz       50 - 9E
    WORD1 = 0xD4      // 0xD4 0x6A -  521Hz          625Hz       9F - FF
};

enum PULSE_V2 {       //  V1   V2
    SHORT2 = 0x1A,    // 0x35 0x1A - 2083Hz
    LONG2 = 0x35,     // 0x6A 0x35 - 1042Hz
    WORD2 = 0x6A      // 0xD4 0x6A -  521Hz
};

void err( const char* msg ) {
    printf( "ERROR: %s\n", msg );
    exit( 1 );
}

unsigned int read_tap_pause_clock( FILE *tapFile ) {
    long pos = ftell( tapFile );
//    unsigned char first = fgetc( tapFile );
//    if ( first!=0 ) {
//        printf( "Invalid clock block: %02X at pos.:%d.\n", first, pos );exit(1);
//    }
    unsigned int clock;
    fread( &clock, 3, 1, tapFile ); //  = fgetc( tapFile ) + 256*fgetc( tapFile ) + 65536*fgetc( tapFile );
    printf( "\t!!! Pause %d (clock) at pos %04X\n", clock, pos );
    return clock;
}

int pulsecnt[255];
void initPulseStat() { for( int i=0; i<256; i++ ) pulsecnt[i]=0; }
void showPulseStat() {
    printf( "Pulsestat:\n" );
    for( int i=0; i<256; i++ ) {
        if ( pulsecnt[i] ) printf( "\t%02X %d\n", i, pulsecnt[i] );
    }
}

bool IS_IN( unsigned char value, unsigned char center ) {
    unsigned char d = 32;
    if ( center < d ) err( "Invalid pulse width limit value!" );
    if ( value >= center-d && value <= center+d ) return true;
    return false;
}

PULSE_V1 get_pulse_v1( FILE *tapFile ) {
    unsigned char c = fgetc( tapFile );
    if ( feof( tapFile ) ) return SHORT1;
    while ( c == 0 ) { // overflow
//        long pos = ftell( tapFile );
//        fseek( tapFile, pos-1, SEEK_SET );
        read_tap_pause_clock( tapFile );
        c = fgetc( tapFile );
        if ( feof( tapFile ) ) return SHORT1;
    }
    pulsecnt[c]++;

    if ( IS_IN( c, SHORT1 ) ) return SHORT1;
    else if ( IS_IN( c, LONG1 ) ) return LONG1;
    else if ( IS_IN( c, WORD1 ) ) return WORD1;
    printf( "Invalid pulse value: %02X\n", c );
    exit(1);
/*
    if ( c < SHORT1 ) return SHORT1;
    else if ( c < LONG1 ) return ( (LONG1-c) < (c-SHORT1) ) ? LONG1 : SHORT1;
    else if ( c < WORD1 ) return ( (WORD1-c) < (c-LONG1)  ) ? WORD1 :  LONG1;
    else return WORD1; // 193 251
*/
}
/*
void skipCountedRawTapeSyncron( FILE *tapFile, int count, PULSE_V1 after ) {
    for (int i=0; i<count; i++ ) {
        if ( get_pulse_v1( tapFile ) != SHORT1 ) {
            printf( "Invalid SHORT in block selector space at pos %04X\n", ftell( tapFile ) );exit(1);
        }
    }
    printf( "\tSkip after %d SHORT PULSE\n", count );
    if ( get_pulse_v1( tapFile ) != after ) {
        printf( "Invalid selector end after %d SHORT at %04X\n", count, ftell( tapFile ) );
    }
}
*/

void skipRawTapeSyncron( FILE *tapFile, PULSE_V1 end_pulse, bool go_back ) {
    long firstBytePos = ftell( tapFile );
    unsigned char lastByte = 0;
    int cnt = 0;
    while( !feof( tapFile ) && ( lastByte = get_pulse_v1( tapFile ) ) == SHORT1 ) {
        cnt++;
        firstBytePos = ftell( tapFile );
    }
    printf( "\tSkip prev %d SHORT PULSE\n", cnt );
    if ( !feof( tapFile ) ) {
        if ( lastByte != end_pulse ) {
            printf( "Invalid leader ending at pos %04X\n", firstBytePos );exit(1);
        }
        if ( go_back ) fseek( tapFile, firstBytePos, SEEK_SET ); // Go back after last SHORT
    }
}

unsigned char read_raw_bit( FILE *tapFile, unsigned char v, bool word_marker ) { // 0, 1, 2=EOF
    long pos = ftell( tapFile );
    if ( v == 1 ) {
        PULSE_V1 p1 = get_pulse_v1( tapFile ), p2 = get_pulse_v1( tapFile );
        if ( feof( tapFile ) ) return 2;
        if ( word_marker ) {
            if ( p1 == WORD1 && p2 == LONG1 ) return 0; // Word marker
            else if ( p1 == LONG1 && p2 == SHORT1 ) return 1; // New block start
            else {
                printf( "Invalid word marker at pos %04X!\n", pos ); exit(1);
            }
        } else if ( p1 == SHORT1 && p2 == LONG1 ) return 0;
        else if ( p1 == LONG1 && p2 == SHORT1 ) return 1;
        else {
            printf( "Invalid pulses p1=%02X, p2=%02X at pos %04X.\n", p1, p2, pos ); exit(1);
        }
    } else {
        err( "V2 not implemented!" );
    }
    return 0;
}

/**
 * data block
 * - leader SHORT SHORT ... SHORT WORD ([ 0x26 - 0x40 ] / [ 0x13 - 0x20 ] ($26/$13 to $40/$20)
 * - First data byte begins with WORD
 */
unsigned char read_raw_byte( FILE *tapFile ) {
    unsigned char byte = 0;
    unsigned char parity = 1;
    unsigned char value = 128; // Helyiérték
    for( int i=0; i<8; i++ ) {
        unsigned char bit = read_raw_bit( tapFile, raw_tape_version, false );
        if ( bit > 1 ) err( "Invalid bit!" );
        parity ^= bit;
        byte *= 2;
        byte += bit;
//        byte += bit * value;
//        value = value / 2;
    }
    if ( parity != read_raw_bit( tapFile, raw_tape_version, false ) ) err( "Tap parity error!" );
    return byte;
}

void checkTapHeaderString( FILE *tapFile, const char* str ) {
    while( (str[0]!=0) && ( fgetc( tapFile )==str[0] ) ) str++;
    if ( str[0] ) err( "Invalid TAP file header string" );
}

unsigned short readRawTapeHeader( FILE *tapFile ) {
return 0;
    unsigned char ctrl = fgetc( tapFile );
    if ( ctrl != 0x03 ) err( "Invalid control byte" );
    unsigned char start_addr_low = read_raw_byte( tapFile );
    unsigned char start_addr_high = read_raw_byte( tapFile );
    unsigned char end_addr_low = read_raw_byte( tapFile );
    unsigned char end_addr_high = read_raw_byte( tapFile );

    unsigned char checksum = 0x03 ^ start_addr_low ^ start_addr_high ^ end_addr_low ^ end_addr_high;

    unsigned short start_addr = start_addr_low + start_addr_high * 256;
    unsigned short end_addr = end_addr_low + end_addr_high * 256;

    // Skip raw tape filename
    for( int i=0; i<16; i++) checksum ^= read_raw_byte( tapFile ); // This name useable for prg filename?
    // Skip header loader
    for( int i=0; i<195-16-5; i++) checksum ^= read_raw_byte( tapFile );
    if ( read_raw_byte( tapFile ) != checksum ) err( "Invalid raw tape header checksum" );
    return start_addr;
}

unsigned short readTapHeader( FILE *tapFile ) {
    checkTapHeaderString( tapFile, "C16-TAPE-RAW" ); // $00-$0B: "xxx-TAPE-RAW", where xxx = C16 or C64
    unsigned char version = fgetc( tapFile );        // $0C: TAP format version (0, 1, 2)
    if ( version != 1 ) err( "Not implemented TAP version. Only version 1 is implemented yet." );
    unsigned char machine = fgetc( tapFile );        // $0D: Machine (0 = C64, 1 = VIC, 2 = C16)
    if ( machine != 2 ) err( "Not a C16 TAP format. Not implemented yet." );
    unsigned char video   = fgetc( tapFile );        // $0E: Video standard (0 = PAL, 1 = NTSC)
    unsigned char unused  = fgetc( tapFile );        // $0F: Unused (0)
    unsigned int file_size = 0;
    fread( &file_size, 4, 1, tapFile );              // $10-$13: File size, not including header, in low-to-high format.
    unsigned short start_addr = 0; // readRawTapeHeader( tapFile );
    // Tap file is on the first data byte position
    return start_addr;
}

int copyBytesFromTapToPrg( FILE *tapFile, FILE *prgFile ) {
    unsigned char first_pulse_code = read_raw_bit( tapFile, raw_tape_version, true ); // 0=word marker=start byte, 1=start new block with 193 SHORT PULSE
    int cnt=0;
    while( !feof(tapFile) && first_pulse_code == 0 ) { // First pulse is Word, read a byte
        unsigned char byte = read_raw_byte( tapFile );
        fwrite( &byte, 1, 1, prgFile );
        cnt++;
        first_pulse_code = read_raw_bit( tapFile, raw_tape_version, true );
    }
    if ( first_pulse_code != 1 ) { // Data block ending: LONG SHORT
        printf( "Invalid data block ending at pos %04X\n", ftell( tapFile ) );
//        printf( "Found data block ending at pos %04X\n", ftell( tapFile ) );
//        skipRawTapeSyncron( tapFile );
//        first_pulse_code = read_raw_bit( tapFile, raw_tape_version, true ); // 0=word marker=start byte, 1=start new block with 193 SHORT PULSE
//        if ( first_pulse_code ) err( "Invalid block end" );
    }
    return cnt;
}

/**
 * Egy blokk szerkezete 
 * - Countdown: 0x89 0x88 ... 0x81 (second esetén 0x09 0x08 ... 0x01)
 * - Type: 
 *    - 0x01: File header for relocatable program (normally used for BASIC programs)
 *    - 0x02: SEQ data
 *    - 0x03: File header for non-relocatable program (normally used for machine code)
 *    - 0x04: File header for SEQ data
 *    - 0x05: End-of-tape (EOT) marker
 * - Data *
 * - Checksum: A data bájtok XOR művelettel (0-ról kezdve)
 * 
 * Header Block (192 bytes) szerkezete (1,3)
 * - Type (1 byte)
 * - start address (2 byte : lh)
 * - end address (2 byte : lh)
 * - Filename
 * The data loaded into memory from a file header consists of:
For program headers (types 1 and 3), the first two bytes are the start address, followed by two bytes for the end address. When a file header is written, the start address is retrieved from memory locations $B2-$B3 (stal+stah) and the end address from $9D-$9E (eal+eah).
If the header is a SEQ file header (type 4), these address bytes are still written, but they are not used. The OPEN statement does not change these addresses, and the tape routines simply write the previously used values.
The addresses are followed by the filename, which is 16 bytes long. Unused characters are filled with spaces ($20).
The remainder of the file header is normally filled with spaces ($20).
An EOT marker (type 5) fills the cassette buffer entirely with spaces ($20).
 */
bool copyDataBlockFromTapToPrg( FILE *tapFile, FILE *prgFile ) { // Return false if eof
    if ( !feof( tapFile ) ) {
        long pos = ftell( tapFile );
printf( "Begin dual blocks copy at pos.: %04X\n", pos );
//    unsigned int clock = read_tap_block_clock( tapFile );
        skipRawTapeSyncron( tapFile, WORD1, true );
        if ( !feof( tapFile ) ) {
            pos = ftell( tapFile );
printf( "\t*** Begin first data block copy. First byte pos: %04X ...\n", pos );
            int block_size = copyBytesFromTapToPrg( tapFile, prgFile ); // First data block
printf( "\tSize = %d\n", block_size );
//            skipRawTapeSyncron( tapFile, LONG1, false );
            skipRawTapeSyncron( tapFile, WORD1, true );

            skipRawTapeSyncron( tapFile, WORD1, true );
printf( "\tBegin second block copy ...\n" );
            int block2size = copyBytesFromTapToPrg( tapFile, prgFile ); // Second data block
            if ( block_size != block2size ) err( "Block shincron error" );
//    skipRawTapeSyncron( tapFile, LONG1, true );
        }
    }
    return !feof( tapFile );
}

bool copyByteFromTapToPrg( FILE *tapFile, FILE *prgFile ) { // Return false if eof
    unsigned char byte = read_raw_byte( tapFile );
    fwrite( &byte, 1, 1, prgFile );
    return !feof( tapFile );
}

void writePrgStartAddr( FILE *prgFile, unsigned short addr ) {
    unsigned char high = addr >> 8, low = addr bitand 0xff;
    fwrite( &low, 1, 1, prgFile );
    fwrite( &high, 1, 1, prgFile );
}

bool convert( const char* tap_arg, const char* prg_arg ) {
    FILE* tapFile = fopen( tap_arg, "rb");
    if ( !tapFile) {
        printf("Unable to open tap file: %s\n", tap_arg);
        return false;
    }
    FILE* prgFile = fopen( prg_arg, "wb");
    if ( !prgFile) {
        printf("Unable to create prg file: %s\n", prg_arg);
        return false;
    }
    unsigned short startAddr = readTapHeader( tapFile );
    writePrgStartAddr( prgFile, startAddr );
    while( copyDataBlockFromTapToPrg( tapFile, prgFile ) );
    fclose( tapFile );
    fclose( prgFile );
    printf( "Conversion finished\n" );
    return true;
}

void help() { printf( "tap2prg tap_filename\n" ); }

int main(int argc, char* argv[]) {
    if ( argc > 1 ) {
        initPulseStat();
        int len = strlen( argv[ 1 ] );
        const char* tap_arg = argv[ 1 ];
        char* prg_arg = (char*)malloc( len + 1 );
        strcpy( prg_arg, tap_arg );
        prg_arg[ len-3 ] = 'p';
        prg_arg[ len-2 ] = 'r';
        prg_arg[ len-1 ] = 'g';
        convert( tap_arg, prg_arg );
        showPulseStat();
    } else {
        help();
    }
}
