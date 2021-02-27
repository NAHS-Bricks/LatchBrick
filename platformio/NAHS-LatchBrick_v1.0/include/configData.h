#ifndef CONFIG_DATA_H
#define CONFIG_DATA_H

#include "global.h"

class configData {
    
    public:
      configData();
      void save();
      void begin();

    public:
      String wifissid;
      String wifipass;
      String url;
      uint16_t adc5V = 99;
};

extern configData cfgdat;

#endif // CONFIG_DATA_H
