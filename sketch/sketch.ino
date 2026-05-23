#include <Arduino_LED_Matrix.h>
#include <Modulino.h>

#include "numbers_frames.h"

Arduino_LED_Matrix matrix;
ModulinoKnob knob;

void setup() {
  // put your setup code here, to run once:
  matrix.begin();
  matrix.clear();

  Modulino.begin();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  
  matrix.renderBitmap(frame, 8, 13);
}
