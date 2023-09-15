#pragma once

#include "wled.h"
#include "HardwareSerial.h"

#define ESP_BT Serial
// #define ESP_BT_RX 3

class UsermodRPM_Meter : public Usermod
{
private:
  String rawData;
  String data;
  byte inData;
  char inChar;
  long OBD_Value;
  long maxRPM = 9000;
  
  int BT_StatePin = D7;
  bool BT_State = false;

  unsigned long refreshRate = 5;
  unsigned long now;
  
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

  // Read OBD data
  void readData()
  {
    rawData = "";
    if(ESP_BT.available())
    {
      inData = 0;
      inChar = 0;
      inData = ESP_BT.read();
      inChar = char(inData);
      rawData += inChar;
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
    RPM_Meter[F("raw-data")] = data;
    RPM_Meter[F("rpm-value")] = OBD_Value;
    RPM_Meter[F("max-rpm")] = maxRPM;
    RPM_Meter[F("refresh-rate")] = refreshRate;
  }

  static const char _name[];
  static const char _maxRPM[];
  static const char _refreshRate[];

public:
  void setup()
  {
    ESP_BT.begin(38400);
    Serial1.begin(38400);
    pinMode(BT_StatePin, INPUT);
  }

  void loop()
  {
    BT_State = digitalRead(BT_StatePin);
    
    // Skip executing if bluetooth not connected
    if(!BT_State)
    {
      return;
    }

    if(millis() - now > 1000 / refreshRate)
    {
      // Read RPM data from OBD
      Serial1.println("010C"); // Send RPM PID request
      readData();

      // Convert to decimal number
      data = rawData.substring(12, 14) + rawData.substring(15, 17);
      OBD_Value = strtol(data.c_str(), NULL, 16) / 4; //convert hex to decimnal
      ESP_BT.println(data);
      ESP_BT.println(OBD_Value);

      // Process to LED
      displayRPM((OBD_Value / maxRPM * 100));
      
      now = millis();
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
    RPM_Meter[FPSTR(_refreshRate)] = refreshRate;
  }
};

const char UsermodRPM_Meter::_name[] PROGMEM = "RPM-meter";
const char UsermodRPM_Meter::_maxRPM[] PROGMEM = "max_RPM";
const char UsermodRPM_Meter::_refreshRate[] PROGMEM = "refresh_rate";