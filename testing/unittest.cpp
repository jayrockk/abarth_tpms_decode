/*
 * This file is not Arduino code!
 * 
 * It contains unit test intended to be running on a Linux box.
 * 
 * usage:   ./run_unittest.sh
 *
 */

#ifndef ARDUINO

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define UNITTEST

#ifndef byte
typedef unsigned char byte;
#endif

#ifndef false
# define false ((byte)0)
# define true  (!false)
#endif

#define LOW  (0)
#define HIGH (1)

#define F(x) x
#define PROGMEM
#define HEX  'h'

/* Fake Arduino Serial class */
class SerialOutput {

  public:
    void print( char *s) {
      printf( "%s", s);
    }

    void print( int b) {
      printf( "%d", b);
    }
    
    void print( int b, char f) {
      if( f == HEX) {
        printf( "%02x", b);
      } else {
        printf( "%d", b);
      }
    }

    void println() {
      printf( "\n");
    }

    void println( char *s) {
      printf( "%s\n", s);
    }
    
    void println( int b) {
      printf( "%d\n", b);
    }
    
    void println( int b, char f) {
      if( f == HEX) {
        printf( "%02x", b);
      } else {
        printf( "%d", b);
      }
    }
};

SerialOutput Serial;

long millis()
{
  clock_t t = clock();
  
  return t * 1000 / CLOCKS_PER_SEC;  
}

#include "../globals.h"
#include "../tpms.h"
#include "../abarth_tpms.h"

/* ***** ***** */

void check( bool ok);

static bool unit_test_bits();
static bool unit_test_manchester_dec();
static bool unit_test_manchester_enc();
static bool unit_test_decode();
static bool unit_test_encode();
static bool unit_test_e2e();

/* ***** ***** */

static int test_ok = 0;
static int test_failed = 0;

/* ***** ***** */

int main( int argc, char *argv[])
{
  check( unit_test_bits());
  check( unit_test_manchester_dec());
  check( unit_test_manchester_enc());
  check( unit_test_decode());
  check( unit_test_encode());
  check( unit_test_e2e());
  
  printf("RESULT: %d tests ok, %d tests failed.\n\n",test_ok, test_failed);
  
  return 0;
}

void check( bool ok)
{
  if( ok) {
    printf("OK.\n\n");
    test_ok++;
  } else {
    printf("FAILED.\n\n");
    test_failed++;
  }
}

/* Set and test various bit pattern */
bool unit_test_bits()
{
  bitArray_t b;
  int i;

  printf("*** unit_test_bits ***\n");
  
  clear_bit_array( &b);

  for( i = 0; i < 16; i += 2) {
    set_bit( &b, i, true);
  }

  for( i = 0; i < 16; i += 2) {
    if( !get_bit( &b, i) || get_bit( &b, i+1)) {
      printf("test fail 1\n");
      return false;      
    }
  }

  for( i = 0; i < 16; i += 2) {
    set_bit( &b, i, false);
    set_bit( &b, i+1, true);
  }

  for( i = 0; i < 16; i += 2) {
    if( get_bit( &b, i) || !get_bit( &b, i+1)) {
      printf("test fail 2\n");
      return false;      
    }
  }

  clear_bit_array( &b);

  for( i = 0; i < 4; i++) {
    set_bit( &b, i, true);
  }

  for( i = 12; i < 16; i++) {
    set_bit( &b, i, true);
  }

  for( i = 0; i<4; i++) {
    if( !get_bit( &b, i)) {
      printf("test fail 3\n");
      return false;      
    }
  }
  for( i = 4; i<12; i++) {
    if( get_bit( &b, i)) {
      printf("test fail 4\n");
      return false;      
    }
  }
  for( i = 12; i<16; i++) {
    if( !get_bit( &b, i)) {
      printf("test fail 5\n");
      return false;      
    }
  }

  return true;
}

bool unit_test_manchester_dec()
{
  bitArray_t b;
  byteArray_t data;

  printf("*** unit_test_manchester_dec ***\n");
  
  clear_bit_array( &b);
  clear_byte_array( &data);
   
  set_bit( &b, 1, true);
  set_bit( &b, 2, true);
  set_bit( &b, 5, true);
  set_bit( &b, 6, true);
  set_bit( &b, 8, true);
  set_bit( &b, 11, true);
  set_bit( &b, 13, true);
  set_bit( &b, 15, true);
  set_bit( &b, 16, true);
  set_bit( &b, 18, true);
  set_bit( &b, 21, true);
   
  // b.bit_count = 22;

  print_bit_array( &b);
   
  manchester_decode( &b, 0, &data);

  print_byte_array( &data);

  if( data.bytes[0] == 0xa7) {
    return true;
  }
    
  return false;
}

bool unit_test_manchester_enc()
{
  bitArray_t bits;
  byteArray_t data;
  
  printf("*** unit_test_manchester_enc ***\n");

  clear_bit_array( &bits);
  clear_byte_array( &data);

  data.bytes[0] = 0xa7;
  data.length = 1;

  manchester_encode( &data, &bits);

  if( bits.length != 16) {
    printf("test fail 1\n");
    return false; 
  }

  /* TODO: Test bits as well */
  
  return true;
}

bool unit_test_decode()
{
  int bytes_decoded;

  byte timings[] = {
    124,48,52,56,56,48,56,52,52,52,52,52,52,52,52,56,52,52,52,52,52,52,52,
    52,52,52,56,52,52,104,104,52,52,56,52,52,52,100,52,56,52,56,52,48,104,
    52,52,108,52,52,52,52,104,52,56,52,52,104,52,52,104,52,52,104,104,104,
    60,48,108,48,52,104,104,112,52,48,56,52,52,52,100,112,52,48,108,52,48,
    104,60,48,52,52,52,52,104,52,56,104,52,52,52,52,52,52,104,52,52,56,52,
    104,52,52,52,56,52,52,52,52,104,48,56,108,100,104,40,56,104,104,52,52,
    108,100,112,52,48,104,52,52,52,56,52,52,104,104,104
  };

  printf("*** unit_test_decode ***\n");

  bytes_decoded = decode_tpms(timings, sizeof(timings)/sizeof(byte), HIGH);

  if( bytes_decoded != 9) {
    return false;
  }
  
  return true;
}

bool unit_test_encode()
{
  byte checksum;
  bitLength_t data_start;
  byteLength_t i;
  bitArray_t preamble;
  bitArray_t bits;
  byteArray_t data;
  byteArray_t dec_data;
  
  printf("*** unit_test_encode ***\n");

  clear_bit_array( &bits);
  clear_bit_array( &preamble);
  clear_byte_array( &data);
  clear_byte_array( &dec_data);
  
  /* Valid data set. 9th byte is XOR checksum.
   *   [0]=0f [1]=38 [2]=cb [3]=2f [4]=67 [5]=9e [6]=3e [7]=5b [8]=4f
   */

  append_byte( &data, 0x0f);
  append_byte( &data, 0x38);
  append_byte( &data, 0xcb);
  append_byte( &data, 0x2f);
  append_byte( &data, 0x67);
  append_byte( &data, 0x9e);
  append_byte( &data, 0x3e);
  append_byte( &data, 0x5b);

  checksum = checksum_xor( &data, data.length);

  append_byte( &data, checksum);

  if( !check_checksum( &data)) {
    printf("test fail 1\n");
    return false; 
  }

  /* Start with sync and preamble */
  bits.bits[0] = 0xAA;
  bits.bits[1] = 0xAA;
  bits.bits[2] = 0xAA;
  bits.bits[3] = 0xA9;
  bits.length = 32;

  /* Add manchester encoded bits */
  manchester_encode( &data, &bits);
  print_bit_array( &bits);

  /* Now decode again to check validity */
  
  preamble.bits[0] = 0xAA;
  preamble.bits[1] = 0xA9;
  preamble.length = 16;
  
  data_start = find_preamble( &bits, &preamble);

  printf("Data starts at: %d\n", data_start);

  if( data_start == 0) {
    printf("test fail 2\n");
    return false;
  }

  manchester_decode( &bits, data_start, &dec_data);
  
  printf("Data length: %d\n", dec_data.length);
  print_byte_array( &dec_data);

  if( dec_data.length != 9) {
    printf("test fail 3\n");
    return false;
  }

  if( !check_checksum( &dec_data)) {
    printf("test fail 4\n");
    return false;    
  }
  printf("Checksum ok.\n");

  /* Verify each decoded byte */
  for( i = 0; i < data.length; i++) {
    if( !(get_byte( &data, i) == get_byte( &dec_data, i))) {
      printf("test fail 5\n");
      return false;
    }
  }

  return true;
}

bool unit_test_e2e()
{
  byte timings[255];
  unsigned int timing_len;
  unsigned int bytes_decoded;
  
  byte checksum;
  
  bitLength_t data_start;
  bitLength_t bitno;
  
  byteLength_t i;
  bitArray_t preamble;
  bitArray_t bits;
  byteArray_t data;
  byteArray_t dec_data;
  
  printf("*** unit_test_e2e ***\n");

  clear_bit_array( &bits);
  clear_bit_array( &preamble);
  clear_byte_array( &data);
  clear_byte_array( &dec_data);
  
  /* Valid data set. 9th byte is XOR checksum.
   *   [0]=0f [1]=38 [2]=cb [3]=2f [4]=67 [5]=9e [6]=3e [7]=5b [8]=4f
   */

  printf("ENCODING [0]=0f [1]=38 [2]=cb [3]=2f [4]=67 [5]=9e [6]=3e [7]=5b [8]=4f\n");

  append_byte( &data, 0x0f);
  append_byte( &data, 0x38);
  append_byte( &data, 0xcb);
  append_byte( &data, 0x2f);
  append_byte( &data, 0x67);
  append_byte( &data, 0x9e);
  append_byte( &data, 0x3e);
  append_byte( &data, 0x5b);

  checksum = checksum_xor( &data, data.length);

  append_byte( &data, checksum);

  if( !check_checksum( &data)) {
    printf("test fail 1\n");
    return false; 
  }

  /* Start with sync and preamble */
  bits.bits[0] = 0xAA;
  bits.bits[1] = 0xAA;
  bits.bits[2] = 0xAA;
  bits.bits[3] = 0xA9;
  bits.length = 32;

  /* Add manchester encoded bits */
  manchester_encode( &data, &bits);
  print_bit_array( &bits);

  /* create timings array from encoded bits */
  timing_len = 0;
  for( bitno = 0; bitno < bits.length; bitno++) {
    if( bitno < bits.length-1 && (get_bit( &bits, bitno) == get_bit( &bits, bitno+1))) {
      printf("100 "); // long pulse worth two bits
      timings[timing_len++] = 100;
      bitno++;
    } else {
      printf("50 ");  // short pulse
      timings[timing_len++] = 50;
    }
  }
  printf("\n");

  /* Decode again */
  bytes_decoded = decode_tpms(timings, timing_len, HIGH);

  if( bytes_decoded != 9) {
    return false;
  }

  return true;
}

#endif
