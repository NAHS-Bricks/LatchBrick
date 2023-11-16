#include <Arduino.h>
#include <nahs-Bricks-OS.h>
// include all features of brick
#include <nahs-Bricks-Feature-Sleep.h>
#include <nahs-Bricks-Feature-Bat.h>
#include <nahs-Bricks-Feature-Latch.h>

void setup() {
  // Now register all the features under All
  // Note: the order of registration is the same as the features are handled internally by FeatureAll
  FeatureAll.registerFeature(&FeatureBat);
  FeatureAll.registerFeature(&FeatureLatch);
  FeatureAll.registerFeature(&FeatureSleep);

  // Set Brick-Specific stuff
  BricksOS.setSetupPin(D5);
  FeatureAll.setBrickType(2);

  // Set Brick-Specific (feature related) stuff
  Wire.begin();
  Latch.begin(45);
  FeatureLatch.assignLatch(Latch);
  for (uint8_t i = 0; i < Latch.latchCount(); ++i) {
    FeatureLatch.assignLatchPin(i);
  }
  FeatureBat.setPins(D6, D7, A0);

  // Finally hand over to BrickOS
  BricksOS.handover();
}

void loop() {
  // Not used on Bricks
}