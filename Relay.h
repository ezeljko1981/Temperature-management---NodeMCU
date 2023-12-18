/*
  relay.h - Library for controlling relay state.
  Created by Željko Eremić, May 30, 2020.
  Released into the public domain.
*/

#ifndef Relay_h
#define Relay_h

class Relay
{
  public:
    Relay();
    void SetPin(int relayPin);
    int GetPin();
    void SetState(int relayState);
    int GetState();
  private:
    int m_relayState;
    int m_relayPin;
};

#endif

