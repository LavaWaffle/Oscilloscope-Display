#include <RGBmatrixPanel.h>
#include <stdint.h>

#define DEBUG 1

#define INITIAL_DP 12

#define BIT4 13
#define BIT3 12
#define BIT2 11
#define BIT1 10
#define USR_CLK  A5
#define SINGLE_EN  A4

#define CLK  8  
#define OE   9
#define LAT A3
#define A   A0
#define B   A1
#define C   A2

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

uint8_t blueDataPoints[30] = {0};
uint8_t redDataPoints[30] = {0};

void setup() {
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
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

  // draw grey divider
  matrix.drawLine(1, 13, 30, 13, matrix.Color333(1, 1, 1)); 

  // initialize the data points as 12
  for (int i = 0; i < 30; i++) {
    blueDataPoints[i] = INITIAL_DP;
    redDataPoints[i] = INITIAL_DP;
  }

  pinMode(BIT4, INPUT_PULLUP);
  pinMode(BIT3, INPUT_PULLUP);
  pinMode(BIT2, INPUT_PULLUP);
  pinMode(BIT1, INPUT_PULLUP);
  pinMode(USR_CLK, INPUT);
  pinMode(SINGLE_EN, INPUT);

  #ifdef DEBUG
  Serial.begin(115200);
  #endif
}

bool prevClk = false;
bool blueCycle = true;
bool prevSingleEn = false;
void loop() {
  bool bit4 = digitalRead(BIT4);
  bool bit3 = digitalRead(BIT3);
  bool bit2 = digitalRead(BIT2);
  bool bit1 = digitalRead(BIT1);

  matrix.drawPixel(1, 14, bit4 ? matrix.Color333(0, 0, 4) : matrix.Color333(0, 0, 0));
  matrix.drawPixel(2, 14, bit3 ? matrix.Color333(0, 0, 4) : matrix.Color333(0, 0, 0));
  matrix.drawPixel(3, 14, bit2 ? matrix.Color333(0, 0, 4) : matrix.Color333(0, 0, 0));
  matrix.drawPixel(4, 14, bit1 ? matrix.Color333(0, 0, 4) : matrix.Color333(0, 0, 0));

  int newValue = 8 * !bit4 + 4 * !bit3 + 2 * !bit2 + 1 * !bit1;

  bool clk = digitalRead(USR_CLK);

  matrix.drawPixel(30, 14, clk ? matrix.Color333(0, 4, 4) : matrix.Color333(0, 0, 0));

  bool single_en = digitalRead(SINGLE_EN);

  matrix.drawPixel(29, 14, single_en ? matrix.Color333(4, 4, 0) : matrix.Color333(4, 0, 0));

  if (clk == HIGH && prevClk == LOW) {
    prevClk = clk;
  } else {
    prevClk = clk;
    return;
  }

  if (single_en == LOW) {
    // dual channel mode : switch blue and red data points
    blueCycle = !blueCycle;
  } else {
    // single channel mode : blue data points only
    blueCycle = true;
  }

  if (single_en != prevSingleEn) {
    // switched modes: Reset Data Points
    for (int i = 0; i < 30; i++) {
      blueDataPoints[i] = INITIAL_DP;
      redDataPoints[i] = INITIAL_DP;
    }
  }

  prevSingleEn = single_en;

  #ifdef DEBUG
  Serial.print("newValue: ");
  Serial.print(newValue);
  Serial.print(" isBlueOn: ");
  Serial.println(blueCycle);
  #endif

  // right shift the data points
  for (int i = 29; i > 0; i--) {
    if (blueCycle) {
      blueDataPoints[i] = blueDataPoints[i - 1];
    } else {
      redDataPoints[i] = redDataPoints[i - 1];
    }
  }

  if (blueCycle) {
    blueDataPoints[0] = newValue;
  } else {
    redDataPoints[0] = newValue;
  }
  
  
  // reset oscilloscope display
  matrix.fillRect(1, 1, 30, 12, matrix.Color333(0, 0, 0));
  
  // draw all blue data points
  for (int i = 0; i < 30; i++) {
    int x = i + 1;
    int y = 12 - blueDataPoints[i];
    if (blueDataPoints[i] != INITIAL_DP) {
      matrix.drawPixel(x, y, matrix.Color333(0, 0, 4));
    }
  }
  
  if (single_en == LOW) {
    // draw all red data points only if on dual channel mode
    for (int i = 0; i < 30; i++) {
      int x = i + 1;
      int y = 12 - redDataPoints[i];
      if (redDataPoints[i] != INITIAL_DP) {
        matrix.drawPixel(x, y, matrix.Color333(4, 0, 0));
      }
    }
  }
}