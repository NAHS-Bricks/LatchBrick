#include "global.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "configData.h"
#include "runtimeData.h"
#include "coic.h"
#include "brickSetup.h"
#include <ArduinoJson.h>


void setup() {
  pinMode(SETUP_PIN, INPUT_PULLUP);
  delay(1);
  if (digitalRead(SETUP_PIN) == LOW) {
    brickSetup *bs = new brickSetup();
    bs->enter();
  }


  //----
  // Debugging
  Serial.begin(115200);
  Serial.println();


  //------------------------------------------
  //Here the normal Operation begins...
  cfgdat = new configData();
  //WiFi.forceSleepWake();  // Enable WiFi Radio
  //delay(1);
  WiFi.begin(cfgdat->wifissid.c_str(), cfgdat->wifipass.c_str());  // Connecting to WiFi can be done in background as it consumes a lot of time
  rundat = new runtimeData();
  latches = new coic();

  Serial.print("Conversion State: ");
  Serial.println(latches->conversion_state());
  Serial.println();
  Serial.println("Start Conversion");
  Serial.println();
  latches->start_conversion();  // can also be done in background as it consumes some time
  Serial.print("Conversion State: ");
  Serial.println(latches->conversion_state());
  Serial.println();


  //------------------------------------------
  // prepare json document to transmit
  DynamicJsonDocument json(1024);
  JsonArray l_array = json.createNestedArray("l");
  JsonArray y_array = json.createNestedArray("y");


  //------------------------------------------
  // collect charging state
  pinMode(BAT_CHRG_PIN, INPUT_PULLUP);
  pinMode(BAT_STDBY_PIN, INPUT_PULLUP);
  if (digitalRead(BAT_CHRG_PIN) == LOW) y_array.add("c");
  if (digitalRead(BAT_STDBY_PIN) == LOW) y_array.add("s");


  //------------------------------------------
  // read bat-voltage (if requested)
  if (rundat->vars.batVoltageRequested) {
    json["b"] = readBatVoltage();
    rundat->vars.batVoltageRequested = false;
  }


  //------------------------------------------
  // deliver version (if requested)
  if (rundat->vars.versionRequested) {
    JsonArray v_array = json.createNestedArray("v");
    for (uint8_t i = 0; i < NUMBER_OF_VERSIONS; i++) {
      JsonArray version_array = v_array.createNestedArray();
      version_array.add(version_names[i]);
      version_array.add(version_numbers[i]);
    }
    rundat->vars.versionRequested = false;
  }


  //------------------------------------------
  // deliver brickType (if requested)
  if (rundat->vars.brickTypeRequested) {
    json["x"] = BRICK_TYPE;
    rundat->vars.brickTypeRequested = false;
  }


  //------------------------------------------
  // deliver features (if requested)
  if (rundat->vars.featuresRequested) {
    JsonArray f_array = json.createNestedArray("f");
    for (uint8_t i = 0; i < NUMBER_OF_FEATURES; i++) {
      f_array.add(features[i]);
    }
    rundat->vars.featuresRequested = false;
  }


  //------------------------------------------
  // pull latch-states from coic
  Serial.println("Wait for conversion");
  Serial.print("Conversion State: ");
  Serial.println(latches->conversion_state());
  while(!latches->ready_to_send_states()) delay(10);  // wait for the conversion to be completed
  Serial.println("Conversion completed");
  latches->get_states();
  for (uint8_t latch = 0; latch < latches->latch_count; latch++) {
    l_array.add(latches->latch_state[latch]);
  }
  Serial.println("States Fetched");
  Serial.println();


  //------------------------------------------
  // wait for wifi and submit data
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
  }

  String httpPayload;
  serializeJson(json, httpPayload);
  WiFiClient client;
  HTTPClient http;
  http.begin(client, cfgdat->url);
  http.addHeader("Content-Type", "application/json");
  http.POST(httpPayload);
  DynamicJsonDocument feedback(1024);
  deserializeJson(feedback, http.getStream());
  http.end();
  // Switch WiFi Radio completely off, as it is not required anymore and just consumes power
  //WiFi.mode(WIFI_OFF);
  //WiFi.forceSleepBegin();
  //delay(1);


  //------------------------------------------
  // process feedback from BrickServer
  if (feedback.containsKey("d")) rundat->vars.deepSleepDelay = feedback["d"].as<uint16_t>();
  if (feedback.containsKey("t")) {
    JsonArray t_array = feedback["t"].as<JsonArray>();
    if (t_array.size() == latches->latch_count) {
      latches->clear_all_triggers();
      for (uint8_t latch = 0; latch < latches->latch_count; ++latch) {
        for (JsonVariant trigger : t_array[latch].as<JsonArray>()) {
          latches->set_trigger(latch, trigger.as<uint8_t>());
        }
      }
    }
  }
  if (feedback.containsKey("r")) {
    for (JsonVariant value : feedback["r"].as<JsonArray>()) {
      switch(value.as<uint8_t>()) {
        case 1:
          rundat->vars.versionRequested = true;
          break;
        case 2:
          rundat->vars.featuresRequested = true;
          break;
        case 3:
          rundat->vars.batVoltageRequested = true;
          break;
        case 5:
          rundat->vars.brickTypeRequested = true;
          break;
      }
    }
  }


  //------------------------------------------
  // write runtimeData
  rundat->write();


  //------------------------------------------
  // inform coic that all data processing is done
  Serial.print("Conversion State: ");
  Serial.println(latches->conversion_state());
  Serial.println();
  Serial.println("Conversion Stop");
  Serial.println();
  Serial.flush();
  latches->stop_conversion();
  Serial.print("Conversion State: ");
  Serial.println(latches->conversion_state());
  Serial.println();


  //------------------------------------------
  // go to deepsleep
  Serial.println("Go Sleep");
  Serial.println();
  Serial.flush();
  //ESP.deepSleep(rundat->vars.deepSleepDelay * 1e6, WAKE_RF_DISABLED);
  ESP.deepSleep(rundat->vars.deepSleepDelay * 1e6);
  //delay(rundat->vars.deepSleepDelay * 1000);
  //ESP.deepSleep(1000);
}

void loop() {
  delay(1000);
}
