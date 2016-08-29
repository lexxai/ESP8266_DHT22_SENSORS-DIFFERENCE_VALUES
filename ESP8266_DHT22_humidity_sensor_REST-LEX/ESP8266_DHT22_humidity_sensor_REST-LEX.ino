/*
  Created by Igor Jarc
  See http://iot-playground.com for details
  Please use community fourum on website do not contact author directly

  External libraries:
  DTH - https://github.com/iot-playground/EasyIoT-Cloud/tree/master/libraries/DHT

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  version 2 as published by the Free Software Foundation.
*/
#include <ESP8266WiFi.h>
#include "DHT.h"


//REDEFINE AP definitions, EasyIoT Cloud definitions 
#include "private.h" 

#ifndef _PRIVATE_DATA
//AP definitions - update this
#define AP_SSID     "APROUTER"
#define AP_PASSWORD "APROUTERPWD"

// EasyIoT Cloud definitions - change EIOT_CLOUD_TEMP_INSTANCE_PARAM_ID and EIOT_CLOUD_HUM_INSTANCE_PARAM_ID
#define EIOT_CLOUD_TEMP_INSTANCE_PARAM_ID_1    "USERID/PARAMID1"
#define EIOT_CLOUD_HUM_INSTANCE_PARAM_ID_1     "USERID/PARAMID2"
#define EIOT_CLOUD_TEMP_INSTANCE_PARAM_ID_2    "USERID/PARAMID3"
#define EIOT_CLOUD_HUM_INSTANCE_PARAM_ID_2     "USERID/PARAMID4"
#endif

#define REPORT_INTERVAL 60 // in sec

//#define DEBUG_PROG
#define DEBUG_SERVER 0

#ifdef DEBUG_PROG
  #define DEBUG_PRINTLN(x)  Serial.println(x)
  #define DEBUG_PRINT(x)    Serial.print(x)
#else
  #define DEBUG_PRINTLN(x) 
  #define DEBUG_PRINT(x)
#endif



#define EIOT_CLOUD_ADDRESS     "cloud.iot-playground.com"
#define EIOT_CLOUD_PORT        40404


// pins of conneced sensor 
#define DHT22_1_PIN 4  // DHT22-1 pin
#define DHT22_2_PIN 0  // DHT22-2 pin

//calibration sensors value for get same values in eq. enviroment
#define CALIBRATE_HUM_2    3.5
#define CALIBRATE_TEMP_2   0.4

//tolerance deviation of one sensor for send to server data
#define TOLERANCE_HUM      0.8
#define TOLERANCE_TEMP     0.2

//alarm for big diference bettwen two sensors
#define ALARM_HUM 10
#define ALARM_TEMP 2




#define USEDSLEEP 1


struct oldvalue {
  float t1;
  float h1;
  float t2;
  float h2;
} old;

float hum_1;
float temp_1;
float hum_2;
float temp_2;
float diffrenceH;
float diffrenceT;

DHT dht_1;
DHT dht_2;

bool posted;
bool isconnected = false;

void setup() {
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  Serial.begin(115200);

  //wifiConnect();

  DEBUG_PRINTLN();
  DEBUG_PRINTLN();
  DEBUG_PRINTLN("\n\nSensor\tStatus\tHumidity (%)\tDiffH\tTemperature (C)\tDiffT\tPOSTED");



#if !USEDSLEEP
}

void loop() {
#else
  //DEBUG_PRINTLN(sizeof(old));
  ESP.rtcUserMemoryRead(0, (uint32_t*) &old, sizeof(old));
#endif
  //  DEBUG_PRINTLN(old.h1);
  //  DEBUG_PRINTLN(old.t1);
  //  DEBUG_PRINTLN(old.h2);
  //  DEBUG_PRINTLN(old.t2);

  dht_1.setup(DHT22_1_PIN); // data pin sensor 1
  delay(dht_1.getMinimumSamplingPeriod());

  hum_1 = dht_1.getHumidity();
  temp_1 = dht_1.getTemperature();

  dht_2.setup(DHT22_2_PIN); // data pin sensor 2
  delay(dht_2.getMinimumSamplingPeriod());

  hum_2 = dht_2.getHumidity() + CALIBRATE_HUM_2;
  temp_2 = dht_2.getTemperature() + CALIBRATE_TEMP_2;
  diffrenceH = hum_2 - hum_1;
  diffrenceT = temp_2 - temp_1;
  bool AlarmH = abs(diffrenceH) > ALARM_HUM;
  bool AlarmT = abs(diffrenceT) > ALARM_TEMP;

  DEBUG_PRINT("1\t");
  DEBUG_PRINT(dht_1.getStatusString());
  DEBUG_PRINT("\t");
  DEBUG_PRINT(hum_1);
  DEBUG_PRINT("\t\t");
  DEBUG_PRINT(diffrenceH);
  DEBUG_PRINT("\t");
  DEBUG_PRINT(temp_1);
  DEBUG_PRINT("\t\t");
  DEBUG_PRINT(diffrenceT);


  posted = false;


  if (abs(temp_1 - old.t1) > TOLERANCE_TEMP)
  {
    sendTeperature(temp_1, 1);
    old.t1 = temp_1;
    posted = true;
  }

  if (abs(hum_1 - old.h1) > TOLERANCE_HUM)
  {
    sendHumidity(hum_1, 1);
    old.h1 = hum_1;
    posted = true;
  }

  DEBUG_PRINT("\t");
  DEBUG_PRINTLN(posted);

  DEBUG_PRINT("2\t");
  DEBUG_PRINT(dht_2.getStatusString());
  DEBUG_PRINT("\t");
  DEBUG_PRINT(hum_2);
  DEBUG_PRINT("\t\t");
  DEBUG_PRINT(diffrenceH);
  DEBUG_PRINT("\t");
  DEBUG_PRINT(temp_2);
  DEBUG_PRINT("\t\t");
  DEBUG_PRINT(diffrenceT);

  posted = false;

  if (abs(temp_2 - old.t2) > TOLERANCE_TEMP)
  {
    sendTeperature(temp_2, 2);
    old.t2 = temp_2;
    posted = true;
  }

  if (abs(hum_2 - old.h2) > TOLERANCE_HUM)
  {
    sendHumidity(hum_2, 2);
    old.h2 = hum_2;
    posted = true;
  }

  DEBUG_PRINT("\t");
  DEBUG_PRINTLN(posted);

  if (AlarmH) DEBUG_PRINTLN("ALARM BY HUMIDITY: " + String(abs(diffrenceH)));
  if (AlarmT) DEBUG_PRINTLN("ALARM BY TEMPERATURE: " + String(abs(diffrenceT)));

#if !USEDSLEEP
  int cnt = REPORT_INTERVAL;
  while (cnt--)
    delay(1000);
#else
  ESP.rtcUserMemoryWrite(0, (uint32_t*) &old, sizeof(old));
  DEBUG_PRINTLN("Enter to sleep " + String(REPORT_INTERVAL) + " sec");
  //disconnectDHTs();
  ESP.deepSleep(REPORT_INTERVAL * 1000000,WAKE_RF_DISABLED);
  delay(100);
#endif

  //unsigned long previousMillis=millis();
  //while (millis() - previousMillis < REPORT_INTERVAL) (
  //  wdt_reset();
  //);

}

void disconnectDHTs() {
  pinMode(DHT22_1_PIN, OUTPUT);
  digitalWrite(DHT22_1_PIN, LOW);
  pinMode(DHT22_2_PIN, OUTPUT);
  digitalWrite(DHT22_2_PIN, LOW);
}

void wifiConnect()
{
  if (!isconnected) {
    int tryconnet = 10;
    DEBUG_PRINT("[ Connecting to AP");
    WiFi.forceSleepWake();
    delay(1);
    WiFi.mode(WIFI_STA);
    WiFi.begin(AP_SSID, AP_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      DEBUG_PRINT(".");
      if (tryconnet--) break;
    }
    if (tryconnet > 0) {
      //DEBUG_PRINTLN("");
      DEBUG_PRINT("  WiFi connected  ] ");
      isconnected = true;
    } else {
      //DEBUG_PRINTLN("");
      DEBUG_PRINT(" WiFi notconnected, limit of try ] ");
      isconnected = false;
    }
  }
}

void sendTeperature(float temp, int id)
{
  wifiConnect();
  WiFiClient client;

  while (!client.connect(EIOT_CLOUD_ADDRESS, EIOT_CLOUD_PORT)) {
    DEBUG_PRINTLN("connection failed");
    wifiConnect();
  }

  String url = "";
  // URL: /RestApi/SetParameter/[instance id]/[parameter id]/[value]
  url += "/RestApi/SetParameter/";
  switch (id) {
    default:
    case 1:  url += String(EIOT_CLOUD_TEMP_INSTANCE_PARAM_ID_1); // generate EasIoT cloud update parameter URL 1
      break;
    case 2:  url += String(EIOT_CLOUD_TEMP_INSTANCE_PARAM_ID_2); // generate EasIoT cloud update parameter URL 2
      break;
  }
  url += "/" + String(temp); // generate EasIoT cloud update parameter URL

#if (DEBUG_SERVER)
  DEBUG_PRINT("POST data to URL: ");
  DEBUG_PRINTLN(url);
#endif
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + String(EIOT_CLOUD_ADDRESS) + "\r\n" +
               "Connection: close\r\n" +
               "Content-Length: 0\r\n" +
               "\r\n");

  delay(100);
  while (client.available()) {
    String line = client.readStringUntil('\r');
#if (DEBUG_SERVER)
    DEBUG_PRINT(line);
#endif
  }
#if (DEBUG_SERVER)
  DEBUG_PRINTLN();
  DEBUG_PRINTLN("Connection closed");
#endif
}

void sendHumidity(float hum, int id)
{
  wifiConnect();
  WiFiClient client;

  while (!client.connect(EIOT_CLOUD_ADDRESS, EIOT_CLOUD_PORT)) {
    DEBUG_PRINTLN("connection failed");
    wifiConnect();
  }

  String url = "";
  // URL: /RestApi/SetParameter/[instance id]/[parameter id]/[value]
  url += "/RestApi/SetParameter/";
  switch (id) {
    default:
    case 1:  url += String(EIOT_CLOUD_HUM_INSTANCE_PARAM_ID_1); // generate EasIoT cloud update parameter URL2
      break;
    case 2:  url += String(EIOT_CLOUD_HUM_INSTANCE_PARAM_ID_2); // generate EasIoT cloud update parameter URL2
      break;
  }
  url += "/" + String(hum); // generate EasIoT cloud update parameter URL
#if (DEBUG_SERVER)
  DEBUG_PRINT("POST data to URL: ");
  DEBUG_PRINTLN(url);
#endif
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + String(EIOT_CLOUD_ADDRESS) + "\r\n" +
               "Connection: close\r\n" +
               "Content-Length: 0\r\n" +
               "\r\n");

  delay(100);
  while (client.available()) {
    String line = client.readStringUntil('\r');
#if (DEBUG_SERVER)
    DEBUG_PRINT(line);
#endif
  }
#if (DEBUG_SERVER)
  DEBUG_PRINTLN();
  DEBUG_PRINTLN("Connection closed");
#endif
}

#if USEDSLEEP
void loop() {
}
#endif
