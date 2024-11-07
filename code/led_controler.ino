/*
Reference code:
https://esp32io.com/tutorials/esp32-neopixel-led-strip
https://docs.sunfounder.com/projects/umsk/en/latest/03_esp32/esp32_lesson09_joystick.html

Menu guide:
https://www.youtube.com/watch?v=HVHVkKt-ldc
*/

#include <Adafruit_NeoPixel.h>
#include <TFT_eSPI.h>
#include <SPI.h>

// Pin definitions
#define PIN_NEO_PIXEL 17
#define NUM_PIXELS 8
#define xPin 12
#define yPin 13
#define swPin 15
#define PMETER 2          // Potentiometer (range: 0 - 4095)
#define BUTTON_PIN 38

// Initialize NeoPixel
Adafruit_NeoPixel NeoPixel(NUM_PIXELS, PIN_NEO_PIXEL, NEO_GRB + NEO_KHZ800);

// Initialize TFT display
TFT_eSPI tft = TFT_eSPI();

// Menu items
const char* mainMenu[] = {"Static", "Breathing", "Color Change"};
const int mainMenuLength = sizeof(mainMenu) / sizeof(mainMenu[0]);

const char* staticColors[] = {"Green", "Blue", "Red", "Orange"};
const int staticColorsLength = sizeof(staticColors) / sizeof(staticColors[0]);

int currentMenu = 0;     // 0: Main menu, 1: Static color selection
int currentOption = 0;   // Index of the current option in the menu

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;  // milliseconds

// Mode variables
bool menuActive = false;
int currentMode = 0;  // 0: None, 1: Static, 2: Breathing, 3: Color Change

// Breathing mode variables
int breathingBrightness = 0;
int breathingDirection = 1;  // 1: increasing, -1: decreasing
unsigned long lastBreathingTime = 0;
unsigned int breathingInterval = 5;  // Adjust speed as needed

// Color Change mode variables
unsigned int colorChangeHue = 0;
unsigned long lastColorChangeTime = 0;
unsigned int colorChangeInterval = 10;  // Adjust speed as needed

void setup() {
  Serial.begin(9600);
  NeoPixel.begin();
  pinMode(swPin, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize TFT display
  tft.init();
  tft.setRotation(1);  // Adjust rotation as needed
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  // White text with black background
}

void loop() {
  // Serial.print("Potentiometer: ");
  // Serial.println(analogRead(PMETER));
  // Serial.print("Button:");
  // Serial.println(digitalRead(BUTTON_PIN));

  // Read joystick button
  int buttonState = digitalRead(BUTTON_PIN);

  // Handle button press to toggle menu
  static int lastButtonState = HIGH;
  if (buttonState != lastButtonState) {
    if (buttonState == LOW) {
      // Button pressed
      menuActive = !menuActive;
      if (menuActive) {
        // Reset menu navigation variables
        currentMenu = 0;
        currentOption = 0;
        displayMenu();
      } else {
        tft.fillScreen(TFT_BLACK);
      }
      delay(200);  // Debounce delay
    }
    lastButtonState = buttonState;
  }

  if (menuActive) {
    // Handle menu navigation and selection
    handleMenu();
  } else {
    // Run current mode
    switch (currentMode) {
      case 1:
        // Static Color Mode (nothing to do here)
        break;
      case 2:
        runBreathingModeStep();
        break;
      case 3:
        runColorChangeModeStep();
        break;
      default:
        // No mode selected
        break;
    }
  }
}

void handleMenu() {
  static unsigned long lastNavTime = 0;
  unsigned long currentTime = millis();

  // Read joystick values
  int yValue = analogRead(yPin);

  if (currentTime - lastNavTime > debounceDelay) {
    if (yValue < 1000) {
      // Joystick moved up
      currentOption--;
      if (currentOption < 0) {
        if (currentMenu == 0) {
          currentOption = mainMenuLength - 1;
        } else if (currentMenu == 1) {
          currentOption = staticColorsLength - 1;
        }
      }
      lastNavTime = currentTime;
      displayMenu();
    } else if (yValue > 3000) {
      // Joystick moved down
      currentOption++;
      if (currentMenu == 0) {
        if (currentOption >= mainMenuLength) {
          currentOption = 0;
        }
      } else if (currentMenu == 1) {
        if (currentOption >= staticColorsLength) {
          currentOption = 0;
        }
      }
      lastNavTime = currentTime;
      displayMenu();
    }
  }

  // Handle selection
  int buttonState = digitalRead(BUTTON_PIN);
  static int lastButtonState = HIGH;
  if (buttonState != lastButtonState) {
    if (buttonState == LOW) {
      // Button pressed
      if (currentMenu == 0) {
        if (currentOption == 0) {
          // Static selected
          currentMenu = 1;
          currentOption = 0;
          displayMenu();
        } else if (currentOption == 1) {
          // Breathing mode
          currentMode = 2;
          menuActive = false;
          tft.fillScreen(TFT_BLACK);
          // Reset breathing mode variables
          breathingBrightness = 0;
          breathingDirection = 1;
        } else if (currentOption == 2) {
          // Color Change mode
          currentMode = 3;
          menuActive = false;
          tft.fillScreen(TFT_BLACK);
          // Reset color change mode variables
          colorChangeHue = 0;
        }
      } else if (currentMenu == 1) {
        // Static color selected
        runStaticColor(staticColors[currentOption]);
        currentMode = 1;
        menuActive = false;
        tft.fillScreen(TFT_BLACK);
      }
      delay(200);  // Debounce delay
    }
    lastButtonState = buttonState;
  }
}

void displayMenu() {
  tft.fillScreen(TFT_BLACK);

  if (currentMenu == 0) {
    // Display main menu
    for (int i = 0; i < mainMenuLength; i++) {
      if (i == currentOption) {
        // Highlight the current option
        tft.setTextColor(TFT_BLACK, TFT_WHITE);
      } else {
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
      }
      tft.setCursor(0, i * 30);  // Adjust position as needed
      tft.println(mainMenu[i]);
    }
  } else if (currentMenu == 1) {
    // Display static color menu
    for (int i = 0; i < staticColorsLength; i++) {
      if (i == currentOption) {
        tft.setTextColor(TFT_BLACK, TFT_WHITE);
      } else {
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
      }
      tft.setCursor(0, i * 30);
      tft.println(staticColors[i]);
    }
  }
}

void runStaticColor(const char* color) {
  // Set NeoPixel strip to the selected color
  uint32_t c;
  if (strcmp(color, "Green") == 0) {
    c = NeoPixel.Color(0, 255, 0);
  } else if (strcmp(color, "Blue") == 0) {
    c = NeoPixel.Color(0, 0, 255);
  } else if (strcmp(color, "Red") == 0) {
    c = NeoPixel.Color(255, 0, 0);
  } else if (strcmp(color, "Orange") == 0) {
    c = NeoPixel.Color(255, 165, 0);
  } else {
    c = NeoPixel.Color(255, 255, 255);  // Default to white
  }

  // Set all pixels to color
  for (int i = 0; i < NUM_PIXELS; i++) {
    NeoPixel.setPixelColor(i, c);
  }
  NeoPixel.show();
}

void runBreathingModeStep() {
  unsigned long currentTime = millis();
  if (currentTime - lastBreathingTime >= breathingInterval) {
    lastBreathingTime = currentTime;
    breathingBrightness += breathingDirection;
    if (breathingBrightness <= 0 || breathingBrightness >= 255) {
      breathingDirection = -breathingDirection;
    }
    uint32_t c = NeoPixel.Color(breathingBrightness, breathingBrightness, breathingBrightness);
    for (int i = 0; i < NUM_PIXELS; i++) {
      NeoPixel.setPixelColor(i, c);
    }
    NeoPixel.show();
  }
}

void runColorChangeModeStep() {
  unsigned long currentTime = millis();
  if (currentTime - lastColorChangeTime >= colorChangeInterval) {
    lastColorChangeTime = currentTime;
    colorChangeHue += 256;  // Adjust hue increment as needed
    if (colorChangeHue >= 65536) {
      colorChangeHue = 0;
    }
    uint32_t c = NeoPixel.ColorHSV(colorChangeHue);
    for (int i = 0; i < NUM_PIXELS; i++) {
      NeoPixel.setPixelColor(i, c);
    }
    NeoPixel.show();
  }
}
