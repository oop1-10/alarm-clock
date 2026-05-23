#include <Arduino_LED_Matrix.h>
#include <Modulino.h>

#include "numbers_frames.h"

Arduino_LED_Matrix matrix;
ModulinoKnob knob;

uint8_t frame[8][13]; // Changed back to 13 to fix the diagonal pitch skew

unsigned long last_second_millis = 0;
int hours = 12;
int minutes = 0;
int seconds = 0;

enum State { DEFAULT, EDIT_HOUR, EDIT_MINUTE };
State currentState = DEFAULT;

int last_knob_val = 0;
bool last_knob_pressed = false;

// Custom 2x5 font for leading '2' to save space
const uint8_t font2x5_2[5][2] = {
  {1, 1},
  {0, 1},
  {1, 1},
  {1, 0},
  {1, 1}
};

void drawDigit3x5(int digit, int startCol, int startRow) {
  for (int r = 0; r < 5; r++) {
    for (int c = 0; c < 3; c++) {
      if (startRow + r < 8 && startCol + c < 13) {
        frame[startRow + r][startCol + c] = font3x5[digit][r][c];
      }
    }
  }
}

void clearFrame() {
  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 13; c++) {
      frame[r][c] = 0;
    }
  }
}

void setup() {
  matrix.begin();
  Modulino.begin();
  knob.begin();
  
  last_knob_val = knob.get();
  last_second_millis = millis();
}

void loop() {
  unsigned long current_millis = millis();
  
  // 1. Update internal clock
  if (current_millis - last_second_millis >= 1000) {
    last_second_millis += 1000;
    seconds++;
    if (seconds >= 60) {
      seconds = 0;
      minutes++;
      if (minutes >= 60) {
        minutes = 0;
        hours++;
        if (hours >= 24) {
          hours = 0;
        }
      }
    }
  }
  
  // 2. Handle button presses to change state
  bool current_pressed = knob.isPressed();
  if (current_pressed && !last_knob_pressed) {
    if (currentState == DEFAULT) {
      currentState = EDIT_HOUR;
    } else if (currentState == EDIT_HOUR) {
      currentState = EDIT_MINUTE;
    } else {
      currentState = DEFAULT;
    }
  }
  last_knob_pressed = current_pressed;
  
  // 3. Handle knob rotations
  int current_knob_val = knob.get();
  int diff = current_knob_val - last_knob_val;
  
  if (diff != 0) {
    if (currentState == EDIT_HOUR) {
      hours = (hours + diff) % 24;
      if (hours < 0) hours += 24; 
    } else if (currentState == EDIT_MINUTE) {
      minutes = (minutes + diff) % 60;
      if (minutes < 0) minutes += 60;
      seconds = 0; 
    }
    last_knob_val = current_knob_val;
  }
  
  // 4. Draw to frame buffer
  clearFrame();
  
  bool showH = true;
  bool showM = true;
  
  // Blink at 1Hz (500ms on, 500ms off) when editing
  if (currentState == EDIT_HOUR && (current_millis / 500) % 2 == 0) {
    showH = false;
  }
  if (currentState == EDIT_MINUTE && (current_millis / 500) % 2 == 0) {
    showM = false;
  }
  
  // Vertically center the 5-pixel high text on the 8-pixel high screen
  int row_offset = 1; 
  
  // --- Draw Hours (Columns 0 to 4) ---
  if (showH) {
    int h1 = hours / 10;
    int h2 = hours % 10;
    
    if (h1 == 1) {
      // Draw a 1-pixel wide '1' at col 1
      for (int r = 0; r < 5; r++) frame[row_offset + r][1] = 1;
    } else if (h1 == 2) {
      // Draw 2x5 '2' at cols 0, 1
      for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 2; c++) {
          frame[row_offset + r][0 + c] = font2x5_2[r][c];
        }
      }
    }
    // If h1 == 0, cols 0 and 1 are left blank!
    
    // Draw ones of hours at cols 2,3,4
    drawDigit3x5(h2, 2, row_offset);
  }
  
  // --- Draw Minutes (Columns 7 to 12) ---
  if (showM) {
    int m1 = minutes / 10;
    int m2 = minutes % 10;
    // Tens of minutes at cols 7,8,9
    drawDigit3x5(m1, 7, row_offset);
    // Ones of minutes at cols 10,11,12
    drawDigit3x5(m2, 10, row_offset);
  }
  
  // --- Draw Colon (Column 6) ---
  if (currentState == DEFAULT) {
    // Blink colon every second in default mode
    if (seconds % 2 == 0) {
      frame[row_offset + 1][6] = 1;
      frame[row_offset + 3][6] = 1;
    }
  } else {
    // Keep colon solid while editing
    frame[row_offset + 1][6] = 1;
    frame[row_offset + 3][6] = 1;
  }
  
  // 5. Render frame (passing 13 to match the matrix expected pitch)
  matrix.renderBitmap(frame, 8, 13);
  
  delay(10);
}
