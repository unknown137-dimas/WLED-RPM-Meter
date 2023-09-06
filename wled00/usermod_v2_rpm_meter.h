#pragma once

#include "wled.h"

class UsermodRPM_Meter : public Usermod
{
private:
  byte inData;
  char inChar;
  String data;
  long rpmValue;

  void ReadData()
  {
    data = "";
    while(Serial.available() > 0)
    {
      inData = 0;
      inChar = 0;
      inData = Serial.read();
      inChar = char(inData);
      data = data + inChar;
    }
  }

public:
  void setup()
  {
    Serial.begin(9600);
    Serial.println("ATZ");
    ReadData();
    Serial.println("0100");
    ReadData();
  }

  void loop()
  {
    data = "";
    Serial.println("0105"); // Send RPM PID request
    ReadData();
  }
};