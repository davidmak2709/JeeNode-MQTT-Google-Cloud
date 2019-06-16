// GOTOVO
#include <JeeLib.h>

#define LDR 0

Port leds (2);
void setup () {
  rf12_initialize(3, RF12_868MHZ, 100);
  Serial.begin(9600);

  pinMode(14+LDR, INPUT);
  digitalWrite(14+LDR, 1);

  leds.mode2(OUTPUT);
}
  
void loop () {
  byte value = 255 - analogRead(LDR) / 4;
  Serial.println(value);

  char c[4];
  String str = String(value);
  str.toCharArray(c,4);
  rf12_sendStart(0, c, sizeof c);
  

  leds.digiWrite2(0x100);
  delay(3500);
  leds.digiWrite2(~0x100);
  delay(1500);
}
