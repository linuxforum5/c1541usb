#include "CBM.hpp"

char CBM::pc2cbm( char c ) {
    switch ( (unsigned char)c ) {
        case '!' ... '?' : return c; break;    //  !"#$%&'()*+,-./0123456789:;<=>?
        case 'A' ... 'Z' : return c; break;    //  A-Z
        case 'a' ... 'z' : return c-32; break;    //  a=129 t=
        case ' ' :
        case 160 :
        case '_' : return ' '; break;
        default : return '?'; break;
    }
}

string CBM::fn2cbm( const char* filename ) {
    const int max_fn_length = 16;
    char cbm_fn[ max_fn_length + 1 ];
    int i = 0;
    while( i<max_fn_length && filename[i] ) {
        cbm_fn[ i ] = CBM::pc2cbm( filename[ i ] );
        i++;
    }
    cbm_fn[ i ] = 0;
    return cbm_fn; // 160=>' ', max length=23,.
}
