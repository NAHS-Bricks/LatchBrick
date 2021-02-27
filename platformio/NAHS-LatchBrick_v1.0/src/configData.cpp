#include "LittleFS.h"
#include "configData.h"
#include <ArduinoJson.h>

configData::configData() {
}

void configData::begin() {
  LittleFS.begin();
  File cfgData = LittleFS.open("/config.json", "r");

  if (cfgData) {
    size_t size = cfgData.size();
    if (size <= 1024) {
      DynamicJsonDocument json(1024);
      DeserializationError error = deserializeJson(json, cfgData);
      if (!error) {
        const byte maxChars = 32;
        char tmp[maxChars];

        if(json.containsKey("wifi-ssid")) {
          strlcpy(tmp, json["wifi-ssid"], maxChars);
          wifissid = String(tmp);
        } else wifissid = "";

        if(json.containsKey("wifi-pass")) {
          strlcpy(tmp, json["wifi-pass"], maxChars);
          wifipass = String(tmp);
        } else wifipass = "";

        if(json.containsKey("url")) {
          strlcpy(tmp, json["url"], maxChars);
          url = String(tmp);
        } else url = "";

        if(json.containsKey("adc5V")) {
          adc5V = (uint16_t) json["adc5V"];
        } else adc5V = 0xFFFF;
      }
    }
    cfgData.close();
  }
}

void configData::save() {
	DynamicJsonDocument json(1024);
	json["wifi-ssid"] = wifissid.c_str();
	json["wifi-pass"] = wifipass.c_str();
	json["url"] = url.c_str();
  json["adc5V"] = adc5V;
	
	File cfgData = LittleFS.open("/config.json", "w");
	if (cfgData) {
		serializeJson(json, cfgData);
		cfgData.close();
	}
}

configData cfgdat;