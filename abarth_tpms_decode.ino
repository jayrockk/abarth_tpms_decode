
#include <EEPROM.h>

#include "globals.h"
#include "cc1101.h"
#include "tpms.h"
#include "abarth_read.h"
#include "abarth_tpms.h"
#include "display.h"

void setup() {

  byte resp;
  unsigned int t;
  int LEDState = LOW;
  int i;
  int mcount;

  //SPI CC1101 chip select set up
  pinMode(CC1101_CS, OUTPUT);
  digitalWrite(CC1101_CS, HIGH);

  Serial.begin(115200);

  pinMode(LED_RX, OUTPUT);
  pinMode(RXPin, INPUT);
  pinMode(CDPin, INPUT);

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


#if USE_ADAFRUIT
  if (!display.begin(SSD1306_EXTERNALVCC, I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
#else
  Wire.begin();
  Wire.setClock(400000L);
  display.begin(&Adafruit128x64, I2C_ADDRESS);
  display.setFont(Adafruit5x7);
#endif

  Serial.println(F("SSD1306 initialised OK"));

  digitalWrite(LED_RX, LED_ON);
  LEDState = HIGH;

  pinMode(DEBUGPIN, OUTPUT);

  // Clear the buffer
#if USE_ADAFRUIT
  display.clearDisplay();
  display.display();
#else
  display.clear();
#endif

  InitTPMS();
  UpdateDisplay();

  digitalWrite(LED_RX, LED_OFF);

  setRxState();
  
  /* Start receiver state machine */
  InitDataBuffer();
  clear_statistics();
  receiver_state = STATE_IDLE;

  attachInterrupt( digitalPinToInterrupt(RXPin), edge_interrupt, CHANGE);
    // attachInterrupt( digitalPinToInterrupt(CDPin), carrier_sense_interrupt, CHANGE);
  
  cli();
  PCMSK0 |= _BV(PCINT1);
  PCICR |= _BV(PCIE0);
  sei();

}

ISR( PCINT0_vect) {
  carrier_sense_interrupt();
}

void loop() {

  int ch;
  int ByteCount = 0;
  bool TPMS_Changed;


  while( Serial.available()) {
    ch = Serial.read();

    if( ch == 'c') {
      clear_statistics();

    } else if( ch == '\n') {
      dump_statistics();
      Serial.print("cd pin is: ");
      Serial.println(digitalRead(CDPin));
      
      for( int i=0; i<4; i++) {
        Serial.print(i);
        Serial.print(" Temp    : ");
        Serial.println( TPMS[i].TPMS_Temperature);
        Serial.print("  Pressure: ");
        Serial.println( TPMS[i].TPMS_Pressure);
      }
    }
  }

  TPMS_Changed = Check_TPMS_Timeouts();

  if ( receiver_state == STATE_DATA_AVAILABLE)
  {
    if ((CD_Width >= 9500) && (CD_Width <= 10500)) //jayrock set upper border to 10500
    {
      ByteCount = decode_tpms( Timings, TimingsIndex, first_edge_state);

      TPMS_Changed = true;  //indicates the display needs to be updated.

      Serial.print( ByteCount);
      Serial.println( F(" Bytes decoded"));
    }

    InitDataBuffer();
    receiver_state = STATE_IDLE;
  }

  if (TPMS_Changed == true)
  {
    UpdateDisplay();
    TPMS_Changed = false;
  }
}
