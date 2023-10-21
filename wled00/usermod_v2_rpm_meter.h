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
  unsigned int maxRPM = 7500;

  // Timing related variables
  unsigned long updateRate = 8;
  unsigned long refreshRate = 8;
  unsigned long flashRate = 20;
  unsigned long updateNow;
  unsigned long refreshNow;
  unsigned long warningNow;

  // Bluetooth related variables
  int btStatePin = D7;
  bool btState = false;
  bool lastBtState = false;

  // Mod related variables
  bool enable = true;

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
    if (!Segment.on)
    {
      Segment.on = true;
    }
    if (percentage > 100)
    {
      Segment.intensity = 100;
    }
    else
    {
      Segment.intensity = percentage;
    }
  }

  // Flash the LED strip
  void warningLight(unsigned long delay)
  {
    Segment &Segment = strip.getSegment(0);
    if (millis() - warningNow > delay)
    {
      Segment.on = !Segment.on;
      warningNow = millis();
    }
  }

  // Write usermod state to JSON
  void writeToJson(JsonObject &RPM_Meter)
  {
    RPM_Meter[F("enable")] = enable;
    RPM_Meter[F("bt-state")] = lastBtState;
    RPM_Meter[F("last-rpm")] = lastRPM;
    RPM_Meter[F("max-rpm")] = maxRPM;
    RPM_Meter[F("update-rate-hz")] = updateRate;
    RPM_Meter[F("refresh-rate-hz")] = refreshRate;
    RPM_Meter[F("flash-rate-hz")] = flashRate;
  }

  static const char _name[];
  static const char _enable[];
  static const char _maxRPM[];
  static const char _updateRate[];
  static const char _refreshRate[];
  static const char _flashRate[];

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

    // Set LED strip to 100% to easily configure the color
    if (!enable)
    {
      displayRPM(100);
    }

    // Only run command if BT is connected and enabled
    if (btState && enable)
    {
      readData(); // Read RPM data from OBD

      if (millis() - updateNow > 1000 / updateRate)
      {
        Serial.println("010C1"); // Send RPM PID request

        // Check if rawData has enough RPM data
        rawDataLength = rawData.length();
        if (rawDataLength > 8)
        {
          data = rawData.substring(rawDataLength - 9, rawDataLength - 7) + rawData.substring(rawDataLength - 6, rawDataLength - 4); // Get RPM hex value
          currentRPM = strtol(data.c_str(), NULL, 16) / 4;                                                                          // Convert to RPM decimal value
        }

        updateNow = millis();
      }

      // Refresh LED
      if (millis() - refreshNow > 1000 / refreshRate)
      {
        displayRPM(currentRPM * 100 / maxRPM); // Process to LED
        lastRPM = currentRPM;
        refreshNow = millis();
      }

      // flash the LED strip if exceeding the maximum RPM value
      if (currentRPM >= maxRPM)
      {
        warningLight(1000 / flashRate);
      }
    }
  }

  // Add rpm meter state to JSON state API
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
    configComplete &= getJsonValue(RPM_Meter["enable"], enable, true);
    configComplete &= getJsonValue(RPM_Meter["max_RPM"], maxRPM, 7500);
    configComplete &= getJsonValue(RPM_Meter["update_rate_hz"], updateRate, 5);
    configComplete &= getJsonValue(RPM_Meter["refresh_rate_hz"], refreshRate, 5);
    configComplete &= getJsonValue(RPM_Meter["flash_rate_hz"], flashRate, 20);

    return configComplete;
  }

  // Write rpm meter configuration
  void addToConfig(JsonObject &root)
  {
    JsonObject RPM_Meter = root[FPSTR(_name)];
    if (RPM_Meter.isNull())
    {
      RPM_Meter = root.createNestedObject(FPSTR(_name));
    }
    RPM_Meter[FPSTR(_enable)] = enable;
    RPM_Meter[FPSTR(_maxRPM)] = maxRPM;
    RPM_Meter[FPSTR(_updateRate)] = updateRate > 0 ? updateRate : 5;
    RPM_Meter[FPSTR(_refreshRate)] = refreshRate > 0 ? refreshRate : 5;
    RPM_Meter[FPSTR(_flashRate)] = flashRate > 0 ? flashRate : 20;
  }
};

const char UsermodRPM_Meter::_name[] PROGMEM = "RPM-meter";
const char UsermodRPM_Meter::_enable[] PROGMEM = "enable";
const char UsermodRPM_Meter::_maxRPM[] PROGMEM = "max_RPM";
const char UsermodRPM_Meter::_updateRate[] PROGMEM = "update_rate_hz";
const char UsermodRPM_Meter::_refreshRate[] PROGMEM = "refresh_rate_hz";
const char UsermodRPM_Meter::_flashRate[] PROGMEM = "flash_rate_hz";