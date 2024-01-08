//Colab Leventiu Adrian-George
#include <LedControl.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <string.h>

const byte swDebounceDelay = 100;

// EEPROM MAP
// 0 - 3 - EASY1
// 4 - 7 - EASY2
// 8 - 11 - EASY3
// 12 - 15 - MEDIUM1
// 16 - 19 - MEDIUM2
// 20 - 23 - MEDIUM3
// 24 - 27 - HARD1
// 28 - 31 - HARD2
// 32 - 35 - HARD3
// 36 - 39 - N11
// 40 - 43 - N12
// 44 - 47 - N13
// 48 - 51 - N21
// 52 - 55 - N22
// 56 - 59 - N23
// 60 - 63 - N31
// 64 - 67 - N32
// 68 - 71 - N33

// 1022 - BrightnessMatrix
// 1023 - BrightnessLCD

const byte startScore = 0;
const byte floatSize = 4;
const byte noOfSavings = 3;
const byte startNicks = 36;

const int brightnessMatrix = 1022;
const int brightnessLCD = 1023;

const byte yPin = A1;
const byte xPin = A0;

const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;

const byte buttonPin = 13;

const byte matSize = 8;

const int BAUD = 9600;

const byte NoRooms = 4;
byte currentRoom = 0;

char matrix[NoRooms][matSize][matSize];

const byte NA = 0;
const byte EASY = 1;
const byte MEDIUM = 2;
const byte HARD = 3;

byte DIFFICULTY = 1;

const byte wallTypes = 2;
const char walls[wallTypes] = { ' ', '#' };

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);  // DIN, CLK, LOAD, No. DRIVER

byte matrixBrightness;

byte xPos = NA;
byte yPos = NA;
byte xLastPos = NA;
byte yLastPos = NA;

const int minThreshold = 200;
const int maxThreshold = 600;

byte playerRow;
byte playerCol;

int bombRow = -1;
int bombCol = -1;
int bombMat = -1;

const int userBlinkInterv = 500;
const byte bombBlinkingInterval[3] = { 50, 100, 200 };

unsigned long currentTime;
unsigned long startTime = millis();

unsigned long startBombingTime;
unsigned long bombPlacedTime;

unsigned long userStartTime;
unsigned long userWinTime;

bool blinkState = false;
bool bombBlinkState = false;

const unsigned int joystickSensitivity = 50;
bool joystickMoved = false;

unsigned int minimumThreshold = 350;
unsigned int maximumThreshold = 650;

const unsigned int debounceDelay = 50;
bool buttonPressed = false;

unsigned long buttonPressedTime;

const byte ASCII0 = 48;

byte start = 0;
bool won = false;
bool exitMenu = false;

//LCD
const byte rs = A4;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 3;
const byte d7 = 4;
const byte k = 5;
byte LCDBrightness;
const byte numRow = 2;
const byte numCol = 16;

unsigned long previousScrollTime = 0;
const unsigned long scrollInterval = 750;

const byte buzzerPin = 9;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

char user[3] = { 'A', 'A', 'A' };

bool sound = true;

bool secP = false;
bool userP = false;

void timeLCD(unsigned long time) {
  lcd.setCursor(0, 0);
  if (!userP) {
    lcd.clear();
    lcd.print("Name:");
    for (int i = NA; i < 3; i++) {
      lcd.print(user[i]);
    }
    userP = !userP;
    lcd.print(" ");
    if (DIFFICULTY == 1) {
      lcd.print("EASY");
    } else if (DIFFICULTY == 2) {
      lcd.print("MEDIUM");
    } else {
      lcd.print("HARD");
    }
  }

  lcd.setCursor(0, 1);
  lcd.print((time - userStartTime) / 1000);
  lcd.print(".");
  lcd.print(((time - userStartTime) % 1000) / 10);
  if (!secP) {
    lcd.print(F(" seconds"));
    secP = !secP;
  }
}

unsigned long getRandomSeed() {
  unsigned long seed = NA;
  seed = millis();
  for (int i = NA; i < matSize; ++i)
    seed = seed + analogRead(i);
  return seed;
}

void setup() {
  Serial.begin(BAUD);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  randomSeed(getRandomSeed());  // seed the random number generator

  lc.shutdown(NA, false);  // turn off power saving, enables display
  EEPROM.get(brightnessMatrix, matrixBrightness);
  lc.setIntensity(NA, matrixBrightness);  // sets brightness (NONE~15 possible values)

  setStartingEndingPositions();

  printWaitingMat();

  lcd.begin(numCol, numRow);
  pinMode(k, OUTPUT);
  EEPROM.get(brightnessLCD, LCDBrightness);
  analogWrite(k, LCDBrightness);  // sets brightness
}

void printWaitingMat() {
  lc.clearDisplay(NA);  // clear screen
  byte waitingMat[matSize][matSize] = {
    { 1, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 1, 0, 0, 1, 0, 1 },
    { 1, 0, 1, 0, 0, 1, 0, 1 },
    { 1, 0, 1, 0, 0, 1, 0, 1 },
    { 1, 0, 0, 1, 1, 0, 0, 1 },
    { 1, 0, 0, 1, 1, 0, 0, 1 }
  };
  rotateMatCounterClockwise(waitingMat);


  for (int row = NA; row < matSize; row++) {
    for (int col = NA; col < matSize; col++) {
      lc.setLed(NA, row, col, waitingMat[row][col]);
    }
  }
}

void printupdown() {
  lc.clearDisplay(NA);  // clear screen
  byte IMAGE[matSize][matSize] = {
    { 0, 0, 0, 1, 1, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 1, 0, 0 },
    { 0, 1, 0, 0, 0, 0, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 0, 0, 0, 0, 1, 0 },
    { 0, 0, 1, 0, 0, 1, 0, 0 },
    { 0, 0, 0, 1, 1, 0, 0, 0 }
  };
  rotateMatCounterClockwise(IMAGE);


  for (int row = NA; row < matSize; row++) {
    for (int col = NA; col < matSize; col++) {
      lc.setLed(NA, row, col, IMAGE[row][col]);
    }
  }
}

void printup() {
  lc.clearDisplay(NA);  // clear screen
  byte IMAGE[matSize][matSize] = {
    { 0, 0, 0, 1, 1, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 1, 0, 0 },
    { 0, 1, 0, 0, 0, 0, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }
  };
  rotateMatCounterClockwise(IMAGE);


  for (int row = NA; row < matSize; row++) {
    for (int col = NA; col < matSize; col++) {
      lc.setLed(NA, row, col, IMAGE[row][col]);
    }
  }
}

void printdown() {
  lc.clearDisplay(NA);  // clear screen
  byte IMAGE[matSize][matSize] = {
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 0, 0, 0, 0, 1, 0 },
    { 0, 0, 1, 0, 0, 1, 0, 0 },
    { 0, 0, 0, 1, 1, 0, 0, 0 }
  };
  rotateMatCounterClockwise(IMAGE);


  for (int row = NA; row < matSize; row++) {
    for (int col = NA; col < matSize; col++) {
      lc.setLed(NA, row, col, IMAGE[row][col]);
    }
  }
}

const byte OptionsMenu = 7;
byte Option = 0;
const byte SettingsOptions = 6;
byte subOption = 0;
byte lastOption = -1;
bool MenuP = false;
bool SubMP = false;
bool LeafP = false;
bool inMenu = false;
bool inSub = false;
bool inLeaf = false;
void Menu() {
  if (inMenu && !inSub && !inLeaf) {
    if (!MenuP) {
      if (Option > NA && Option < OptionsMenu - 1)
        printupdown();
      switch (Option) {
        case 0:
          lcd.setCursor(0, 0);
          lcd.clear();
          lcd.print(F("||Main Menu||"));
          printdown();
          break;
        case 1:
          lcd.setCursor(0, 0);
          lcd.clear();
          lcd.print(F("||Difficulty||"));
          break;
        case 2:
          lcd.setCursor(0, 0);
          lcd.clear();
          lcd.print(F("||Start||"));
          break;
        case 3:
          lcd.setCursor(0, 0);
          lcd.clear();
          lcd.print(F("||Settings||"));
          break;
        case 4:
          lcd.setCursor(0, 0);
          lcd.clear();
          lcd.print(F("||Highscores||"));
          break;
        case 5:
          lcd.setCursor(0, 0);
          lcd.clear();
          lcd.print(F("||About us||"));
          break;
        case 6:
          lcd.setCursor(0, 0);
          lcd.clear();
          lcd.print(F("||How to Play||"));
          printup();
          break;
      }
      MenuP = true;
    }
    unsigned int xValue = analogRead(xPin);
    if (xValue + joystickSensitivity < minimumThreshold && !joystickMoved) {
      if (Option < OptionsMenu - 1) {
        joystickMoved = true;
        Option += 1;
        MenuP = false;
        if (sound)
          tone(buzzerPin, 500, 100);
      }
    } else if (xValue - joystickSensitivity > maximumThreshold && !joystickMoved) {
      if (Option) {
        joystickMoved = true;
        Option -= 1;
        MenuP = false;
        if (sound)
          tone(buzzerPin, 500, 100);
      }
    } else if (xValue - joystickSensitivity < maximumThreshold && xValue + joystickSensitivity > minimumThreshold) {
      joystickMoved = false;
      exitMenu = false;
    }

    byte buttonState = digitalRead(buttonPin);

    if (!buttonState) {
      if (!buttonPressed) {               // verifying if the button was pressed
        buttonPressedTime = currentTime;  // starting recording the time when the button was first pressed
        buttonPressed = true;             // saving the fact that it was pressed
      }
    } else {
      if (buttonPressed && currentTime - buttonPressedTime >= swDebounceDelay) {  // if it's not a long press, it will just !state of the LED
        inSub = true;
        inMenu = false;
      }
      buttonPressed = false;  // resetting the value for the next press
    }
  }
  if (inSub) {
    switch (Option) {
      case 1:
        selectDifficulty();
        break;
      case 2:
        if (DIFFICULTY != NA) {
          start = 1;
          userStartTime = currentTime;
        }
        break;
      case 3:
        settings();
        break;
      case 4:
        seeHighscores();
        break;
      case 5:
        aboutUs();
        break;
      case 6:
        howTo();
        break;
    }
  }
}

void printStartMat() {
  const byte IMAGE[8][8] = {
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 1, 1, 1, 0, 0 },
    { 0, 1, 0, 0, 0, 0, 1, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 1, 0, 0, 0, 0, 1, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 1, 0, 0, 0, 0, 1, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 0 }
  };
  rotateMatCounterClockwise(IMAGE);
  for (int row = NA; row < matSize; row++) {
    for (int col = NA; col < matSize; col++) {
      lc.setLed(NA, row, col, IMAGE[row][col]);
    }
  }
}

void printBrightMat() {
  const byte IMAGE[8][8] = {
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 }
  };
  rotateMatCounterClockwise(IMAGE);
  for (int row = NA; row < matSize; row++) {
    for (int col = NA; col < matSize; col++) {
      lc.setLed(NA, row, col, IMAGE[row][col]);
    }
  }
}

bool StartP = false;
bool inStart = true;
void Start() {
  printStartMat();
  if (!StartP) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("  Press Switch  "));
    lcd.setCursor(0, 1);
    lcd.print(F("  ||To Start||  "));
    StartP = true;
  }
  byte buttonState = digitalRead(buttonPin);

  if (!buttonState) {
    if (!buttonPressed) {               // verifying if the button was pressed
      buttonPressedTime = currentTime;  // starting recording the time when the button was first pressed
      buttonPressed = true;             // saving the fact that it was pressed
    }
  } else {
    if (buttonPressed && currentTime - buttonPressedTime >= swDebounceDelay) {  // if it's not a long press, it will just !state of the LED
      inMenu = true;
      inStart = false;
      if (sound)
        tone(buzzerPin, 1000, 100);
    }
    buttonPressed = false;  // resetting the value for the next press
  }
}

byte highI = 0;

void seeHighscores() {
  if (!SubMP) {
    SubMP = true;
    lcd.clear();
    char names[3][3];
    float highscore[noOfSavings];
    for (int i = NA; i < noOfSavings; i++) {
      for (int j = NA; j < 3; j++) {
        EEPROM.get(startNicks + (DIFFICULTY - 1) * 3 * noOfSavings + i * 3 + j, names[i][j]);
        if (names[i][j] == 0)
          names[i][j] = '-';
      }
      EEPROM.get((DIFFICULTY - 1) * floatSize * noOfSavings + i * floatSize, highscore[i]);
    }

    switch (highI) {
      case 0:
        lcd.setCursor(0, 0);
        lcd.print(F("1."));
        for (int j = NA; j < 3; j++)
          lcd.print(names[0][j]);
        lcd.print(F(" "));
        if (highscore[0] != 0)
          lcd.print(highscore[0]);
        else
          lcd.print(F("NA"));
        lcd.setCursor(0, 1);
        lcd.print(F("2."));
        for (int j = NA; j < 3; j++)
          lcd.print(names[1][j]);
        lcd.print(F(" "));
        if (highscore[1] != 0)
          lcd.print(highscore[1]);
        else
          lcd.print(F("NA"));
        break;
      case 1:
        lcd.setCursor(0, 0);
        lcd.print(F("2."));
        for (int j = NA; j < 3; j++)
          lcd.print(names[1][j]);
        lcd.print(F(" "));
        if (highscore[1] != 0)
          lcd.print(highscore[1]);
        else
          lcd.print(F("NA"));
        lcd.setCursor(0, 1);
        lcd.print(F("3."));
        for (int j = NA; j < 3; j++)
          lcd.print(names[2][j]);
        lcd.print(F(" "));
        if (highscore[2] != 0)
          lcd.print(highscore[2]);
        else
          lcd.print(F("NA"));
        break;
    }
  }
  unsigned int xValue = analogRead(xPin);
  if (xValue + joystickSensitivity < minimumThreshold && !joystickMoved) {
    if (highI < 1) {
      joystickMoved = true;
      highI += 1;
      SubMP = false;
    }
  } else if (xValue - joystickSensitivity > maximumThreshold && !joystickMoved) {
    if (highI) {
      joystickMoved = true;
      highI -= 1;
      SubMP = false;
    }
  } else if (xValue - joystickSensitivity < maximumThreshold && xValue + joystickSensitivity > minimumThreshold) {
    joystickMoved = false;
  }

  byte buttonState = digitalRead(buttonPin);

  if (!buttonState) {
    if (!buttonPressed) {               // verifying if the button was pressed
      buttonPressedTime = currentTime;  // starting recording the time when the button was first pressed
      buttonPressed = true;             // saving the fact that it was pressed
    }
  } else {
    if (buttonPressed && currentTime - buttonPressedTime >= swDebounceDelay) {  // if it's not a long press, it will just !state of the LED
      inSub = false;
      inMenu = true;
      MenuP = false;
      SubMP = false;
      exitMenu = false;
      highI = 0;
    }
    buttonPressed = false;  // resetting the value for the next press
  }
}

byte index = 0;
void aboutUs() {
  if (!SubMP) {
    SubMP = true;
    printInfo();
    lcd.clear();
    switch (index) {
      case 0:
        lcd.setCursor(0, 0);
        lcd.print(F("You are playing"));
        lcd.setCursor(0, 1);
        lcd.print(F("Bombing Master"));
        break;
      case 1:
        lcd.setCursor(0, 0);
        lcd.print(F("Bombing Master"));
        lcd.setCursor(0, 1);
        lcd.print(F("GitHub : Ionescu"));
        break;
      case 2:
        lcd.setCursor(0, 0);
        lcd.print(F("GitHub : Ionescu"));
        lcd.setCursor(0, 1);
        lcd.print(F("MihaiLeonard"));
        break;
    }
  }
  unsigned int xValue = analogRead(xPin);
  if (xValue + joystickSensitivity < minimumThreshold && !joystickMoved) {
    if (index < 2) {
      joystickMoved = true;
      index += 1;
      SubMP = false;
    }
  } else if (xValue - joystickSensitivity > maximumThreshold && !joystickMoved) {
    if (index) {
      joystickMoved = true;
      index -= 1;
      SubMP = false;
    }
  } else if (xValue - joystickSensitivity < maximumThreshold && xValue + joystickSensitivity > minimumThreshold) {
    joystickMoved = false;
  }

  byte buttonState = digitalRead(buttonPin);

  if (!buttonState) {
    if (!buttonPressed) {               // verifying if the button was pressed
      buttonPressedTime = currentTime;  // starting recording the time when the button was first pressed
      buttonPressed = true;             // saving the fact that it was pressed
    }
  } else {
    if (buttonPressed && currentTime - buttonPressedTime >= swDebounceDelay) {  // if it's not a long press, it will just !state of the LED
      inSub = false;
      inMenu = true;
      MenuP = false;
      SubMP = false;
      exitMenu = false;
      index = 0;
    }
    buttonPressed = false;  // resetting the value for the next press
  }
}

void howTo() {
  if (!SubMP) {
    printInfo();
    SubMP = true;
    lcd.clear();
    switch (index) {
      case 0:
        lcd.setCursor(0, 0);
        lcd.print(F("Use joystick"));
        lcd.setCursor(0, 1);
        lcd.print(F("to move"));
        break;
      case 1:
        lcd.setCursor(0, 0);
        lcd.print(F("to move"));
        lcd.setCursor(0, 1);
        lcd.print(F("up and down"));
        break;
      case 2:
        lcd.setCursor(0, 0);
        lcd.print(F("up and down"));
        lcd.setCursor(0, 1);
        lcd.print(F("left and right"));
        break;
      case 3:
        lcd.setCursor(0, 0);
        lcd.print(F("left and right"));
        lcd.setCursor(0, 1);
        lcd.print(F("press joystick"));
        break;
      case 4:
        lcd.setCursor(0, 0);
        lcd.print(F("press joystick"));
        lcd.setCursor(0, 1);
        lcd.print(F("to bomb every"));
        break;
      case 5:
        lcd.setCursor(0, 0);
        lcd.print(F("to bomb every"));
        lcd.setCursor(0, 1);
        lcd.print(F("wall"));
        break;
      case 6:
        lcd.setCursor(0, 0);
        lcd.print(F("wall"));
        lcd.setCursor(0, 1);
        lcd.print(F("faster = better"));
        break;
    }
  }
  unsigned int xValue = analogRead(xPin);
  if (xValue + joystickSensitivity < minimumThreshold && !joystickMoved) {
    if (index < 6) {
      joystickMoved = true;
      index += 1;
      SubMP = false;
    }
  } else if (xValue - joystickSensitivity > maximumThreshold && !joystickMoved) {
    if (index) {
      joystickMoved = true;
      index -= 1;
      SubMP = false;
    }
  } else if (xValue - joystickSensitivity < maximumThreshold && xValue + joystickSensitivity > minimumThreshold) {
    joystickMoved = false;
  }

  byte buttonState = digitalRead(buttonPin);

  if (!buttonState) {
    if (!buttonPressed) {               // verifying if the button was pressed
      buttonPressedTime = currentTime;  // starting recording the time when the button was first pressed
      buttonPressed = true;             // saving the fact that it was pressed
    }
  } else {
    if (buttonPressed && currentTime - buttonPressedTime >= swDebounceDelay) {  // if it's not a long press, it will just !state of the LED
      inSub = false;
      inMenu = true;
      MenuP = false;
      SubMP = false;
      exitMenu = false;
      index = 0;
    }
    buttonPressed = false;  // resetting the value for the next press
  }
}

byte all = true;
byte userI = 0;

void selectUsername() {
  if (inLeaf) {
    if (!LeafP) {
      LeafP = true;
      if (all) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(String(user[0]));
        lcd.setCursor(1, 0);
        lcd.print(String(user[1]));
        lcd.setCursor(2, 0);
        lcd.print(String(user[2]));
      } else {
        lcd.setCursor(0 + userI, 0);
        lcd.blink();
        lcd.print(user[userI]);
        lcd.setCursor(0 + userI, 0);
        lcd.blink();
      }
    }

    unsigned int yValue = analogRead(yPin);
    unsigned int xValue = analogRead(xPin);
    //Stg-drpt
    if (yValue + joystickSensitivity < minimumThreshold && !joystickMoved) {
      if (!joystickMoved) {
        joystickMoved = true;
        if (userI - 1 >= 0) {
          userI -= 1;
          all = false;
          LeafP = false;
        }
      }
    } else if (yValue - joystickSensitivity > maximumThreshold && !joystickMoved) {
      if (!joystickMoved) {
        joystickMoved = true;
        if (userI + 1 < 3) {
          userI += 1;
          all = false;
          LeafP = false;
        }
      }
    }
    // up down
    if (xValue + joystickSensitivity < minimumThreshold && !joystickMoved) {
      if (!joystickMoved) {
        joystickMoved = true;
        if (user[userI] - 1 >= 'A') {
          user[userI] -= 1;
          all = false;
          LeafP = false;
        } else {
          user[userI] = 'Z';
          all = false;
          LeafP = false;
        }
      }
    } else if (xValue - joystickSensitivity > maximumThreshold && !joystickMoved) {
      if (!joystickMoved) {
        joystickMoved = true;
        if (user[userI] + 1 <= 'Z') {
          user[userI] += 1;
          all = false;
          LeafP = false;
        } else {
          user[userI] = 'A';
          all = false;
          LeafP = false;
        }
      }
    }
    if ((xValue - joystickSensitivity < maximumThreshold && xValue + joystickSensitivity > minimumThreshold) && (yValue - joystickSensitivity < maximumThreshold && yValue + joystickSensitivity > minimumThreshold)) {
      joystickMoved = false;
    }

    byte buttonState = digitalRead(buttonPin);

    if (!buttonState) {
      if (!buttonPressed) {               // verifying if the button was pressed
        buttonPressedTime = currentTime;  // starting recording the time when the button was first pressed
        buttonPressed = true;             // saving the fact that it was pressed
      }
    } else {
      if (buttonPressed && currentTime - buttonPressedTime >= swDebounceDelay) {  // if it's not a long press, it will just !state of the LED
        inLeaf = false;
        LeafP = false;
        all = true;
        lcd.noBlink();
        exitMenu = false;
      }
      buttonPressed = false;  // resetting the value for the next press
    }
  }
}

void printSmile() {
  const byte IMAGE[8][8] = {
    { 0, 0, 1, 1, 1, 1, 0, 0 },
    { 0, 1, 0, 0, 0, 0, 1, 0 },
    { 1, 0, 1, 0, 0, 1, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 1, 0, 0, 1, 0, 1 },
    { 1, 0, 0, 1, 1, 0, 0, 1 },
    { 0, 1, 0, 0, 0, 0, 1, 0 },
    { 0, 0, 1, 1, 1, 1, 0, 0 }
  };
  rotateMatCounterClockwise(IMAGE);
  for (int row = NA; row < matSize; row++) {
    for (int col = NA; col < matSize; col++) {
      lc.setLed(NA, row, col, IMAGE[row][col]);
    }
  }
}

void printInfo() {
  const byte IMAGE[8][8] = {
    { 0, 0, 0, 1, 1, 0, 0, 0 },
    { 0, 0, 1, 1, 1, 1, 0, 0 },
    { 0, 0, 0, 1, 1, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 0, 0, 0 }
  };
  rotateMatCounterClockwise(IMAGE);
  for (int row = NA; row < matSize; row++) {
    for (int col = NA; col < matSize; col++) {
      lc.setLed(NA, row, col, IMAGE[row][col]);
    }
  }
}


void settings() {
  if (inSub && !inLeaf) {
    if (!SubMP) {
      lcd.clear();
      printSmile();
      SubMP = true;
      switch (subOption) {
        case 0:
          lcd.setCursor(0, 0);
          lcd.clear();
          lcd.print(F("||Back||"));
          break;
        case 1:
          lcd.setCursor(0, 0);
          lcd.clear();
          lcd.print(F("||Username||"));
          break;
        case 2:
          lcd.setCursor(0, 0);
          lcd.clear();
          lcd.print(F("||MatrixBright||"));
          break;
        case 3:
          lcd.setCursor(0, 0);
          lcd.clear();
          lcd.print(F("||LCDBright||"));
          break;
        case 4:
          lcd.setCursor(0, 0);
          lcd.clear();
          lcd.print(F("||Sound||"));
          lcd.setCursor(0, 1);
          if (sound == 1)
            lcd.print(F("ON"));
          else
            lcd.print(F("OFF"));
          break;
        case 5:
          lcd.setCursor(0, 0);
          lcd.clear();
          lcd.print(F("ClearHighscore"));
          break;
      }
    }
    unsigned int xValue = analogRead(xPin);
    if (xValue + joystickSensitivity < minimumThreshold && !joystickMoved) {
      if (subOption < SettingsOptions - 1) {
        joystickMoved = true;
        subOption += 1;
        SubMP = false;
        if (sound)
          tone(buzzerPin, 500, 100);
      }
    } else if (xValue - joystickSensitivity > maximumThreshold && !joystickMoved) {
      if (subOption) {
        joystickMoved = true;
        subOption -= 1;
        SubMP = false;
        if (sound)
          tone(buzzerPin, 500, 100);
      }
    } else if (xValue - joystickSensitivity < maximumThreshold && xValue + joystickSensitivity > minimumThreshold) {
      joystickMoved = false;
      exitMenu = false;
    }

    byte buttonState = digitalRead(buttonPin);

    if (!buttonState) {
      if (!buttonPressed) {               // verifying if the button was pressed
        buttonPressedTime = currentTime;  // starting recording the time when the button was first pressed
        buttonPressed = true;             // saving the fact that it was pressed
      }
    } else {
      if (buttonPressed && currentTime - buttonPressedTime >= swDebounceDelay) {  // if it's not a long press, it will just !state of the LED
        if (!subOption) {
          inSub = false;
          inMenu = true;
          MenuP = false;
          SubMP = false;
        } else {
          // inSub = false;
          SubMP = false;
          inLeaf = true;
        }
        exitMenu = false;
      }
      buttonPressed = false;  // resetting the value for the next press
    }
  }
  if (inLeaf) {
    switch (subOption) {
      // case 0:
      //   break;
      case 1:
        selectUsername();
        break;
      case 2:
        selectMatBright();
        break;
      case 3:
        selectLCDBright();
        break;
      case 4:
        selectSound();
        break;
      case 5:
        clearHighScores();
        break;
    }
  }
}

void selectSound() {
  sound = !sound;
  inLeaf = false;
  LeafP = false;
}

void clearHighScores() {
  if (!LeafP) {
    LeafP = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Click to clear");
  }

  byte buttonState = digitalRead(buttonPin);

  if (!buttonState) {
    if (!buttonPressed) {               // verifying if the button was pressed
      buttonPressedTime = currentTime;  // starting recording the time when the button was first pressed
      buttonPressed = true;             // saving the fact that it was pressed
    }
  } else {
    if (buttonPressed && currentTime - buttonPressedTime >= swDebounceDelay) {  // if it's not a long press, it will just !state of the LED
      inLeaf = false;
      LeafP = false;
      for (int i = 0; i <= 71; i++)
        EEPROM.update(i, 0);
      exitMenu = false;
    }
    buttonPressed = false;  // resetting the value for the next press
  }
}

void printDifficulty(int dif) {
  if (dif == 1 || dif == NA) {
    lcd.setCursor(0, 1);
    lcd.print(F("                "));
    lcd.setCursor(0, 1);
    lcd.print(F("| EASY >"));
  } else if (dif == 2) {
    lcd.setCursor(0, 1);
    lcd.print(F("                "));
    lcd.setCursor(0, 1);
    lcd.print(F("< MEDIUM >"));
  } else if (dif == 3) {
    lcd.setCursor(0, 1);
    lcd.print(F("                "));
    lcd.setCursor(0, 1);
    lcd.print(F("< HARD |"));
  }
}

void selectDifficulty() {
  static byte dif = DIFFICULTY;
  static byte ldif = dif;
  if (!SubMP) {
    SubMP = true;
    lcd.clear();
    printDifficulty(dif);
  }
  const char* scrollingText = "Select a difficulty:";
  int scrollingTextLength = strlen(scrollingText);
  scrollDisplay(scrollingText, scrollingTextLength);
  unsigned int yValue = analogRead(yPin);
  unsigned int xValue = analogRead(xPin);
  if (yValue + joystickSensitivity < minimumThreshold && !joystickMoved) {
    joystickMoved = true;
    dif -= 1;
  } else if (yValue - joystickSensitivity > maximumThreshold && !joystickMoved) {
    joystickMoved = true;
    dif += 1;
  } else if (yValue - joystickSensitivity < maximumThreshold && yValue + joystickSensitivity > minimumThreshold) {
    joystickMoved = false;
    exitMenu = false;
  }

  byte buttonState = digitalRead(buttonPin);

  if (!buttonState) {
    if (!buttonPressed) {               // verifying if the button was pressed
      buttonPressedTime = currentTime;  // starting recording the time when the button was first pressed
      buttonPressed = true;             // saving the fact that it was pressed
    }
  } else {
    if (buttonPressed && currentTime - buttonPressedTime >= swDebounceDelay) {  // if it's not a long press, it will just !state of the LED
      inSub = false;
      inMenu = true;
      MenuP = false;
      SubMP = false;
      exitMenu = false;
    }
    buttonPressed = false;  // resetting the value for the next press
  }

  if (dif != ldif) {
    if (dif == 0) {
      dif = 1;
    } else if (dif == 4) {
      dif = 3;
    }
    printDifficulty(dif);
    ldif = dif;
    DIFFICULTY = ldif;
  }
}

const byte minBrightnessLCD = 0;
const byte maxBrightnessLCD = 255;
const byte stepLCD = 15;
void printLCDBright(byte& lastBrightness) {
  lcd.setCursor(0, 1);
  lcd.print(F("                "));
  lcd.setCursor(0, 1);
  if (LCDBrightness == minBrightnessLCD) {
    lcd.print(F("|"));
    lcd.print(LCDBrightness);
    lcd.print(F(">"));
  } else if (LCDBrightness == maxBrightnessLCD) {
    lcd.print(F("<"));
    lcd.print(LCDBrightness);
    lcd.print(F("|"));
  } else {
    lcd.print(F("<"));
    lcd.print(LCDBrightness);
    lcd.print(F(">"));
  }
  EEPROM.update(brightnessLCD, LCDBrightness);
  lastBrightness = LCDBrightness;
  analogWrite(k, LCDBrightness);
}

void selectLCDBright() {
  const char* scrollingText = "Select LCD Brightness:";
  byte lastBrightness = LCDBrightness;

  if (!LeafP) {
    LeafP = true;
    lcd.clear();
    printLCDBright(lastBrightness);
    lcd.clear();
    printBrightMat();
  }

  int scrollingTextLength = strlen(scrollingText);
  scrollDisplay(scrollingText, scrollingTextLength);
  unsigned int yValue = analogRead(yPin);
  unsigned int xValue = analogRead(xPin);
  if (yValue + joystickSensitivity < minimumThreshold && !joystickMoved) {
    joystickMoved = true;
    if (LCDBrightness > minBrightnessLCD) {
      LCDBrightness -= stepLCD;
      if (sound)
        tone(buzzerPin, 700, 100);
    }
  } else if (yValue - joystickSensitivity > maximumThreshold && !joystickMoved) {
    joystickMoved = true;
    if (LCDBrightness < maxBrightnessLCD) {
      LCDBrightness += stepLCD;
      if (sound)
        tone(buzzerPin, 700, 100);
    }
  } else if (yValue - joystickSensitivity < maximumThreshold && yValue + joystickSensitivity > minimumThreshold) {
    joystickMoved = false;
    exitMenu = false;
  }

  byte buttonState = digitalRead(buttonPin);

  if (!buttonState) {
    if (!buttonPressed) {               // verifying if the button was pressed
      buttonPressedTime = currentTime;  // starting recording the time when the button was first pressed
      buttonPressed = true;             // saving the fact that it was pressed
    }
  } else {
    if (buttonPressed && currentTime - buttonPressedTime >= swDebounceDelay) {  // if it's not a long press, it will just !state of the LED
      inLeaf = false;
      LeafP = false;
      // exitMenu = true;
    }
    buttonPressed = false;  // resetting the value for the next press
  }

  if (lastBrightness != LCDBrightness) {
    printLCDBright(lastBrightness);
    lastBrightness = LCDBrightness;
  }
}

const byte minBrightnessMat = 0;
const byte maxBrightnessMat = 15;
void printMatBright(byte& lastBrightness) {
  lcd.setCursor(0, 1);
  lcd.print(F("                "));
  lcd.setCursor(0, 1);
  if (matrixBrightness == minBrightnessMat) {
    lcd.print(F("|"));
    lcd.print(matrixBrightness);
    lcd.print(F(">"));
  } else if (matrixBrightness == maxBrightnessMat) {
    lcd.print(F("<"));
    lcd.print(matrixBrightness);
    lcd.print(F("|"));
  } else {
    lcd.print(F("<"));
    lcd.print(matrixBrightness);
    lcd.print(F(">"));
  }
  EEPROM.update(brightnessMatrix, matrixBrightness);
  lastBrightness = matrixBrightness;
  lc.setIntensity(NA, matrixBrightness);
}

void selectMatBright() {
  const char* scrollingText = "Select Matrix Brightness:";
  byte lastBrightness = matrixBrightness;

  if (!LeafP) {
    lcd.clear();
    printBrightMat();
    LeafP = true;
    lcd.clear();
    printMatBright(lastBrightness);
  }

  int scrollingTextLength = strlen(scrollingText);
  scrollDisplay(scrollingText, scrollingTextLength);
  unsigned int yValue = analogRead(yPin);
  unsigned int xValue = analogRead(xPin);
  if (yValue + joystickSensitivity < minimumThreshold && !joystickMoved) {
    joystickMoved = true;
    if (matrixBrightness > minBrightnessMat) {
      matrixBrightness -= 1;
      if (sound)
        tone(buzzerPin, 700, 100);
    }
  } else if (yValue - joystickSensitivity > maximumThreshold && !joystickMoved) {
    joystickMoved = true;
    if (matrixBrightness < maxBrightnessMat) {
      matrixBrightness += 1;
      if (sound)
        tone(buzzerPin, 700, 100);
    }
  } else if (yValue - joystickSensitivity < maximumThreshold && yValue + joystickSensitivity > minimumThreshold) {
    joystickMoved = false;
    exitMenu = false;
  }

  byte buttonState = digitalRead(buttonPin);

  if (!buttonState) {
    if (!buttonPressed) {               // verifying if the button was pressed
      buttonPressedTime = currentTime;  // starting recording the time when the button was first pressed
      buttonPressed = true;             // saving the fact that it was pressed
    }
  } else {
    if (buttonPressed && currentTime - buttonPressedTime >= swDebounceDelay) {  // if it's not a long press, it will just !state of the LED
      inLeaf = false;
      LeafP = false;
      // exitMenu = true;
    }
    buttonPressed = false;  // resetting the value for the next press
  }

  if (lastBrightness != matrixBrightness) {
    printMatBright(matrixBrightness);
    lastBrightness = matrixBrightness;
  }
}

void scrollDisplay(const char* text, int textLength) {
  static int scrollPosition = 0;

  if (millis() - previousScrollTime >= scrollInterval) {
    lcd.setCursor(0, 0);            // Setează cursorul la începutul primei linii
    lcd.print("                ");  // Șterge textul anterior cu spații
    lcd.setCursor(0, 0);            // Setează din nou cursorul la începutul primei linii

    for (int i = 0; i < numCol; ++i) {
      lcd.print(text[(scrollPosition + i) % textLength]);  // Afișează textul cu scroll
    }

    scrollPosition++;
    if (scrollPosition >= textLength) {
      scrollPosition = 0;
    }

    previousScrollTime = millis();
  }
}

void setStartingEndingPositions() {
  // Clear the matrix
  clearMat();

  playerCol = random(matSize);
  playerRow = random(matSize);

  matrix[NA][playerRow][playerCol] = 'S';
}

void loop() {
  currentTime = millis();
  if (inStart) {
    Start();
  }
  if (!start) {
    Menu();
  } else {
    if (start == 1) {
      fillMatrix();
      printMat();
      start = 2;
    }
    if (!won) {
      timeLCD(currentTime);
      updateUserBlinking();
      movement();
      bombing();
    } else {
      if (sound)
        tone(buzzerPin, 1200, 100);
      byte buttonState = digitalRead(buttonPin);

      if (!buttonState) {
        if (!buttonPressed) {               // verifying if the button was pressed
          buttonPressedTime = currentTime;  // starting recording the time when the button was first pressed
          buttonPressed = true;             // saving the fact that it was pressed
        }
      } else {
        if (buttonPressed && currentTime - buttonPressedTime >= swDebounceDelay) {  // if it's not a long press, it will just !state of the LED
          MenuP = false;
          SubMP = false;
          LeafP = false;
          inMenu = true;
          MenuP = false;
          inSub = false;
          inLeaf = false;
          currentRoom = NA;
          all = true;
          Option = 0;
          start = 0;
          bombRow = -1;
          bombCol = -1;
          bombMat = -1;
          won = false;
          secP = false;
          userP = false;
          setStartingEndingPositions();
          printWaitingMat();
          lcd.clear();
        }
        buttonPressed = false;  // resetting the value for the next press
      }
    }
  }
}

void bombing() {
  byte buttonState = digitalRead(buttonPin);
  if (!buttonState) {
    if (!buttonPressed) {               // verifying if the button was pressed
      buttonPressedTime = currentTime;  // starting recording the time when the button was first pressed
      buttonPressed = true;             // saving the fact that it was pressed
    }
  } else {
    if (buttonPressed && currentTime - buttonPressedTime >= debounceDelay) {  // if it's not a long press, it will just !state of the LED
      if (bombRow == -1) {
        bombRow = playerRow;
        bombCol = playerCol;
        bombMat = currentRoom;
        matrix[bombMat][bombRow][bombCol] = '3';
        startBombingTime = millis();
        bombPlacedTime = millis();
        if (sound)
          tone(buzzerPin, 400, 100);
      }
    }
    buttonPressed = false;  // resetting the value for the next press
  }
  if (bombRow != -1) {
    if (sound)
      tone(buzzerPin, 300, 100);
    if (currentTime - bombPlacedTime >= 1000) {
      matrix[bombMat][bombRow][bombCol] -= 1;
      printMat();
      bombPlacedTime = currentTime;
    }
    if (matrix[bombMat][bombRow][bombCol] == '0') {
      matrix[bombMat][bombRow][bombCol] = walls[NA];
      bombed();
      bombRow = -1;
    } else if (currentTime - startBombingTime >= bombBlinkingInterval[matrix[bombMat][bombRow][bombCol] - 1 - ASCII0]) {
      bombBlinkState = !bombBlinkState;
      if (currentRoom == bombMat) {
        if (bombRow - 1 >= NA && matrix[bombMat][bombRow - 1][bombCol] == walls[1]) {
          lc.setLed(NA, bombRow - 1, bombCol, bombBlinkState);
        }
        if (bombRow + 1 < matSize && matrix[bombMat][bombRow + 1][bombCol] == walls[1]) {
          lc.setLed(NA, bombRow + 1, bombCol, bombBlinkState);
        }
        if (bombCol - 1 >= NA && matrix[bombMat][bombRow][bombCol - 1] == walls[1]) {
          lc.setLed(NA, bombRow, bombCol - 1, bombBlinkState);
        }
        if (bombCol + 1 < matSize && matrix[bombMat][bombRow][bombCol + 1] == walls[1]) {
          lc.setLed(NA, bombRow, bombCol + 1, bombBlinkState);
        }
      }
      startBombingTime = currentTime;
    }
  }
}

void bombed() {
  if (bombRow - 1 >= NA) {
    lc.setLed(NA, bombRow - 1, bombCol, false);
    matrix[bombMat][bombRow - 1][bombCol] = walls[NA];
  }
  if (bombRow + 1 < matSize) {
    lc.setLed(NA, bombRow + 1, bombCol, false);
    matrix[bombMat][bombRow + 1][bombCol] = walls[NA];
  }
  if (bombCol - 1 >= NA) {
    lc.setLed(NA, bombRow, bombCol - 1, false);
    matrix[bombMat][bombRow][bombCol - 1] = walls[NA];
  }
  if (bombCol + 1 < matSize) {
    lc.setLed(NA, bombRow, bombCol + 1, false);
    matrix[bombMat][bombRow][bombCol + 1] = walls[NA];
  }
  if (sound)
    tone(buzzerPin, 10, 100);

  if (checkWin()) {
    won = true;
    displayWinLED();
  }
}

void displayWinLED() {
  byte winMatrix[matSize][matSize] = {
    { 1, 1, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 1, 1 },
    { 1, 1, 0, 1, 1, 0, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 0, 0, 1, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 1, 1 }
  };
  rotateMatCounterClockwise(winMatrix);
  for (int row = NA; row < matSize; row++) {
    for (int col = NA; col < matSize; col++) {
      lc.setLed(NA, row, col, winMatrix[row][col]);
    }
  }
}

void updateUserBlinking() {
  if (currentTime - startTime >= userBlinkInterv) {  // blinking state of the cursor
    blinkState = !blinkState;
    lc.setLed(NA, playerRow, playerCol, blinkState);
    startTime = currentTime;
  }
}

void movement() {
  unsigned int xValue = analogRead(xPin);
  unsigned int yValue = analogRead(yPin);

  if (xValue + joystickSensitivity < minimumThreshold) {  // verifying the joystick motion for x value
    verifyJoystickMotion(NA, 1);                          // and the fact that the joystick could go in the desired direction
  } else if (xValue - joystickSensitivity > maximumThreshold) {
    verifyJoystickMotion(NA, -1);
  } else if (yValue + joystickSensitivity < minimumThreshold) {  // verifying the joystick motion for y value
    verifyJoystickMotion(-1, NA);                                // and the fact that the joystick could go in the desired direction
  } else if (yValue - joystickSensitivity > maximumThreshold) {
    verifyJoystickMotion(1, NA);
  } else joystickMoved = false;
}

bool DifferentRoom = false;
void verifyJoystickMotion(int posY, int posX) {
  if (!joystickMoved) {                   // verifying if it's not true so it won't move more than 1 place at a time
    if (verifyModifyState(posY, posX)) {  // verifying that it can go to that path
      // Serial.println(String(posX) + String(posY));
      lc.setLed(NA, playerRow, playerCol, false);
      if (!DifferentRoom) {
        playerRow += posY;
        playerCol += posX;
      } else {
        DifferentRoom = false;
      }

      lc.setLed(NA, playerRow, playerCol, true);
      startTime = currentTime;
      joystickMoved = true;  // setting it true so it won't move more than 1 place at a time
    }
  }
}

bool verifyModifyState(int posY, int posX) {
  if (matrix[currentRoom][playerRow + posY][playerCol + posX] == walls[1])
    return false;
  if (playerCol + posX == -1)
    if (currentRoom + 1 < NoRooms) {
      currentRoom++;
      playerCol = matSize - 1;
      DifferentRoom = true;
      Serial.println(currentRoom);
      printMat();
      return true;
    } else return false;
  if (playerCol + posX == matSize)
    if (currentRoom - 1 >= NA) {
      currentRoom--;
      playerCol = NA;
      DifferentRoom = true;
      printMat();
      return true;
    } else return false;
  if (playerRow + posY >= matSize)
    return false;
  if (playerRow + posY < NA)
    return false;
  if (playerCol + posX >= matSize)
    return false;
  if (playerCol + posX < NA)
    return false;
  return true;
}

void fillMatrix() {
  lc.clearDisplay(NA);  // clear screen
  int numHash;

  switch (DIFFICULTY) {
    case 1:
      numHash = 1;
      break;
    case 2:
      numHash = 4;
      break;
    case 3:
      numHash = 5;
      break;
  }

  // Fill the matrix
  for (int j = NA; j < NoRooms; j++) {
    for (int i = NA; i < numHash; i++) {
      int row = random(1, 6);
      int col = random(1, 6);
      if (matrix[j][row][col] == walls[NA])
        matrix[j][row][col] = walls[1];
      else i -= 1;
    }
  }

  matrix[NA][playerRow][playerCol] = ' ';
}

void clearMat() {
  for (int k = NA; k < NoRooms; k++)
    for (int i = NA; i < matSize; i++)
      for (int j = NA; j < matSize; j++)
        matrix[k][i][j] = walls[NA];
}

void printMat() {
  for (int row = NA; row < matSize; row++)
    for (int col = NA; col < matSize; col++)
      lc.setLed(NA, row, col, matrix[currentRoom][row][col] == walls[1] ? true : false);  // turns on LED at col, row
}

bool checkWin() {
  for (int room = 0; room < NoRooms; room++)
    for (int row = NA; row < matSize; row++)
      for (int col = NA; col < matSize; col++)
        if (matrix[room][row][col] != ' ')
          return false;
  userWinTime = millis();
  float score = float(userWinTime - userStartTime) / 1000;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("WON"));
  lcd.print(score);
  lcd.print(F("s"));
  lcd.setCursor(0, 1);
  lcd.print(F("Button to reset"));

  float highscores[noOfSavings];
  char names[noOfSavings][3];
  for (int i = 0; i < noOfSavings; i++) {
    EEPROM.get((DIFFICULTY - 1) * floatSize * noOfSavings + i * floatSize, highscores[i]);
    for (int j = 0; j < 3; j++) {
      names[i][j] = eeprom_read_byte(startNicks + (DIFFICULTY - 1) * 3 * noOfSavings + i * 3 + j);
    }
  }
  for (int i = 0; i < noOfSavings; i++)
    if (score < highscores[i] || !highscores[i]) {
      for (int j = noOfSavings - 1; j > i; j--) {
        highscores[j] = highscores[j - 1];
        for (int k = NA; k < 3; k++)
          names[j][k] = names[j - 1][k];
      }
      highscores[i] = score;
      for (int j = NA; j < 3; j++)
        names[i][j] = user[j];
      break;
    }

  for (int i = 0; i < noOfSavings; i++) {
    EEPROM.put((DIFFICULTY - 1) * floatSize * noOfSavings + i * floatSize, highscores[i]);
    for (int j = 0; j < 3; j++) {
      EEPROM.put(startNicks + (DIFFICULTY - 1) * 3 * noOfSavings + i * 3 + j, names[i][j]);
    }
  }
  return true;
}

void rotateMatCounterClockwise(byte matrix[matSize][matSize]) {
  char rotatedMatrix[matSize][matSize];
  for (int i = 0; i < matSize; ++i) {
    for (int j = 0; j < matSize; ++j) {
      rotatedMatrix[matSize - 1 - j][i] = matrix[i][j];
    }
  }
  for (int i = 0; i < matSize; ++i) {
    for (int j = 0; j < matSize; ++j) {
      matrix[i][j] = rotatedMatrix[i][j];
    }
  }
}
