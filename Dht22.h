/*
  Dht22.h - Library for reading from Dht22 sensor
  Created by Željko Eremić, May 30, 2020.
  Released into the public domain.

  DHT22      NODEMCU
  -------------
  DATA       D1
  VCC        5V
  GND        0V
  
*/
#ifndef Dht22_h
#define Dht22_h
#include "Arduino.h"

class Dht22
{
  public:
    Dht22();
    Dht22(int pin);
    float ReadDht22temperature();
    float ReadDht22humidity();    
};

#endif

