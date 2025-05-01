#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Rotary Encoder pins
const int CLK = 2;
const int DT = 3;
const int SW = 4;

// LED pins
const int LED_PINS[] = {5, 6, 7, 8};  // Connect LEDs to these pins
const int NUM_LEDS = 4;

// OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Grid variables
const int numRows = 2;
const int numCols = 2;

int selectedRow = 0;
int selectedCol = 0;

bool ledStates[NUM_LEDS] = {false, false, false, false};

int lastCLKState;
bool buttonPressed = false;

void setup() {
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(LED_PINS[i], OUTPUT);
  }

  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);

  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }
  
  display.clearDisplay();
  display.display();
  
  lastCLKState = digitalRead(CLK);
  updateDisplay();
}

void loop() {
  int currentCLKState = digitalRead(CLK);
  int buttonState = digitalRead(SW);

  // Rotary Encoder rotation detection
  if (currentCLKState != lastCLKState) {
    if (digitalRead(DT) != currentCLKState) {
      moveRight();
    } else {
      moveLeft();
    }
    updateDisplay();
    lastCLKState = currentCLKState;
    delay(150);  // Small delay for debounce
  }

  // Button press detection
  if (buttonState == LOW && !buttonPressed) {
    toggleLed();
    buttonPressed = true;
    delay(200);
  }

  if (buttonState == HIGH) {
    buttonPressed = false;
  }

  // Listen for serial commands from Python GUI
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd >= 'A' && cmd <= 'D') {
      int index = cmd - 'A';
      ledStates[index] = !ledStates[index];
      digitalWrite(LED_PINS[index], ledStates[index]);
      updateDisplay();
      sendLedStates();
    }
  }
}

void moveRight() {
  selectedCol++;
  if (selectedCol >= numCols) {
    selectedCol = 0;
    selectedRow++;
    if (selectedRow >= numRows) selectedRow = 0;
  }
}

void moveLeft() {
  selectedCol--;
  if (selectedCol < 0) {
    selectedCol = numCols - 1;
    selectedRow--;
    if (selectedRow < 0) selectedRow = numRows - 1;
  }
}

void toggleLed() {
  int index = selectedRow * numCols + selectedCol;
  ledStates[index] = !ledStates[index];
  digitalWrite(LED_PINS[index], ledStates[index]);
  sendLedStates();
  updateDisplay();
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  int boxWidth = SCREEN_WIDTH / numCols;
  int boxHeight = SCREEN_HEIGHT / numRows;

  for (int row = 0; row < numRows; row++) {
    for (int col = 0; col < numCols; col++) {
      int x = col * boxWidth;
      int y = row * boxHeight;

      int index = row * numCols + col;
      if (index >= NUM_LEDS) continue; // skip if more cells than LEDs

      // Draw border around selected cell
      if (row == selectedRow && col == selectedCol) {
        display.drawRect(x, y, boxWidth, boxHeight, WHITE);
      } else {
        display.drawRect(x, y, boxWidth, boxHeight, BLACK);
      }

      display.setCursor(x + 10, y + 10);
      display.print("LED");
      display.print(index + 1);
      display.print(":");
      display.print(ledStates[index] ? "ON" : "OFF");
    }
  }
  
  display.display();
}

void sendLedStates() {
  for (int i = 0; i < NUM_LEDS; i++) {
    Serial.print(ledStates[i] ? "1" : "0");
  }
  Serial.println();
}
