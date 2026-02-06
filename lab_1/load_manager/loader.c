#include "loader.h"
/***
 * TODO
 * This Function Loads The Binary Code In The Guest Physical Memory.
 * - Copy the code in the guest physical memory area.
 */
int load_vm_code( const uint8_t *code ) {
    int ret = 0 ;
    uint8_t *mem = get_memory() ; // We get a ptr to the memory
    if ( mem == NULL ){
        errx( 1 , "Error while returning the memory. \n" ) ;
        return -1 ;
    }

    int codeSize = 12 ;

    for ( int i = 0 ; i < codeSize ; i++ ){ // On copie octet par octet
        *( mem + 0x1000 + i ) = *( code + i ) ;
    }
    return 0 ;
}