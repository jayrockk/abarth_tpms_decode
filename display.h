
   #include "SSD1306Ascii.h"
   #include "SSD1306AsciiWire.h"

   SSD1306AsciiWire display;


   void ShowTitle()
  {
    display.clear();
  
    display.set1X();             // Normal 1:1 pixel scale
    //display.setTextColor(WHITE, BLACK);       // Draw white text
  
    display.setCursor(0, 0);
    display.println(" Abarth TPMS Monitor");
    display.println("   (TWJ Solutions)");
 
  
  }

  char DisplayTimeoutBar(unsigned long TimeSinceLastUpdate)
  {
      int HowCloseToTimeout;
      HowCloseToTimeout = (int)(TimeSinceLastUpdate/(TPMS_TIMEOUT/5));

      switch(HowCloseToTimeout)
      {
        case 0: 
           //return(FONTBAR_7);
           return('5');
           break;
        case 1: 
           //return(FONTBAR_5);
           return('4');
           break;
        case 2: 
           //return(FONTBAR_3);
           return('3');
           break;
        case 3: 
           //return(FONTBAR_2);
           return('2');
           break;
        case 4: 
           //return(FONTBAR_1);
           return('1');
           break;
        default: 
           //return(FONTBAR_0);
           return('0');
           break;
                      
      }
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
        case 12:
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

        //display vertical bar showing how long since last update 7 bars = recent 1 bar = nearing timeout (at timeout it will be removed from display altogether)
        display.setFont(System5x7);          
        display.print(DisplayTimeoutBar(millis() - TPMS[i].lastupdated));
      }


    }
    
  
  }



/*#include <U8x8lib.h>
//#include <Arduino.h>


//U8X8_SH1106_128X64_NONAME_HW_I2C u8x8( U8X8_PIN_NONE);
//U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8X8_PIN_NONE);


/*void ShowTitle()
{
    u8x8.setFont(u8x8_font_artossans8_u);
    u8x8.drawString(0,0,"TPMS Monitor");
    u8x8.refreshDisplay();
}

void SetupDisplay()
{
  u8x8.begin();
  u8x8.setPowerSave(0);
  ShowTitle();
}



void UpdateDisplay()
{
}*/
