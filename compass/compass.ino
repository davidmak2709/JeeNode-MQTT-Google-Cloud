//GOTOVO
#include <JeeLib.h>

PortI2C myBus (1);
Port leds (2);
CompassBoard compass (myBus);

void setup() {
    Serial.begin(9600);
    Serial.println("\n[compass]");
    rf12_initialize(2, RF12_868MHZ, 100);
    leds.mode2(OUTPUT);
}

void loop() {
    int x = (int) compass.heading(); 
    x = x * -1;
    if(x < 0){
       x = 360 + x;
    }
    Serial.println(x);
    char c[4];
    String str;
    str = String(x);
    str.toCharArray(c,4);
    rf12_sendStart(0, c, sizeof c);
    
    leds.digiWrite2(0x100);
    delay(3500);
    leds.digiWrite2(~0x100);
    delay(1500);
}
