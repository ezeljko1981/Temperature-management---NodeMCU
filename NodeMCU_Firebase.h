/*
  NodeMCU_Firebase.h - Library for reading values and sending to Firebase. Also getting commands from Firebase.
  Created by Željko Eremić
  Released into the public domain.
*/
#ifndef NodeMCU_Firebase_h
#define NodeMCU_Firebase_h

#include "Arduino.h"

class NodeMCU_Firebase
{
  public:
    NodeMCU_Firebase(int relayHPin, int relayFPin);   
    void Loop();
    void SetupFirebase();
    void GetParamsFromDB();
    void GetTimeFromNTP();
    void FanOn();
    void FanOff();    
    void HeaterOn();
    void HeaterOff();
    void BusinessLogic();
    
  private:  
    #define IDLE_STATE                0 
    #define MEASURE_STATE             3
    #define WRITING_STATE             4
    #define EXECUTECMD_STATE          5
    #define CALCULATE_STATE           6

    uint8_t m_NextNodeMCU_FirebaseState;
    uint8_t m_State; 

    String stateOfTemperature;
    String stateOfHumidity;

    float m_h_max;
    float m_h_min;
    float m_t_max;
    float m_t_min;
    String m_time;    
    int m_dayOfWeek;
    float m_t_un;
    int mc;
    String smc;
    String m_archive_name;
    String m_series_name;

    //params
    int delay_fan_after_heater;
    int delay_fan_before_heater;
    int temp_margin_bottom;
    int temp_margin_top;
    int temp_target;
    int time_step = 15;
         
    unsigned long currentTime;
    unsigned long targetTime;
    unsigned long timeSpan;  
};

#endif


