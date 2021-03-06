/*
 *  Clock with WS8212 60 LED Ring
 *  
 *  MAWISoft 2017
 *
 *  subroutines for color schemes
*/

#include "clock_schemes.h"


void defineColorScheme(byte scheme) {

  // define color schemes

  switch (scheme) {

    default:

    case SKY:
      rgbDivisions = RGB(65, 12, 0);  // light bloody orange
      rgbBackground = RGB(0, 0, 0);   // black
      rgbHour = RGB(232, 0, 0);       // dark red
      rgbHouredge = RGB(12, 0, 0);    // light red
      rgbMinute = RGB(53, 53, 98);    // violet blue
      rgbSecond = RGB(0, 240, 12);    // dark green
      break;

    case SPRING:

      rgbDivisions = RGB(48, 12, 0);  // very light bloody orange
      rgbBackground = RGB(0, 0, 0);   // black
      rgbHour = RGB(240, 26, 0);      // dark crimson
      rgbHouredge = RGB(14, 2, 0);    // very light crimson
      rgbMinute = RGB(96, 196, 0);    // moose green
      rgbSecond = RGB(180, 100, 0);   // gold
      break;

    case NIGHT:

      rgbDivisions = RGB(48, 48, 48); // turquoise
      rgbBackground = RGB(0, 0, 0);   // black
      rgbHour = RGB(168, 2, 46);      // purple
      rgbHouredge = RGB(24, 0, 12);   // light purple
      rgbMinute = RGB(216, 52, 136);  // lilac
      rgbSecond = RGB(218, 127, 0);   // light orange
      break;

    case DAY:

      rgbDivisions = RGB(0, 32, 0);   // light green
      rgbBackground = RGB(0, 0, 0);   // black
      rgbHour = RGB(0, 0, 236);       // dark blue
      rgbHouredge = RGB(16, 30, 32);  // light blue
      rgbMinute = RGB(180, 100, 0);   // gold
      rgbSecond = RGB(240, 26, 0);    // dark crimson
      break;

    case USER:

      if (flagUserColorsSet) {
        rgbDivisions = rgbUserDivisions;
        rgbBackground =  rgbUserBackground;
        rgbHour = rgbUserHour;
        rgbHouredge = rgbUserHouredge;
        rgbMinute = rgbUserMinute;
        rgbSecond = rgbUserSecond;
      }
      else {
        rgbDivisions = RGB(36, 0, 36);  // light purple
        rgbBackground = RGB(0, 0, 0);   // black
        rgbHour = RGB(196, 64, 0);      // dark organge
        rgbHouredge = RGB(36, 8, 0);    // light orange
        rgbMinute = RGB(36, 178, 34);   // smaragd
        rgbSecond = RGB(248, 178, 0);   // blue green
      }
      break;
  }
}


byte setCustomColors(char *strColor) {

  byte intColor[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  const char delim[2] = ",";              //char *token = "";
  int i = 0;
  // get the first token
  char *token = strtok(strColor, delim);
  // walk through other tokens
  while ( token != NULL && i < 12 ) {
    intColor[i] = atoi(token);
    token = strtok(NULL, delim);
    i++;
  }

#ifdef DEBUG
  Serial.println("Custom Colors set: ");
  for(i=0;i<12;i++){
          Serial.printf(" %i,", intColor[i]);
  }
  Serial.println(" \n");  
#endif


  // read gamma corrected RGB table
  rgbUserDivisions = RGB(pgm_read_byte(&gamma_R[intColor[0]]), pgm_read_byte(&gamma_G[intColor[1]]), pgm_read_byte(&gamma_B[intColor
  rgbUserHour = RGB(pgm_read_byte(&gamma_R[intColor[3]]), pgm_read_byte(&gamma_G[intColor[4]]), pgm_read_byte(&gamma_B[intColor[5]]));
  rgbUserMinute = RGB(pgm_read_byte(&gamma_R[intColor[6]]), pgm_read_byte(&gamma_G[intColor[7]]), pgm_read_byte(&gamma_B[intColor[8]]));
  rgbUserSecond = RGB(pgm_read_byte(&gamma_R[intColor[9]]), pgm_read_byte(&gamma_G[intColor[10]]), pgm_read_byte(&gamma_B[intColor[11]]));
  rgbUserHouredge = rgbUserHour.nscale8_video(212);    
  rgbUserBackground = RGB(0, 0, 0);            
  
  flagUserColorsSet = true;

  return 1;									                // success
}

void setCustomElement(RGB clockfaceElement, byte r, byte g, byte b) {

  clockfaceElement = RGB(r, g, b);

}




