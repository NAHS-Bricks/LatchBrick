#include <Arduino.h>
#include <nahs-Brick-OS.h>
// include all features of brick
#include <nahs-Brick-Feature-Sleep.h>
#include <nahs-Brick-Feature-Bat.h>
#include <nahs-Brick-Feature-Latch.h>

void setup() {
  // Now register all the features under All
  // Note: the order of registration is the same as the features are handled internally by FeatureAll
  FeatureAll.registerFeature(&FeatureBat);
  FeatureAll.registerFeature(&FeatureLatch);
  FeatureAll.registerFeature(&FeatureSleep);

  // Set Brick-Specific stuff
  BrickOS.setSetupPin(D5);
  FeatureAll.setBrickType(2);

  // Set Brick-Specific (feature related) stuff
  Wire.begin();
  Latch.begin(45);
  FeatureLatch.assignLatch(Latch);
  FeatureLatch.assignLatchPin(0);
  FeatureLatch.assignLatchPin(1);
  FeatureBat.setPins(D6, D7, A0);

  // Finally hand over to BrickOS
  BrickOS.handover();
}

void loop() {
  // Not used on Bricks
}