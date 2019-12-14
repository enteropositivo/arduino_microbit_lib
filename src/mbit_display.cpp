/*
  mbit_display.cpp - LED Screen library for micro:bit
  (for "Arduino Core for Nordic Semiconductor nRF5 based boards")
  Copyright (c) 2017 Hideaki Tominaga. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "mbit_display.h"

volatile uint8_t current_row = 0;


void mbit_display::setBuffer(uint8_t row, uint8_t col, uint8_t data){
  col = col>=9?8:col;
  row = row>=3?2:row;

  bitWrite(buffer_data,  row*9 +col, data );

}
uint8_t mbit_display::getBuffer(uint8_t row, uint8_t col){
  col = col>=9?8:col;
  row = row>=3?2:row;
  return bitRead(buffer_data,  row*9 +col );
}

volatile int8_t render_count=0;
void mbit_display::renderRow() {
 if (!isEnabled )   return;

 if( buffer_data==0 && (--render_count)==0 ) return;
 if( buffer_data>0 ) render_count=4;

 //- ¿? ver antes si hay que mostrar algo
  update_showString();

 
  digitalWrite(rows[current_row], LOW); 
  ++current_row = current_row % 3;

  for (uint8_t c=0; c<9; c++) {
    digitalWrite(cols[c], !getBuffer(current_row,c) );
  }
  digitalWrite(rows[current_row], HIGH); 

}


void mbit_display::pset(const uint8_t x, const uint8_t y, const uint8_t mode) {
  if (!isEnabled)
    return;
 
  MBIT_LED pixel = LED_FROM_5X5[x][y];
  setBuffer(pixel.row, pixel.col, mode);

}

/**
   showData() - private method
*/
void mbit_display::showData(const uint8_t *DataArray) {
  if (!isEnabled)
    return;

   for (uint8_t x = 0; x < colCount; x++) {
      uint8_t data = DataArray[x];
      for (uint8_t y = 0; y < rowCount; y++) {
		    pset(y, x, data & 1);
        data >>= 1;
      }
    }

}

/**
   constructor
*/
mbit_display::mbit_display() {
}

/**
   begin()
   Set it once in setup().
*/
void mbit_display::begin() {
  for (uint8_t i = 0; i < max_cols; i++){
    pinMode(cols[i], OUTPUT);
  }
  for (uint8_t i = 0; i < max_rows; i++){
    pinMode(rows[i], OUTPUT);
  }

  buffer_data=0;
  isEnabled = true;
}

/**
 * clearScreen()
 * https://makecode.microbit.org/reference/basic/clear-screen
 */
void mbit_display::clearScreen() {

  buffer_data=0;
  isAnimated = true;
}

/**
   enable()
   https://makecode.microbit.org/reference/led/enable
*/
void mbit_display::enable(bool on) {
  isEnabled = on;
  current_row = 0;

}

/**
   plot()
   https://makecode.microbit.org/reference/led/plot
*/
void mbit_display::plot(const uint8_t x, const uint8_t y) {
  pset(y, x, 1);

}

/**
   plotBarGraph()
   https://makecode.microbit.org/reference/led/plot-bar-graph
*/
void mbit_display::plotBarGraph(const uint32_t value, const uint32_t high) {
  uint8_t LED_DATA[colCount] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  uint32_t dValue = value;
  uint32_t dHigh = high;
  if (dHigh < 15)
    dHigh = 15;
  if (dValue > dHigh)
    dValue = dHigh;

  uint8_t percentage = dValue * 100 / dHigh;
  uint8_t shiftBit = 5 - (percentage / 20);
  uint8_t remainder = (percentage % 20);
  uint8_t remainderBit = 1 << (shiftBit - 1);
  for (uint8_t i = 0; i < colCount; i++)
    LED_DATA[i] <<= shiftBit;
  if (remainder > 13) {
    LED_DATA[0] |= remainderBit;
    LED_DATA[4] |= remainderBit;
  }
  if (remainder > 6) {
    LED_DATA[1] |= remainderBit;
    LED_DATA[3] |= remainderBit;
  }
  LED_DATA[2] |= remainderBit;

  showData(LED_DATA);
}

/**
   point()
   https://makecode.microbit.org/reference/led/point
*/
bool mbit_display::point(const uint8_t x, const uint8_t y) {
  MBIT_LED pixel =  LED_FROM_5X5[y][x];
  return getBuffer(pixel.row,pixel.col);
}

/**
   showArrow()
   https://makecode.microbit.org/reference/basic/show-arrow
*/
void mbit_display::showArrow(const ArrowNames direction) {
  uint8_t d = (int)direction % 8;
  showData(MBIT_ARROWS[d]);
}

/**
   showAnimation()
   https://makecode.microbit.org/reference/basic/show-animation
*/
void mbit_display::showAnimation(const String str) {
  uint8_t numLineChar = (str.length() / rowCount);
  uint8_t numAnimation = (numLineChar + 1) / (colCount * 2);
  uint8_t charWidth = colCount * 2; 
  uint8_t* strBuf = new uint8_t[numAnimation * colCount]();

  for (uint32_t f = 0; f < numAnimation; f++) {
    for (int y = 0; y < rowCount; y++) {
      for (int x = 0; x < colCount; x++) {
        if (str.charAt((f * charWidth) + (y * numLineChar) + (x * 2)) == 0x23)
          bitWrite(strBuf[f * colCount + x], y, HIGH);
      }
    }
  }  
  
  /*  REVIISAR,  comentad por temas de compilacion solo
  for (uint32_t f = 0; f < numAnimation; f++) {
    uint32_t tick = millis();
    do {
      if (!isAnimated) {
        clearScreen();
        break;
      }
      showData(&strBuf[f * colCount]);
    } while ((millis() - tick) < interval);
  }
*/


  delete[] strBuf;
}

/**
   showIcon()
   https://makecode.microbit.org/reference/basic/show-icon
*/
void mbit_display::showIcon(const IconNames icon) {
   showData(MBIT_ICONS[(int)icon]);
}

/**
   showLeds()
   https://makecode.microbit.org/reference/basic/show-leds
*/
void mbit_display::showLeds(const String str) {

  String strleds = str;
  strleds.replace(" ", ""); // remove spaces from " . . # . . "

  for (int r = 0; r < rowCount; r++) {
    for (int c = 0; c < colCount; c++) {
      if (strleds.charAt((r * (colCount ) + (c ) )) == 0x23/*#*/){
     	   pset(r, c, HIGH);
	     }else{
		     pset(r, c, LOW);
	     }
    }
  }

}

/**
   showNumber()
   https://makecode.microbit.org/reference/basic/show-number
*/
void mbit_display::showNumber(int32_t value) {
  String dStr = String(value, DEC);
  showString(dStr);
}

/**
   showString()
   https://makecode.microbit.org/reference/basic/show-string
*/

uint8_t* strBuf = NULL;
uint32_t sindx = 0;     
uint32_t index_gt = 0;  
bool strIsSingleChar = false;
bool screen_bussy = false;
uint32_t str_anim_tick=millis();


void mbit_display::update_showString(){

  int interval=150;

   if(!screen_bussy ) return;

  if(strIsSingleChar){
     showData(&strBuf[0]);
      screen_bussy = false;
      delete[] strBuf;
     return;
  }

  if(!((sindx < index_gt) && !strIsSingleChar)){
    delete[] strBuf;
    screen_bussy = false;
    return;
  }
  
    if (!isAnimated) {
      clearScreen();
      screen_bussy = false;
     delete[] strBuf;
     return;
    }

    if( (millis() - str_anim_tick) < interval ) return ;
    str_anim_tick = millis();
    showData(&strBuf[sindx]);
   
    sindx++;
}

void mbit_display::showString(const String text) {
  
  if(screen_bussy ) return;
  
  
   delete[] strBuf;
  clearScreen();

  String dStr;
  if (text.length() == 0)
    dStr = " ";
  else if (text.length() == 1)
    dStr = text;
  else
    dStr = " " + text + " ";
  strIsSingleChar = (dStr.length() == 1);

  
  
  uint32_t bufSize = dStr.length() * (colCount + 1) - 1;
  strBuf = new uint8_t[bufSize];

  sindx = 0;
  for (uint32_t c = 0; c < dStr.length(); c++) {
    uint32_t charsindx = dStr.charAt(c);
    if ((charsindx < 0x20) || (charsindx > 0x7f))
      charsindx = 0x00;
    else
      charsindx = charsindx - 0x20;
    for (uint8_t x = 0; x < colCount; x++) {
      strBuf[sindx] = MBIT_FONT[charsindx][x];
      sindx++;
    }
    if (c < dStr.length() - 1) {
      strBuf[sindx] = 0x00;
      sindx++;
    }
  }

  sindx = 0;
  index_gt = bufSize - colCount - 1;

screen_bussy = true;

  /*do {
    if (!isAnimated) {
      clearScreen();
      break;
    }
    uint32_t tick = millis();
    do {
      showData(&strBuf[sindx]);
    } while ((millis() - tick) < interval);
    sindx++;
  } while ((sindx < index_gt) && !strIsSingleChar);

  delete[] strBuf;*/
}

/**
   stopAnimation()
   https://makecode.microbit.org/reference/led/stop-animation
*/
void mbit_display::stopAnimation() {
  screen_bussy = false;
  isAnimated = false;
}


/**
   unplot()
   https://makecode.microbit.org/reference/led/unplot
*/
void mbit_display::unplot(const uint8_t x, const uint8_t y) {
  pset(y, x, 0);
}

mbit_display display;
