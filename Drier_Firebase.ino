/*
 System for automatic measurement and storage of data for temperture management
 NodeMCU_Firebase Library for :
 - Reading values from sensors for temperature and humididty and sending values to Firebase. 
 - Relays are for actuators

 The circuit:  

***************************
 INPUT: 
  DHT22(1)   NODEMCU
  -------------
  DATA       D0  
  VCC        5V
  GND        0V

  DHT22(2)   NODEMCU
  -------------
  DATA       D1 
  VCC        5V
  GND        0V

  HX711   WEIGHT
  -------------
  A-         wire4 (green)
  A+         wire2 (white)  
  E-         wire3 (black)
  E+         wire1 (red)
***************************

***************************
OUTPUT: 
  RELAY(3)   NODEMCU
  -------------
  DATA       D3 
  VCC        3.3V
  GND        0V

  PCFAN   NODEMCU
  -------------
  DATA       D4 
  VCC        5V
  GND        0V
***************************
  
 Library originally added Oct 27, 2023.
 by Željko Eremić

 This example code is in the public domain.
*/

#include "NodeMCU_Firebase.h"

NodeMCU_Firebase nodeMCU_Firebase(D3, D7); // 0 = D3

void setup(void) 
{
  Serial.begin(9600);
  nodeMCU_Firebase.SetupFirebase();
  while(1)
  {
    nodeMCU_Firebase.Loop();  
  } 
}
void loop(void) { }

