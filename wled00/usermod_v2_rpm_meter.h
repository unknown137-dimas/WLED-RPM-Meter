#pragma once

#include "wled.h"

class UsermodRPM_Meter : public Usermod
{
private:
  // OBD related variables
  String rawData;
  String data;
  int rawDataLength;

  // RPM related variables
  unsigned int currentRPM = 0;
  unsigned int lastRPM = 0;
  unsigned int maxRPM = 9000;

  // Timing related variables
  unsigned long refreshRate = 8;
  unsigned long now;

  // Bluetooth related variables
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
    while (Serial.available())
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
    pinMode(btStatePin, INPUT);
  }

  void loop()
  {
    // Check BT connection to the OBD
    btState = digitalRead(btStatePin);
    if (btState != lastBtState)
    {
      lastBtState = btState;
    }

    // Only run command if BT is connected
    if (btState)
    {
      readData(); // Read RPM data from OBD

      if (millis() - now > 1000 / refreshRate)
      {
        Serial.println("010C1"); // Send RPM PID request

        // Check if rawData has enough RPM data
        rawDataLength = rawData.length();
        if (rawDataLength > 8)
        {
          data = rawData.substring(rawDataLength - 9, rawDataLength - 7) + rawData.substring(rawDataLength - 6, rawDataLength - 4); // Get RPM hex value
          currentRPM = strtol(data.c_str(), NULL, 16) / 4;                                                                          // Convert to RPM decimal value
        }

        now = millis();
      }

      // Only update LED if there is a change in RPM value
      if (currentRPM != lastRPM)
      {
        displayRPM(currentRPM * 100 / maxRPM); // Process to LED
        lastRPM = currentRPM;
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
    configComplete &= getJsonValue(RPM_Meter["refresh_rate"], refreshRate, 5);

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
    RPM_Meter[FPSTR(_refreshRate)] = refreshRate > 0 ? refreshRate : 5;
  }
};

const char UsermodRPM_Meter::_name[] PROGMEM = "RPM-meter";
const char UsermodRPM_Meter::_maxRPM[] PROGMEM = "max_RPM";
const char UsermodRPM_Meter::_refreshRate[] PROGMEM = "refresh_rate";