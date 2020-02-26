
#include "globals.h"
#include "cc1101.h"
#include "tpms.h"
#include "abarth_tpms.h"
#include "abarth_send.h"

unsigned long last_send = 0;
bool ledState = false;

void setup() {

  byte resp;
  unsigned int t;
  int i;
  int mcount;

  //SPI CC1101 chip select set up
  pinMode(CC1101_CS, OUTPUT);
  digitalWrite(CC1101_CS, HIGH);

  Serial.begin(115200);

  pinMode(LED_RX, OUTPUT);
  pinMode(RXPin, INPUT);
  pinMode(TXPin, OUTPUT);
  digitalWrite( TXPin, LOW);

  SPI.begin();
  //initialise the CC1101
  CC1101_reset();

  delay(2000);

  Serial.println(F("Starting..."));

  setIdleState();
  digitalWrite(LED_RX, LED_OFF);

  resp = readStatusReg(CC1101_PARTNUM);
  Serial.print(F("Part no: "));
  Serial.println(resp, HEX);

  resp = readStatusReg(CC1101_VERSION);
  Serial.print(F("Version: "));
  Serial.println(resp, HEX);

  pinMode(DEBUGPIN, OUTPUT);
  
  clear_statistics();
}

void loop() {

  int ch;
  bitLength_t bitno;
  
  byteArray_t bytes;
  bitArray_t bits;
  
  while( Serial.available()) {
    ch = Serial.read();
    
    if( ch == '\n') {
      dump_statistics();
    }
  }

  if( millis() > last_send + 2000) {
    last_send = millis();

    digitalWrite(LED_RX, ledState);
    ledState = !ledState;
    
    clear_byte_array( &bytes);

    /* Fill 8 bytes with id, pressure and temperature.
     * Status and unknown byte is set to 0.
     */
    append_byte( &bytes, 0xab); // 4 bytes id
    append_byte( &bytes, 0xa1);
    append_byte( &bytes, 0x24);
    append_byte( &bytes, 0x01);

    append_byte( &bytes, 0x00);

     // pressure
    append_byte( &bytes, (byte)(2.1 * 100 / 1.38));
     // temperature
    append_byte( &bytes, 12 +50);
    
    append_byte( &bytes, 0x00);

    /* Adds checksum
     * and returns Manchester encoded bits including sync and preamble bits.
     */
    encode_tpms( &bytes, &bits);

    Serial.print( bits.length);
    Serial.println( " bits to send.");
    for( bitno = 0; bitno < bits.length; bitno++) {
      Serial.print(get_bit( &bits, bitno));
    }
    Serial.println();

    send_tpms( &bits);
  }
}
