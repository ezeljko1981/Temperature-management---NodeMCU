/*
  NodeMCU_Firebase.h - Library for reading values and sending to Firebase. Also getting commands from Firebase.
  Created by Željko Eremić
  Released into the public domain.
*/

#include "Arduino.h"
#include "NodeMCU_Firebase.h"
#include "Dht22.h"
#include "Relay.h"
#include <ESP8266WiFi.h>                                            // esp8266 library
#include <FirebaseArduino.h>                                        // firebase library

#include <NTPClient.h>  // Network Time Protocol (NTP)
#include <WiFiUdp.h>    //User Datagram Protocol (UDP) on port 123

#define FIREBASE_HOST "vtszr-9e108.firebaseio.com"                  // the project name address from firebase id
#define FIREBASE_AUTH "6caoXvFRNJW0CnCp3PYtAoEWvVq7Qg2NmqusddxG"    // the secret key generated from firebase Service accounts>Database secrets
                      
#define WIFI_SSID     "SSID-NAME"                                      // put your home or public wifi name 
#define WIFI_PASSWORD "SSID-PASSWORD"                                  // put your password of wifi ssid

//counters
int mc = 0;
String smc = "";
String m_archive_name = "";
String m_series_name = "default";

//time offset
const long utcOffsetInSeconds = 7200;  //UTC offset for your timezone in milliseconds
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", utcOffsetInSeconds);
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// sensors and actuators
Dht22  dht22_un;  //Object of Dht22 sensor
int relayP;
int fanP;  
Relay relayH;    // Heater
Relay relayF;    // Fan(s)
 
NodeMCU_Firebase::NodeMCU_Firebase(int relayHPin, int relayFPin){ 
  m_NextNodeMCU_FirebaseState = IDLE_STATE; 
  m_State = IDLE_STATE;
  time_step = Firebase.getInt("/drier_temp_keeping/params/time_step");delay(500);
  currentTime = millis();
  targetTime = currentTime + timeSpan;
  relayH.SetPin(relayHPin);
  relayF.SetPin(relayFPin);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(relayH.GetPin(), OUTPUT);
  pinMode(relayF.GetPin(), OUTPUT);  

  digitalWrite(relayH.GetPin(), LOW);
  digitalWrite(relayF.GetPin(), LOW);

  stateOfTemperature = "N/A";
  stateOfHumidity = "N/A";

  relayH.SetState(0);
  relayF.SetState(0);
};

void NodeMCU_Firebase::Loop()
{
  m_State = m_NextNodeMCU_FirebaseState;
  switch (m_State)
  {
    case IDLE_STATE:
      delay(10);
      timeSpan = 1 * time_step * 1000; // mm * sec * msec
      currentTime = millis();
      if(targetTime <= currentTime){
        targetTime = currentTime + timeSpan;
        m_NextNodeMCU_FirebaseState = MEASURE_STATE;;
        delay(100);
      }
      break;
      
    case MEASURE_STATE: 
      delay(1000);
      m_t_un = dht22_un.ReadDht22temperature();
      delay(3000);
      GetParamsFromDB();  
      m_NextNodeMCU_FirebaseState = CALCULATE_STATE; 
      delay(1000);
      Serial.print("DHT22 >> ");
      Serial.println(m_t_un);
      break;

   case CALCULATE_STATE:   
      BusinessLogic();
      m_NextNodeMCU_FirebaseState = WRITING_STATE; 
      delay(500);
      break;   

    case WRITING_STATE:
      GetTimeFromNTP();  
      smc = String(mc);     

      //node: current
      delay(250);
      Firebase.setFloat("/drier_temp_keeping/curent/t_un", m_t_un); 
      delay(250);                                  
      Firebase.setFloat("/drier_temp_keeping/curent/dayofweek", m_dayOfWeek);                  
      Firebase.setString("/drier_temp_keeping/curent/time", m_time); 
      Firebase.setInt("/drier_temp_keeping/curent/limit_r", temp_target + temp_margin_top);     
      Firebase.setInt("/drier_temp_keeping/curent/limit_l", temp_target - temp_margin_bottom);     

      //node: archive
      Firebase.setFloat("/drier_temp_keeping/" + m_archive_name + "/" + smc + "/t_un", m_t_un);
      Firebase.setFloat("/drier_temp_keeping/" + m_archive_name + "/" + smc + "/dayofweek", m_dayOfWeek);                  
      Firebase.setString("/drier_temp_keeping/" + m_archive_name + "/" + smc + "/time", m_time);
      Firebase.setInt("/drier_temp_keeping/" + m_archive_name + "/" + smc + "/is_heat_on", relayH.GetState());
      Firebase.setInt("/drier_temp_keeping/" + m_archive_name + "/" + smc + "/is_fan_on", relayF.GetState());
                      
      m_NextNodeMCU_FirebaseState = EXECUTECMD_STATE;   
      delay(500);
      break;

    case EXECUTECMD_STATE: 
    
     relayH.SetState(Firebase.getInt("/drier_temp_keeping/cmd/relay_h"));
     relayF.SetState(Firebase.getInt("/drier_temp_keeping/cmd/relay_f"));
     if(relayH.GetState() == 0){ digitalWrite(relayH.GetPin(), LOW); }else{ digitalWrite(relayH.GetPin(), HIGH); }   
     if(relayF.GetState() == 0){ digitalWrite(relayF.GetPin(), LOW); }else{ digitalWrite(relayF.GetPin(), HIGH); }
     m_NextNodeMCU_FirebaseState = IDLE_STATE;
     mc++;
     delay(500);
     break;   
  }
  delay(500);
}

void NodeMCU_Firebase::SetupFirebase()
{
  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); //try to connect with wifi
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP()); //print local IP address
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); // connect to firebase

  delay(2500);
  Firebase.setInt("/drier_temp_keeping/cmd/relay_f", 0);delay(500);
  Firebase.setInt("/drier_temp_keeping/cmd/relay_h", 0);delay(500);  
  m_series_name = Firebase.getInt("/drier_temp_keeping/params/series_name");delay(500);
      
  m_time = timeClient.getFormattedTime();
  m_dayOfWeek = timeClient.getDay();
  m_archive_name = m_series_name;
}

void NodeMCU_Firebase::GetParamsFromDB()
{
    delay_fan_after_heater = Firebase.getInt("/drier_temp_keeping/params/delay_fan_after_heater");delay(500);
    delay_fan_before_heater = Firebase.getInt("/drier_temp_keeping/params/delay_fan_before_heater");delay(500);
    temp_margin_bottom = Firebase.getInt("/drier_temp_keeping/params/temp_margin_bottom");delay(500);
    temp_margin_top = Firebase.getInt("/drier_temp_keeping/params/temp_margin_top");delay(500);
    temp_target = Firebase.getInt("/drier_temp_keeping/params/temp_target");delay(500);
    time_step = Firebase.getInt("/drier_temp_keeping/params/time_step");delay(500); 
}

void NodeMCU_Firebase::GetTimeFromNTP(){
    timeClient.update();
    m_time = timeClient.getFormattedTime();
    m_dayOfWeek = timeClient.getDay();
}

void NodeMCU_Firebase::BusinessLogic(){ 
    if(m_t_un > (temp_target + temp_margin_top)){ //iznad smo
      HeaterOff();
    }else  
    if(m_t_un < (temp_target - temp_margin_bottom)){ //ispod smo
      HeaterOn();
    }  
}

void NodeMCU_Firebase::FanOn()
{
  digitalWrite(relayF.GetPin(), HIGH); 
  Firebase.setInt("/drier_temp_keeping/cmd/relay_f", 1);
  relayF.SetState(1);
}

void NodeMCU_Firebase::FanOff()
{ 
  if(relayH.GetState() == 0){
    digitalWrite(relayF.GetPin(), LOW); 
    Firebase.setInt("/drier_temp_keeping/cmd/relay_f", 0);
    relayF.SetState(0);    
  }
}

void NodeMCU_Firebase::HeaterOn()
{
  FanOn(); 
  delay(delay_fan_before_heater * 1000); 
  digitalWrite(relayH.GetPin(), HIGH); 
  Firebase.setInt("/drier_temp_keeping/cmd/relay_h", 1);
  relayH.SetState(1);
}

void NodeMCU_Firebase::HeaterOff()
{
  digitalWrite(relayH.GetPin(), LOW); 
  Firebase.setInt("/drier_temp_keeping/cmd/relay_h", 0); 
  relayH.SetState(0);
  delay(delay_fan_after_heater * 1000);  
  FanOff();    
}


