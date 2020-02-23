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

#include "globals.h"
#include "abarth_read.h"
#include "abarth_tpms.h"

/* ***** ***** */

void check( bool ok);

static bool unit_test_bits();
static bool unit_test_manchester_dec();
static bool unit_test_manchester_enc();
static bool unit_test_decode();
static bool unit_test_encode();

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
  
  printf("\n%d tests ok, %d tests failed.\n\n",test_ok, test_failed);
  
  return 0;
}

void check( bool ok)
{
  if( ok) {
    printf("OK.\n");
    test_ok++;
  } else {
    printf("FAILED.\n");
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
  bitArray_t b;
  byteArray_t data;
  
  printf("*** unit_test_manchester_enc ***\n");

  clear_bit_array( &b);
  clear_byte_array( &data);

  

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

  TimingsIndex = sizeof(timings)/sizeof(byte);
  memcpy( (void*)Timings, (void*)timings, TimingsIndex);
  FirstEdgeState = HIGH;
 
  bytes_decoded = decode_tpms();

  if( bytes_decoded != 9) {
    return false;
  }
  
  return true;
}

bool unit_test_encode()
{
  bitArray_t b;
  byteArray_t data;
  
  printf("*** unit_test_encode ***\n");

  clear_bit_array( &b);
  clear_byte_array( &data);

  /* Valid data set. 9th byte is XOR checksum.
   *   [0]=0f [1]=38 [2]=cb [3]=2f [4]=67 [5]=9e [6]=3e [7]=5b [8]=4f
   */

  set_byte( &data, 0, 0x0f);
  set_byte( &data, 1, 0x38);
  set_byte( &data, 2, 0xcb);
  set_byte( &data, 3, 0x2f);
  set_byte( &data, 4, 0x67);
  set_byte( &data, 5, 0x9e);
  set_byte( &data, 6, 0x3e);
  set_byte( &data, 7, 0x4f);
  

  return true;
}

#endif
