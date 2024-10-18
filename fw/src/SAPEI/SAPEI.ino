#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "pins.h"
#include "config.h"

bitreg_u PORTb_bitreg;
bitreg_u PINb_bitreg;
bitreg_u PORTc_bitreg;
bitreg_u PINc_bitreg;
bitreg_u PORTd_bitreg;
bitreg_u PINd_bitreg;

void setup(){
  DDRD = _BV(DDD5) | _BV(DDD6) | _BV(DDD7);
  Serial.begin(115200);
  
  PORTd_bitreg.REG = &PORTD;

  Serial.println("HOLA!");
}

void loop(){
  for (int i = 0; i < 3; i++){
  PORTD ^= _BV(5 + i);
  _delay_ms(500);
  PORTD ^= _BV(5 + i);
  _delay_ms(500);
  }
  PORTdBits.b5 = 1;
  PORTdBits.b6 = 1;
  PORTdBits.b7 = 1;
  _delay_ms(1000);
  PORTd = 0;

}
