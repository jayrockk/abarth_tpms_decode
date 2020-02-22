/*
 * Set below mode according to you preference
 */

//#define DISPLAYMODE_SETUP 1 //displays pressure (large) and ID (small)
#define DISPLAYMODE_TEMPERATURE 1 //displays temperature (large) and pressure (small)
//#define DISPLAYMODE_PRESSURE 1 //displays pressure (large) and temperatur (small)

#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

SSD1306AsciiWire display;

char DisplayTimeoutBar(unsigned long TimeSinceLastUpdate)
{
      int HowCloseToTimeout;
      HowCloseToTimeout = (int)(TimeSinceLastUpdate/(TPMS_TIMEOUT/5));

      switch(HowCloseToTimeout)
      {
        case 0: 
           return('5');
           break;
        case 1: 
           return('4');
           break;
        case 2: 
           return('3');
           break;
        case 3: 
           return('2');
           break;
        case 4: 
           return('1');
           break;
        default: 
           return('0');
           break;
                      
      }
}

#if DISPLAYMODE_SETUP

   void ShowTitle()
  {
    display.clear();
    display.set1X();             // Normal 1:1 pixel scale
    display.setCursor(0, 0);
    display.println(" Abarth TPMS Monitor");
    display.println("   (TWJ Solutions)");
  
  }

  void UpdateDisplay()
  {
    int i;
    int x = 0;
    int y = 0;
    char s[6];
  
    ShowTitle();
  
    for (i = 0; i < 4; i++)
    {
      switch (i)
      {
        case 0:
          x = 2;
          y = 2;
          break;
        case 1:
          x = 60;
          y = 2;
          break;
        case 2:
          x = 2;
          y = 5;
          break;
        case 3:
          x = 60;
          y = 5;
          break;
      }
  
      display.setCursor(x, y);
  
      if (TPMS[i].TPMS_ID != 0)
      {
        display.setFont(Adafruit5x7);
        display.set2X();
        
        //in bar...
        dtostrf(TPMS[i].TPMS_Pressure, 3, 2, s);
        display.print(s);
        display.setCursor(x, y+2);
        display.setFont(Adafruit5x7);
        display.set1X();
        display.print(TPMS[i].TPMS_ID,HEX);
        
        //display.setFont(System5x7);          
        //display.print(DisplayTimeoutBar(millis() - TPMS[i].lastupdated));
      }
      
    }
    
  }
  
#endif


#if DISPLAYMODE_TEMPERATURE

   void ShowTitle()
  {
    display.clear();
    display.set1X();             // Normal 1:1 pixel scale
    display.setCursor(0, 0);
    display.println(" Abarth TPMS Monitor");
    display.println("   (TWJ Solutions)");
  
  }

  void UpdateDisplay()
  {
    int i;
    int x = 0;
    int y = 0;
    char s[6];
  
    ShowTitle();
  
    for (i = 0; i < 4; i++)
    {
      switch (i)
      {
        case 0:
          x = 2;
          y = 2;
          break;
        case 1:
          x = 60;
          y = 2;
          break;
        case 2:
          x = 2;
          y = 5;
          break;
        case 3:
          x = 60;
          y = 5;
          break;
      }
  
      display.setCursor(x, y);
  
      if (TPMS[i].TPMS_ID != 0)
      {
        display.setFont(Adafruit5x7);
        display.set2X();
        dtostrf(TPMS[i].TPMS_Temperature, 2, 0, s);
        display.print(" ");
        display.print(s);
        display.setFont(System5x7);
        display.print(char(128));  //degrees symbol
        display.setFont(Adafruit5x7);
        display.print("C");
        display.print("  ");


        display.setCursor(x, y+2);
        display.setFont(Adafruit5x7);
        display.set1X();
        //in bar...
        dtostrf(TPMS[i].TPMS_Pressure, 3, 2, s);
        display.print(s);

        display.setFont(System5x7);          
        display.print(DisplayTimeoutBar(millis() - TPMS[i].lastupdated));
      }
      
    }
    
  
  }
  
#endif
  
#if DISPLAYMODE_PESSUUE

   void ShowTitle()
  {
    display.clear();
    display.set1X();             // Normal 1:1 pixel scale
    display.setCursor(0, 0);
    display.println(" Abarth TPMS Monitor");
    display.println("   (TWJ Solutions)");
  
  }

  void UpdateDisplay()
  {
    int i;
    int x = 0;
    int y = 0;
    char s[6];
  
    ShowTitle();
  
    for (i = 0; i < 4; i++)
    {
      switch (i)
      {
        case 0:
          x = 2;
          y = 2;
          break;
        case 1:
          x = 60;
          y = 2;
          break;
        case 2:
          x = 2;
          y = 5;
          break;
        case 3:
          x = 60;
          y = 5;
          break;
      }
  
      display.setCursor(x, y);
  
      if (TPMS[i].TPMS_ID != 0)
      {
        display.setFont(Adafruit5x7);
        display.set2X();
        
        //in bar...
        dtostrf(TPMS[i].TPMS_Pressure, 3, 2, s);
        display.print(s);

        display.setCursor(x, y+2);
        display.setFont(Adafruit5x7);
        display.set1X();
        dtostrf(TPMS[i].TPMS_Temperature, 2, 0, s);
        display.print(" ");
        display.print(s);
        display.setFont(System5x7);
        display.print(char(128));  //degrees symbol
        display.setFont(Adafruit5x7);
        display.print("C");
        display.print("  ");

        display.setFont(System5x7);          
        display.print(DisplayTimeoutBar(millis() - TPMS[i].lastupdated));
      }
      
    }
    
  
  }
  
#endif
