#pragma once

#include "wled.h"

class UsermodRPM_Meter : public Usermod
{
private:
  String rawData;
  String data;
  
  int currentRPM = 0;
  int lastRPM = 0;
  int maxRPM = 9000;
  
  unsigned long refreshRate = 5;
  unsigned long now;

  int btStatePin = D7;
  bool btState = false;
  bool lastBtState = false;
  
  /*
  OBD-II PID Requests, more info on https://en.wikipedia.org/wiki/OBD-II_PIDs#Standard_PIDs
  Examples:
  01 05 -> Engine coolant temperature
  01 0C -> Engine RPM
  01 11 -> Throttle position

  OBD Responses
  Examples:
  Engine coolant temperature -> 41 05 44
  Engine RPM -> 41 0C 00 00
  Throttle position -> 41 11 7D
  */

  // Read serial data
  void readData()
  {
    while(Serial.available() > 0)
    {
      rawData = Serial.readString();
    }
  }
  
  // Display RPM value to LED strip
  void displayRPM(int percentage)
  {
    Segment &Segment = strip.getSegment(0);
    Segment.intensity = percentage;
  }

  // Write usermod state to JSON
  void writeToJson(JsonObject &RPM_Meter)
  {
    RPM_Meter[F("bt-state")] = lastBtState;
    RPM_Meter[F("last-rpm")] = lastRPM;
    RPM_Meter[F("max-rpm")] = maxRPM;
    RPM_Meter[F("refresh-rate")] = refreshRate;
  }

  static const char _name[];
  static const char _maxRPM[];
  static const char _refreshRate[];

public:
  void setup()
  {
    Serial.begin(38400);
    Serial1.begin(38400);
    pinMode(btStatePin, INPUT);
  }

  void loop()
  {
    // Check BT connection to the OBD
    btState = digitalRead(btStatePin);
    if(btState != lastBtState)
    {
      lastBtState = btState;
    }

    // Only run command if BT is connected
    if(btState)
    {
      // Read RPM data from OBD
      readData();

      if(millis() - now > 1000 / refreshRate)
      {
        Serial1.println("010C"); // Send RPM PID request

        // Check if rawData has enough RPM data
        int rawDataLength = rawData.length();
        if(rawDataLength > 8)
        {
          data = rawData.substring(rawDataLength - 9, rawDataLength - 7) + rawData.substring(rawDataLength - 6, rawDataLength - 4); // Get RPM hex value
          currentRPM = strtol(data.c_str(), NULL, 16) / 4; // Convert to RPM decimal value

          // Only update LED if there is a change in RPM value
          if(currentRPM != lastRPM)
          {
            // Process to LED
            displayRPM(currentRPM * 100 / maxRPM);
            
            lastRPM = currentRPM;
          }
        }
        
        now = millis();
      }
    }
  }

  // Add turn signal state to JSON state API
  void addToJsonState(JsonObject &root)
  {
    JsonObject RPM_Meter = root[FPSTR(_name)];
    if (RPM_Meter.isNull())
    {
      RPM_Meter = root.createNestedObject(FPSTR(_name));
    }
    writeToJson(RPM_Meter);
  }

  // Read rpm meter configuration
  bool readFromConfig(JsonObject &root)
  {
    JsonObject RPM_Meter = root[FPSTR(_name)];
    bool configComplete = !RPM_Meter.isNull();

    // A 3-argument getJsonValue() assigns the 3rd argument as a default value if the Json value is missing
    configComplete &= getJsonValue(RPM_Meter["max_RPM"], maxRPM, 9000);
    configComplete &= getJsonValue(RPM_Meter["refresh_rate"], refreshRate, 1);
    if(RPM_Meter["refresh_rate"] != 0)
    {
      RPM_Meter["refresh_rate"] = 1;
    }

    return configComplete;
  }

  // Write turn signal configuration
  void addToConfig(JsonObject &root)
  {
    JsonObject RPM_Meter = root[FPSTR(_name)];
    if (RPM_Meter.isNull())
    {
      RPM_Meter = root.createNestedObject(FPSTR(_name));
    }
    RPM_Meter[FPSTR(_maxRPM)] = maxRPM;
    if(refreshRate > 0)
    {
      RPM_Meter[FPSTR(_refreshRate)] = refreshRate;
    }
  }
};

const char UsermodRPM_Meter::_name[] PROGMEM = "RPM-meter";
const char UsermodRPM_Meter::_maxRPM[] PROGMEM = "max_RPM";
const char UsermodRPM_Meter::_refreshRate[] PROGMEM = "refresh_rate";