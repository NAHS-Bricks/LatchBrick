#ifndef BRICK_SETUP_H
#define BRICK_SETUP_H

#include "global.h"

class brickSetup {
    
    public:
      brickSetup();
      void enter();

    private:
      String readLine();
      void showMenu();
      void showBrickInfo();
      void showConfig();
      void saveConfig();
      void configureWifi();
      void testWifi();
      void configureBrickServer();
      void connectBrickServer();
      void calibrateADC();
      void inspectRuntimeData();
      void showLatchInfo();
};

#endif // BRICK_SETUP_H
