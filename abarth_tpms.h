/*
 * Abarth 124 TPMS Sensor decoding
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <Arduino.h>

#ifndef byte
typedef unsigned char byte;
#endif

/*#ifndef boolean
typedef byte boolean;
#endif*/

#ifndef TRUE
# define FALSE 0
# define TRUE (!FALSE)
#endif

/*************** input data ***************************/

/*const byte timings[] = {
    100,60,48,52,52,52,52,52,52,52,56,52,52,52,52,52,52,52,52,52,56,52,52,52,52,52,52,52,52,104,104,60,48,52,56,48,52,104,52,52,56,52,52,52,104,52,52,108,48,52,56,52,104,52,52,52,56,100,52,56,104,52,52,104,104,104,60,48,104,52,52,104,104,108,56,48,52,56,48,52,104,104,60,48,104,52,52,104,52,52,56,52,52,52,104,52,52,104,52,56,52,52,104,100,112,52,48,108,52,52,52,52,52,52,52,52,104,104,60,48,52,56,100,52,52,104,104,52,56,104,52,52,52,56,52,52,104,56,48,104,52,52,52,56,104,52,52,104,100
};/*

/********************************************************/

#define MAX_BITS   256
#define MAX_BYTES  ((MAX_BITS + 7) / 8)

typedef struct bitArray_t {
    int bit_count;
    byte bits[MAX_BYTES];    
} bitArray_t;

typedef struct byteArray_t {
    int byte_count;
    byte bytes[MAX_BYTES];
} byteArray_t;


/****************** PARAMETERS ***************************/

const byte preamble_length = 16;
const byte preamble[] = { 0xAA, 0xA9 };

#define MANCHESTER_DECODING_MASK  0b1010101010101010

/* Pulse range in micro seconds */
#define MIN_SHORT_usec   ((byte) 30)
#define MAX_SHORT_usec   ((byte) 70)
#define MIN_LONG_usec    ((byte) 80)
#define MAX_LONG_usec    ((byte)130)
#define MIN_SYNC_usec    ((byte)175)

/* Pulse type */
#define INVALID ((byte)0)
#define SHORT   ((byte)1)
#define LONG    ((byte)2)
#define SYNC    ((byte)3)


/***************** forward defines **********************/

void clear_bit_array( bitArray_t *data);
void print_bit_array( bitArray_t *bits);
boolean get_bit( bitArray_t *data, byte bitno);
void set_bit( bitArray_t *data, byte bitno, boolean value);

void clear_byte_array( byteArray_t *data);
void print_byte_array( byteArray_t *bytes);
int pulse_type( byte time);
void bit_decode( const byte timing[], int count, boolean start_value, bitArray_t *data);

byte find_preamble( bitArray_t *data);

void manchester_decode( bitArray_t *bits, int start, byteArray_t *data);

boolean checksum_xor( byteArray_t *data);

void unit_test_bits();
void unit_test_manchester();

/********************************************************/

int wolfgang_main()
{
    bitArray_t decoded_bits;   // Decoded timing bits
    byteArray_t data;          // Manchester decoded bytes

    byte data_start;           // Data start after preamble
    
    byte a_byte;
    byte start;

    
    // unit_test_bits();
    // unit_test_manchester();

    
    // Wir versuchen mit start level TRUE 
    printf("Trying start value: TRUE\n");
    bit_decode( Timings, sizeof(Timings)/sizeof(byte), TRUE, &decoded_bits);
    data_start = find_preamble( &decoded_bits);

    if( data_start == 0) {
        // Hat nicht funktioniert, nochmal mit FALSE 
        printf("Trying start value: FALSE\n");
        bit_decode( Timings, sizeof(Timings)/sizeof(byte), FALSE, &decoded_bits);
        data_start = find_preamble( &decoded_bits);
    }
    
    print_bit_array( &decoded_bits);

    if( data_start == 0) {
        printf( "Preamble not found\n\n");
        
    } else {
        printf( "Preamble found. Data starts at index %d\n\n", data_start);
        
        manchester_decode( &decoded_bits, data_start, &data);

        printf("Manchester decode found %d bytes\n\n", data.byte_count);

        print_byte_array( &data);

        if( checksum_xor( &data)) {
            printf("\nChecksum OK\n\n");

            printf("ID      : %02x %02x %02x %02x\n",
                   data.bytes[0],data.bytes[1],data.bytes[2],data.bytes[3]);
            
            printf("Pressure: %.2f bar\n", (double)data.bytes[5] * 1.38 / 100);
            printf("Temp    : %d C\n", data.bytes[6] - 50);
            
        } else {
            printf("\nChecksum FAIILED\n");
        }
    }
}

/********************************************************/

void clear_bit_array( bitArray_t *data)
{
    int i;

    data->bit_count = 0;

    for( i = 0; i < MAX_BYTES; i++) {
        data->bits[i] = 0;
    }
}

void print_bit_array( bitArray_t *bits)
{
    int i;
    
    for( i = 0; i < bits->bit_count; i++) {
        printf("%c", get_bit( bits, i) ? '1' : '0');
        
        if( ((i+1) % 40) == 0) { /* Newline every 40 bits */
            printf("\n");
        }
    }
    printf("\n\n");
}

/* 
 * Bitno starts at 0.
 */
boolean get_bit( bitArray_t *data, byte bitno)
{
    return (data->bits[bitno/8] & (1 << (7-(bitno % 8)))) ? TRUE : FALSE;
}

void set_bit( bitArray_t *data, byte bitno, boolean value)
{
    if( value) {
        data->bits[bitno/8] |= (byte)(1 << (7-(bitno % 8)));
    } else {
        data->bits[bitno/8] &= ~((byte)(1 << (7-(bitno % 8))));
    }
}

/********************************************************/

void clear_byte_array( byteArray_t *data)
{
    int i;

    data->byte_count = 0;

    for( i = 0; i < MAX_BYTES; i++) {
        data->bytes[i] = 0;
    }
}

void print_byte_array( byteArray_t *bytes)
{
    int i;

    for( i = 0; i < bytes->byte_count; i++) {
        printf("Byte [%d] %02x (%d)\n", i, bytes->bytes[i], bytes->bytes[i]);
    }
}

int pulse_type( byte time)
{
    if( time >= MIN_SHORT_usec && time <= MAX_SHORT_usec) {
        return SHORT;
    }
    if( time >= MIN_LONG_usec && time <= MAX_LONG_usec) {
        return LONG;
    }
    if( time >= MIN_SYNC_usec) {
        return SYNC;
    }

    return INVALID;
}

/********************************************************/

/* Konvertiert timings zu bits.
 *
 * Parameter:
 *   const byte timing[]    - Timing array
 *   int count              - Anzahl der bytes in timing array
 *   data                   - Rückgabe der Bit Werte
 *
 * Return:
 *   nix
 */
void bit_decode( const byte timing[], int count, boolean start_value,  bitArray_t *data)
{
    byte i = 0;
    int bit_count = 0;
    int timing_len_usec = 0;  
    boolean level = start_value;

    printf("Input: %d Timings\n", count);
    
    clear_bit_array( data);

    for( i = 0; i < count; i++) {
        
        timing_len_usec += timing[i];        
        
        switch( pulse_type( timing[i] ) ) {
        case LONG: /* Ergibt 2 Pulse */
            set_bit( data, bit_count++, level);
            /* Fall through */

        case SHORT: /* Ergibt einen Puls */
            set_bit( data, bit_count++, level);
            break;
        }

        level = !level;
    }

    data->bit_count = bit_count;
    printf("Output: %d bits. Length %d usec\n", bit_count, timing_len_usec);
}

/********************************************************/

/* Findet die Präambel im bit array.
 * Gibt den index des ersten bits nach der Präambel zurück.
 *
 * Parameter:
 *
 * Return: > 0    - Start Index nach der Präambel
 *         0      - Keine Präambel gefunden
 */
byte find_preamble( bitArray_t *data)
{
    byte bit_idx = 0;
    byte start = 0;
    
    byte saved_start = 0;
    byte pre_idx = 0;

    byte pre;
    byte pre_no;
    
    while( bit_idx < data->bit_count - preamble_length) {

        if( pre_idx == 0) { /* Remember start for restart on match failure */
            saved_start = bit_idx;
        }
        
        pre = preamble[pre_idx/8];
        pre_no = pre_idx % 8;

        if( get_bit( data, bit_idx) == ((pre & (1 << (7-pre_no))) >> (7-pre_no)) ) { 
            /* Match, advance preamble and bit index */
            pre_idx++;
            bit_idx++;
            
            if( pre_idx >= preamble_length) { /* All preamble bits found, done */
                start = bit_idx;
                break;
            }
        } else { /* fail, restart with next bit */
            pre_idx = 0;
            bit_idx = saved_start + 1;
        }
    }

    return start;
}

/********************************************************/

/* Manchester decode geht am einfachsten über ein XOR verknüpfung
 * mit dem Clock Signal  ( 1010101010101.... )
 */
void manchester_decode( bitArray_t *bits, int start, byteArray_t *data)
{
    int i, n;

    unsigned int an_int = 0;
    byte a_byte = 0;
    byte bit_count = 0;

    clear_byte_array( data);
    
    for( i = start; i < bits->bit_count; i++) {

        if( bit_count == 16) { /* Decode 16 bits via XOR with clock signal to one byte */
            an_int ^= MANCHESTER_DECODING_MASK;
            a_byte = 0;
            
            for( n = 0; n < 8; n++) {
                a_byte <<= 1;
                a_byte |= ((an_int & 0xc000) ? 1 : 0);
                an_int <<= 2;
            }
            
            data->bytes[data->byte_count++] = a_byte;
            bit_count = 0;
            an_int = 0;
        }

        an_int <<= 1;
        an_int |= get_bit( bits, i) ? 1 : 0;
        bit_count++;
    }

    if( bit_count > 0) {
        printf("WARNING: %d bits left over.\n", bit_count);
    }
}

/* Einfache XOR checksum.
 */
boolean checksum_xor( byteArray_t *data)
{
    byte checksum;
    
    if( data->byte_count > 2) {
        checksum = data->bytes[0];
        for( int i=1; i<data->byte_count; i++) {
            checksum ^= data->bytes[i];
        }

        if( checksum == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

/*********************** UNIT TESTS *****************************/
/*********************** UNIT TESTS *****************************/
/*********************** UNIT TESTS *****************************/

/*void unit_test_manchester()
{
    bitArray_t b;
    byteArray_t data;

    clear_bit_array( &b);
    clear_byte_array( &data);
    
    set_bit( &b, 1, TRUE);
    set_bit( &b, 2, TRUE);
    set_bit( &b, 5, TRUE);
    set_bit( &b, 6, TRUE);
    set_bit( &b, 8, TRUE);
    set_bit( &b, 11, TRUE);
    set_bit( &b, 13, TRUE);
    set_bit( &b, 15, TRUE);
    set_bit( &b, 16, TRUE);
    set_bit( &b, 18, TRUE);
    set_bit( &b, 21, TRUE);
    
    b.bit_count = 22;

    print_bit_array( &b);
    
    manchester_decode( &b, 0, &data);

    print_byte_array( &data);

    if( data.bytes[0] == 0xa7) {
        printf("OK\n");
    } else {
        printf("FAILED, expected 0xA7\n");
    }
    
    exit(0);
}

void unit_test_bits()
{
    bitArray_t b;
    int i;
    
    clear_bit_array( &b);

    for( i = 0; i < 16; i += 2) {
        set_bit( &b, i, TRUE);
    }

    for( i = 0; i<16; i++) {
        printf("%c", get_bit( &b, i) ? '1' : '0');
    }
    printf( "\n");

    for( i = 0; i < 16; i += 2) {
        set_bit( &b, i, FALSE);
        set_bit( &b, i+1, TRUE);
    }

    for( i = 0; i<16; i++) {
        printf("%c", get_bit( &b, i) ? '1' : '0');
    }
    printf( "\n");

    clear_bit_array( &b);

    for( i = 0; i < 4; i++) {
        set_bit( &b, i, TRUE);
    }

    for( i = 12; i < 16; i++) {
        set_bit( &b, i, TRUE);
    }

    for( i = 0; i<16; i++) {
        printf("%c", get_bit( &b, i) ? '1' : '0');
    }
    printf( "\n");

    exit(0);
}*/
