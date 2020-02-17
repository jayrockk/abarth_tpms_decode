#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>

#include "globals.h"
#include "cc1101.h"
#include "display.h"
#include "abarth_read.h"
#include "abarth_tpms.h"

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

  SPI.begin();
  //initialise the CC1101
  CC1101_reset();

  delay(2000);

  Serial.println("Starting...");

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


  digitalWrite(LED_RX, LED_OFF);

  setRxState();
}

void loop() {

  static long lastts = millis();
  int ByteCount = 0;
  bool TPMS_Changed;


  TPMS_Changed = Check_TPMS_Timeouts();

  //wait for carrier status to go low
  while (GetCarrierStatus == true)
  {
  }

  //wait for carrier status to go high  looking for rising edge
  while (GetCarrierStatus == false)
  {
  }

  if (ReceiveMessage())
  {
    Serial.println("jetzt wolfgang_main....");

    ByteCount = wolfgang_main();

    Serial.print( ByteCount);
    Serial.println( "Bytes decoded");
    
    TPMS_Changed = true;  //indicates the display needs to be updated.
  }
  
    /*if (TPMS_Changed == true)
    {
      UpdateDisplay();
      TPMS_Changed = false;
   }*/
}
