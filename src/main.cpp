#include <RGBmatrixPanel.h>
#include <stdint.h>

#define CLK  8  
#define OE   9
#define LAT A3
#define A   A0
#define B   A1
#define C   A2

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

#define DEBUG 1

uint8_t dataPoints[30] = {0};
#define INITIAL_DP -1

void setup() {
  // Serial.begin(115200);
  matrix.begin();

  // fix the screen with black
  matrix.fillRect(0, 0, 32, 16, matrix.Color333(0, 0, 0));

  // draw a box in green
  matrix.drawRect(0, 0, 32, 16, matrix.Color333(0, 4, 0));

  // draw extra horiz lines in light green to help see time.
  for (int i = 0; i < 32; i += 8) {
    matrix.drawLine(i, 0, i+3, 0, matrix.Color333(2, 4, 1));
    matrix.drawLine(i+4, 15, i+3+4, 15, matrix.Color333(2, 4, 1));
  }

  // draw extra vert lines in red to help see time.
  for (int i = 1; i < 13; i+=4) {
    uint8_t length = 2;
    if (i == 5) {
      length = 3;
    }
    matrix.drawLine(0, i, 0, i+length, matrix.Color333(3, 1, 3));
    matrix.drawLine(31, i, 31, i+length, matrix.Color333(3, 1, 3));
    if (i == 5) {
      i++;
    }
  }

  #ifdef DEBUG
  // draw grey divider
  matrix.drawLine(1, 13, 30, 13, matrix.Color333(1, 1, 1)); 

  #endif

  // initialize the data points as -1
  for (int i = 0; i < 30; i++) {
    dataPoints[i] = INITIAL_DP;
  }
  randomSeed(analogRead(A4));

}

uint8_t getTriangleWave() {
  // Static variable to keep track of the current value between function calls
  static uint8_t currentValue = 0;
  // Static variable to track direction (0 = ascending, 1 = descending)
  static bool descending = false;
  
  // Store the current value to return
  uint8_t valueToReturn = currentValue;
  
  // Update the value for the next call
  if (!descending) {
    // Moving up
    currentValue++;
    // If we've reached the top value, start descending
    if (currentValue >= 11) {
      descending = true;
    }
  } else {
    // Moving down
    currentValue--;
    // If we've reached the bottom value, start ascending
    if (currentValue <= 0) {
      descending = false;
    }
  }
  
  return valueToReturn;
}

uint8_t getSineWave() {
  // Static variable to keep track of the angle (in degrees)
  static uint16_t angle = 0;
  
  // Calculate sine value (-1.0 to 1.0) and map to range 0-11
  // sin() works with radians, so convert degrees to radians
  float sinValue = sin(angle * PI / 180.0);
  uint8_t mappedValue = (uint8_t)round(sinValue * 11.0/2 + 11.0/2);
  
  // Increment angle for next call (0 to 359 degrees)
  angle = (angle + 15) % 360;
  
  return mappedValue;
}

// TODO: Shift the data points within the display loop
void loop() {
  // right shift the data points
  for (int i = 29; i > 0; i--) {
    dataPoints[i] = dataPoints[i - 1];
  }
  dataPoints[0] = getTriangleWave();
  // dataPoints[0] = getSineWave();

  // reset oscilloscope display
  matrix.fillRect(1, 1, 30, 12, matrix.Color333(0, 0, 0));

  // draw the data points
  for (int i = 0; i < 30; i++) {
    int x = i + 1;
    int y = 12 - dataPoints[i];
    if (dataPoints[i] != INITIAL_DP) {
      matrix.drawPixel(x, y, matrix.Color333(0, 0, 4));
    }
  }

  delay(5);
}
