#include <ESP8266WiFi.h>
#include "brickSetup.h"
#include "runtimeData.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>


//------------------------------------------
// constructor
brickSetup::brickSetup() {
  Serial.begin(115200);
  cfgdat = new configData();
}


//------------------------------------------
// enter
void brickSetup::enter() {
  while(true) {
    Serial.print("\nSelect: ");
    long selection = readLine().toInt();
    switch(selection) {
      case 1:
        showBrickInfo();
        break;
      case 2:
        showConfig();
        break;
      case 3:
        saveConfig();
        break;
      case 4:
        configureWifi();
        break;
      case 5:
        testWifi();
        break;
      case 6:
        configureBrickServer();
        break;
      case 7:
        connectBrickServer();
        break;
      case 8:
        calibrateADC();
        break;
      case 9:
        inspectRuntimeData();
        break;
      default:
        showMenu();
    }
  }
}


//------------------------------------------
// readLine
String brickSetup::readLine() {
  const byte maxChars = 32;
  char inputBuffer[maxChars];
  byte nextPos = 0;
  
  while (true) {
    if (Serial.available() > 0) {
      char input = Serial.read();
      if (input == '\n') {
        inputBuffer[nextPos] = '\0';
        Serial.print('\n');
        return String(inputBuffer);
      }
      if (input == '\b') {
        if (nextPos > 0) {
          nextPos--;
          inputBuffer[nextPos] = '\b';
          Serial.print('\b');
        }
      }
      else if (input != '\r') {
        inputBuffer[nextPos] = input;
        nextPos++;
        if (nextPos >= maxChars) nextPos--;
        Serial.print(input);
      }
    }
  }
}


//------------------------------------------
// showMenu
void brickSetup::showMenu() {
  Serial.println(" 1) Show Brick Info");
  Serial.println(" 2) Show Config");
  Serial.println(" 3) Save Config");
  Serial.println(" 4) Configure WiFi");
  Serial.println(" 5) Test WiFi");
  Serial.println(" 6) Configure BrickServer");
  Serial.println(" 7) Connect to BrickServer");
  Serial.println(" 8) Calibrate ADC");
  Serial.println(" 9) Inspect RuntimeData");
}


//------------------------------------------
// showConfig
void brickSetup::showBrickInfo() {
  Serial.print("BrickType: ");
  Serial.println(BRICK_TYPE);
  Serial.println("Features:");
  for(uint8_t i = 0; i < NUMBER_OF_FEATURES; i++) {
    Serial.print("  ");
    Serial.println(features[i]);
  }
  Serial.println("Versions:");
  for(uint8_t i = 0; i < NUMBER_OF_VERSIONS; i++) {
    Serial.print("  ");
    Serial.print(version_names[i]);
    Serial.print(": v");
    Serial.println(version_numbers[i]);
  }
  Serial.println("\nNAHS-BrickOS created by NiJO's");
  Serial.println("For more information visit my website: nijos.de");
  Serial.print("Copyright (");
  Serial.print(OS_YEAR);
  Serial.println(") NiJO's");
}


//------------------------------------------
// showConfig
void brickSetup::showConfig() {
  Serial.print("SSID: ");
  Serial.println(cfgdat->wifissid);
  Serial.print("PASS: ");
  Serial.println(cfgdat->wifipass);
  Serial.print("URL: ");
  Serial.println(cfgdat->url);
  Serial.print("adc5V: ");
  Serial.println(cfgdat->adc5V);
}


//------------------------------------------
// saveConfig
void brickSetup::saveConfig() {
  Serial.print("Saving config...");
  cfgdat->save();
  Serial.println(" done");
}


//------------------------------------------
// configureWifi
void brickSetup::configureWifi() {
  Serial.print("SSID: ");
  cfgdat->wifissid = readLine();
  Serial.print("Pass: ");
  cfgdat->wifipass = readLine();
}


//------------------------------------------
// testWifi
void brickSetup::testWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi allready connected. Stopping it");
    WiFi.mode(WIFI_OFF);
    while (WiFi.status() == WL_CONNECTED) {
      Serial.print('.');
      delay(200);
    }
    Serial.println('.');
  }
  Serial.print("Connecting WiFi");
  WiFi.begin(cfgdat->wifissid.c_str(), cfgdat->wifipass.c_str());
 
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
    if (i++ >= 20) break;
  }
  if (WiFi.status() == WL_CONNECTED) Serial.println("Success");
  else {
    Serial.println("Failed");
    WiFi.mode(WIFI_OFF);
  }
}


//------------------------------------------
// configureBrickServer
void brickSetup::configureBrickServer() {
  cfgdat->url = "http://";
  Serial.print("Server: ");
  cfgdat->url += readLine() + ':';
  Serial.print("Port: ");
  cfgdat->url += readLine();
}


//------------------------------------------
// connectBrickServer
void brickSetup::connectBrickServer() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Setup WiFi first!");
    return;
  }
  Serial.print("Sending Initial Data... ");
  
  // prepare json document to transmit
  DynamicJsonDocument json(1024);
  JsonArray v_array = json.createNestedArray("v");
  for (uint8_t i = 0; i < NUMBER_OF_VERSIONS; i++) {
    JsonArray version_array = v_array.createNestedArray();
    version_array.add(version_names[i]);
    version_array.add(version_numbers[i]);
  }
  JsonArray f_array = json.createNestedArray("f");
  for(uint8_t i = 0; i < NUMBER_OF_FEATURES; i++) {
    f_array.add(features[i]);
  }

  // generate payload
  String httpPayload;
  serializeJson(json, httpPayload);

  // connect to brickserver and transmit data
  WiFiClient client;
  HTTPClient http;
  http.begin(client, cfgdat->url);
  http.addHeader("Content-Type", "application/json");
  http.POST(httpPayload);
  DynamicJsonDocument feedback(1024);
  deserializeJson(feedback, http.getStream());
  http.end();

  if(feedback.containsKey("s")) {
    if(feedback["s"] == 0) Serial.println("Success");
    else Serial.println("Failed");
  } else Serial.println("Failed");
}


//------------------------------------------
// calibrateADC
void brickSetup::calibrateADC() {
  Serial.print("Voltage on TPB- and TPB+: ");
  float tpb = readLine().toFloat();
  if (tpb <= 0) {
    Serial.println("Invalid value, breaking...");
    return;
  }
  Serial.println("Sampling ADC...");
  uint32_t avg = 0;
  for(uint8_t i = 0; i < 3; i++) {
    uint16_t reading = analogRead(BAT_ADC_PIN);
    Serial.println(reading);
    avg += reading;
    delay(20);
  }
  avg /= 3;
  Serial.print("AVG: ");
  Serial.println(avg);

  cfgdat->adc5V = (avg * 5.0) / tpb;
  Serial.print("Calculated value for 5.0 Volt: ");
  Serial.println(cfgdat->adc5V);
}


//------------------------------------------
// inspectRuntimeData
void brickSetup::inspectRuntimeData() {
  rundat = new runtimeData();
  //all
  Serial.print("memWritten1: ");
  Serial.println(rundat->vars.memWritten1);
  Serial.print("memWritten2: ");
  Serial.println(rundat->vars.memWritten2);
  
  //requests
  Serial.println("requests:");
  Serial.print("  brickType: ");
  Serial.println(rundat->vars.brickTypeRequested);
  Serial.print("  features: ");
  Serial.println(rundat->vars.featuresRequested);
  Serial.print("  version: ");
  Serial.println(rundat->vars.versionRequested);
  Serial.print("  batVoltage: ");
  Serial.println(rundat->vars.batVoltageRequested);
  
  //sleep
  Serial.println("sleep:");
  Serial.print("  deepSleepDelay: ");
  Serial.println(rundat->vars.deepSleepDelay);
  
  //bat
  Serial.println("bat:");
  Serial.print("  adc5V: ");
  Serial.println(rundat->vars.adc5V);
  
  //latch
}
