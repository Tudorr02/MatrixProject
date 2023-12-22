#include <LiquidCrystal.h>

#include "pitches.h"

#include <LedControl.h>

#include <EEPROM.h>


// Pin configuration for the LCD display
const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 13;
const byte d6 = 5;
const byte d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const byte dinPin = 12; // Pin 12 is connected to the MAX7219 pin 1
const byte clockPin = 11; // Pin 11 is connected to the CLK pin 13
const byte loadPin = 10; // Pin 10 is connected to LOAD pin 12

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);

const byte lcdBPin = 6; //brightness pin for LCD

// Analog joystick pins
const byte xPin = A0;
const byte yPin = A1;
const byte swPin = 2;

// EEPROM addresses for LCD brightness settings
const byte lcdBrightnessAddr = 50;
byte lcdBrightnessVal = 0;
byte brightnessSteps = 7;

// EEPROM addresses for MATRIX brightness settings
const byte matrixBrightnessAddr = 60;
byte matrixBrightnessVal = 0;

//Sound configuration
const byte soundPin = A2; // Sound output pin
const int volume = 200;
// Frequencies for different sounds
const int gotkeyFreq = 698;
const int menuFreq = 93;
const int gameFreq = 14;
const int introFreq = 1700;
const int introFreq2 = 1519;

// Introductory text displayed on the LCD
const char introText[16] = "Impossible Road";

// Custom LCD character for the game
const byte skeletonIndex = 0;
const byte skeleton[] = { // used to simulate the loading bar
  B01110,
  B11111,
  B10101,
  B10101,
  B11111,
  B01010,
  B01110,
  B00000
};

byte selectedOptionIndex = 1;
byte selectedOptionCharacter[] = {
  B00000,
  B00000,
  B00100,
  B00110,
  B00111,
  B00110,
  B00100,
  B00000
};

const byte lockerIndex = 2;
byte lockerCharacter[] = {
  B00000,
  B01110,
  B10001,
  B10001,
  B11111,
  B11111,
  B11011,
  B11111
};

const byte keyIndex = 3;
byte keyCharacter[] = {
  B01110,
  B10001,
  B10001,
  B01110,
  B00100,
  B00100,
  B00110,
  B00100
};

const byte livesIndex = 4;
byte livesCharacter[] = {
  B00000,
  B01010,
  B11111,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000
};

const byte arrowIndex = 5;
byte arrowCharacter[] = {
  B00000,
  B00100,
  B00100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100
};

// LCD dimensions
const byte lcdRowSize = 2;
const byte lcdColSize = 16;

// Joystick movement thresholds
const int minThreshold = 300;
const int maxThreshold = 700;

// Enum for joystick directions
enum joystickDirection {
  UP,
  DOWN,
  RIGHT,
  LEFT,
  PRESS
};

// Delays for joystick movement and debounce
const int moveJoystickDelay = 150;
unsigned long lastMove = 0;
const int debounceDelay = 200;
unsigned long lastDebounce = 0;
bool pressed = false;

// EEPROM data for game levels,
// If the player gets the minimum keys in the 1st level then 2nd level unlocks and so on, 
// If the player changes the nickname , the levels 2 and 3 becomes locked again ( means that a new player will play the game  / a new profile is generated)
const int level2LockedAddr = 700;
const int level3LockedAddr = 750;
bool level2Locked = true;
bool level3Locked = true;

// Start menu text
const char * startOptionsMessage[] = {
  "     Level 1",
  "     Level 2",
  "     Level 3",
  "     Exit",
  " "

};

// Parent Menu text 
const char * parentOptionsMessage[] = {
  "  Start Game ",
  "  High Score ",
  "  Settings ",
  "  About ",
  "  How To Play",
  " "
};

//Settings text
char soundsMsg[] = "  Sounds [OFF] "; // [OFF] <-> [ON]
char alertLedMsg[] = "  AlertKey [OFF] "; //  [OFF] <-> [ON]
const char * settingsOptionsMessage[] = {
  "  Enter Name",
  "  LCD Brightness",
  "  Matrix Brightness ",
  soundsMsg,
  alertLedMsg,
  "  Reset Scores",
  "  Exit ",
  " "
};

//About Text
const char aboutMessage[] = "Impossible Road | Author: Ilie Octavian Tudor | Github: Tudorr02";

// Variables for menu navigation
byte selectedOption = 0; // Currently selected menu option
byte arrayComponents = 0; // Number of components in the current menu array
byte pointerRow = 0; // The user cursor
byte submenuPointerRow = 0; // The user cursor, used in submenus
int state = 0;
// state=0 => Cycle though Main menu
// state=-1 => Block infinite main menu printing
// state=2 => Call menu option's functions 

enum menuOptions {
  START = 10, LVL1, LVL2, LVL3, S_EXIT,
    HIGHSCORE = 20,
    SETTINGS = 30, ENTERNAME, LCD_BRIGHTNESS, MATRIX_BRIGHTNESS, SOUNDS, ALERTS, RESETHIGHSCORE, EXIT,
    ABOUT = 40,
    HOWTOPLAY = 50
};

// Flag to prevent redundant menu prints
bool printLock = false;

struct level {

  byte numberOfWalls; // Number of walls in the level

  byte NoEnemies;
  int minAttackDelay; // Minimum delay for enemies shooting
  int maxAttackDelay; // Maximum delay for enemies shooting
  byte minAttackArea; // The minimum number of ( bullet ) dots per attack
  byte maxAttackArea; //  The minimum number of ( bullet ) dots per attack

  byte lives;
  byte maxPoints; // Available points in the map
  byte minPoints; // Minimum points you need to have to be able to unlock the next level
};

const byte distanceBetweenEnemies = 2; // Distance between enemies for spawn validation
byte gameLevel = 0; // Current game level
const byte NoLevels = 3;
level gameLevels[NoLevels] = {
  { //first level
    10, // numberOfWalls
    5, // NoEnemis
    2000, // minADelay
    2000, // maxADelay
    3, // minAttackArea
    3, // maxAttackArea
    5, // lives
    20, // maxPoints
    4, // minPoints
  },
  { //second level
    7, // numberOfWalls
    7, // NoEnemis
    1000, // minADelay
    2000, // maxADelay
    3, // minAttackArea
    3, // maxAttackArea
    5, // lives
    15, // maxPoints
    6, // minPoints
  },
  { //third level
    5, // numberOfWalls
    9, // NoEnemis
    800, // minADelay
    1000, // maxADelay
    4, // minAttackArea
    4, // maxAttackArea
    5, // lives
    6, // maxPoints
    5, // minPoints
  }
};

struct enemy {

  byte xPos;
  byte yPos;
  byte attackArea; // The number of bullets per attack
  byte bulletArea; // The current bullet 
  int attackDelay; // Delay between attacks
  unsigned long lastAttack = 0; //  Last time the enemy attacked
  unsigned long lastBlink = 0; // // Last time the enemy blinked
};

// Dynamic array for storing enemies
enemy * enemies;

const byte mapSize = 16; // Size of the game map
byte gameMap[mapSize][mapSize] = {}; // Game map array

enum components {
  EMPTY,
  WALL,
  ENEMY,
  POINT,
  BULLET,
  PLAYER
};

const int enemyBlinkDelay = 100; // Delay between enemy blinks

const byte playerNameLength = 6; // Maximum length of the player's name
char pName[playerNameLength] = "_____"; // The initial text when trying to type a new nickname
byte letterIndex = 0; // Used to track the position of the cursor when trying to type the nickname

struct Player {
  byte xPos;
  byte yPos;
  bool hasPressed;
  byte lives;
  byte keys; // Number of keys collected
  byte score;
  char name[playerNameLength];
  components futureComponent; // Component that the player will move onto

  /// viewLUx,viewLUy  
  /// ................  
  /// ... matrix.....
  ///.................
  /// ................
  byte viewLUx, viewLUy; // used for getting the first point of the  matrix in which player is randomly spawned ( take in consideration that we have a 16x16 map and we print a portion of it into a 8x8 matrix)
  //
  // 0,0 -> 1 st matrix
  // 0,8 -> 2 nd matrix
  // 8,0 -> 3 rd matrix 
  // 8,8 -> 4 th matrix
};

Player * player;

byte viewSize = 8; // Size of the viewable area in the LED 8x8 matrix

bool gameStarted = 0; // Flag to indicate if the game has started 
unsigned long lastUpdate = 0; // Time of the last printed stats on lcd
byte attacked = false; // Flag to indicate if the player was attacked

// If the button press exceeds this delay then :
// 1. if the player has collected the minimum required keys , then the level will be passed
// 2. if the player has not collected enough keys , then the level will be taken as lost and he won t be able to access future levels
// In both cases the game will be ended , and the player will be able to cycle through the menu options 
const unsigned long longPressedDelay = 2000;
unsigned long pressStartTime = 0; // Store the timestamp when the button was initially pressed.

struct HighScore {
  byte score;
  char playerName[playerNameLength];
  int addr; // EEPROM address for storing this high score
};

// Array for storing top 3 high scores , saved in EEPROM
HighScore highscore[3] = {
  {
    0,
    " ",
    300
  },
  {
    0,
    "",
    330
  },
  {
    0,
    "",
    360
  }
};

// Temporary high score for the current player
// This highscore will be restored when the player's nickname will be changed
HighScore tempHighScore = {
  0,
  "",
  150
};

//This variable represents the current index in the high score list.
// It can be used to track which high score entry is currently being displayed or updated.
byte highscoreIndex = 0;

// Sounds status saves in EEPROM
// sounds=1 -> sounds ON
// sounds=0 -> sounds OFF
byte soundsAddr = 400;
byte sounds = 0;

// Alert LED variables
const byte ledPin = A3;
const byte blinkDelayLED = 500;
unsigned long lastBlink = 0;
byte ledState = LOW;
const byte alertArea = 1; // Trigger alert area 
byte alertsLed = 0;
const int alertsLedAddr = 500;

//Delay before triggering a reset if the map is not generated
const int resetErrorDelay = 5000;

unsigned long previousMillis = 0; // Used for non-blocking melody playing
const int noteDuration = 200; // Duration of each note in the melody
int melodyIndex = 0; // Index of the current note in the melody array

byte playerHsPlace = 5; // Decides if the player has beaten a highscore position 

// Array of notes for the melody
int melody[] = {
  NOTE_E7,
  NOTE_E7,
  0,
  NOTE_E7,
  0,
  NOTE_C7,
  NOTE_E7,
  0,
  NOTE_G7,
  0,
  0,
  0,
  NOTE_G6,
  0,
  0,
  0,
  // ... More notes ...
};

//This character array contains instructions or information on how to play the game.
char howToPlayMsg[] = "Navigate through a maze-like map. Avoid enemies that move and attack. Collect keys scattered across the map. Unlock and advance through levels.";
int txtLine = 0; // Index of the current text line in the scrolling text

void setup() {

  randomSeed(analogRead(A5)); // Initializes the random number generator 

  Serial.begin(9600);
  getEEPROMVals();
  matrixConfiguration();
  setLCDBrightness(lcdBrightnessVal);

  // set up the LCD's number of columns and rows:
  lcd.begin(lcdColSize, lcdRowSize);

  pinMode(ledPin, OUTPUT);
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(swPin, INPUT_PULLUP);
  pinMode(lcdBPin, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(swPin), buttonPressed, FALLING);
  setCustomCharacters();
  introMessage();

}
void loop() {

  Menu();

}

void playSong() {
  // Function to play a song using the melody array
  playNoteNonBlocking(melody[melodyIndex]); // Play the current note
}

void playNoteNonBlocking(int note) {
  unsigned long currentMillis = millis();

  // Check if it's time to play the next note
  if (currentMillis - previousMillis >= noteDuration) {
    // Update the previousMillis variable to the current time
    previousMillis = currentMillis;

    // Check if there are more notes in the melody to play
    if (melodyIndex < sizeof(melody) / sizeof(int)) {
      // Play the next note
      tone(soundPin, melody[melodyIndex], noteDuration);
      // Move to the next note in the melody array
      melodyIndex++;
    } else {
      // Reset the melody
      melodyIndex = 0;
      noTone(soundPin); // Stop playing
    }
  }
}
void( * resetFunc)(void) = 0; // Function pointer for resetting the Arduino

// void printEEPROMValues() {
//   int eepromSize = EEPROM.length(); // Get the total number of bytes in the EEPROM
//   byte value;
//   for (int i = 100; i < 200; i++) {
//     EEPROM.get(i, value); // Read each byte of the EEPROM

//     Serial.print("Address ");
//     Serial.print(i);
//     Serial.print(": ");
//     Serial.println(value); // Print the value in hexadecimal
//   }

// }

void updateLevel() {
  // Updates the current level, handling player movement and game state.
  updateView(); // Updates the LED matrix view
  readPlayerMove(); // Handles player's movement
  updateComponents(); // Updates enemies and other game components
  updateStats(); // Updates the player's stats
  menuSound(); // Handles sound effects
  AlertKey(alertArea); // Handles alert LED functionality

}

void printMap() {
  // Prints the current game map to the serial monitor. Useful for debugging.
  Serial.println("-----------------------------------");
  for (int i = 0; i < 16; i++) {
    Serial.println();
    for (int j = 0; j < 16; j++)
      Serial.print(gameMap[i][j]);
  }

}

void writeHighScoreToEEPROM(HighScore & hs) {
  // Writes a high score to EEPROM.
  EEPROM.put(hs.addr, hs);

}

void AlertKey(byte area) {
  // Alerts the player with an LED blink when near a point in the game.
  if (alertsLed) {
    // Checks for points near the player and blinks the LED
    for (int i = (player -> xPos - area); i <= player -> xPos + area; i++) {
      for (int j = (player -> yPos - area); j <= player -> yPos + area; j++)
        if (gameMap[i][j] == POINT) {
          ledBlink(blinkDelayLED);
        }

    }
  }

}

void ledBlink(int delay) {
  // Function to blink an LED with a specified delay.
  digitalWrite(ledPin, ledState);
  if (millis() - lastBlink > blinkDelayLED) {
    ledState = !ledState;
    lastBlink = millis();
  }
}

void getHighScores() {
  // Retrieves high scores from EEPROM.
  for (int i = 0; i < 3; i++) {
    EEPROM.get(highscore[i].addr, highscore[i]);
    //Serial.println(highscore[i].playerName);
  }

}

void updateView() {
  // Updates the LED matrix to show the current view of the game map.
  for (int row = 0; row < viewSize; row++) {
    for (int col = 0; col < viewSize; col++) {
      lc.setLed(0, row, col, gameMap[player -> viewLUx + row][player -> viewLUy + col]);
    }
  }

}

void updateComponents() {
  // Updates game components like enemies
  for (int i = 0; i < gameLevels[gameLevel].NoEnemies; i++) {
    enemyBlinkAnimtion(enemies[i]);
    enemyAttackAnimation(enemies[i]);
  }
}

void generatePoints() {
  // Generates keys (objectives) on the game map.
  byte xPos;
  byte yPos;

  for (int i = 0; i < gameLevels[gameLevel].maxPoints; i++) {

    bool search = true;

    while (search) {

      xPos = random(0, mapSize);
      yPos = random(0, mapSize);

      // Randomly places points on the map, avoiding the player and enemies
      if (gameMap[xPos][yPos] != PLAYER && notInEnemyArea(xPos, yPos) && gameMap[xPos][yPos] == EMPTY) {
        gameMap[xPos][yPos] = POINT;
        search = false;

        Serial.print("Point generated X: ");
        Serial.print(xPos);
        Serial.print("  Y: ");
        Serial.println(yPos);
      }

    }
  }
}

void spawnPlayer() {
  // Spawns the player at a random location on the map.
  bool search = true;
  while (search) {

    byte xSpawnPos = random(0, mapSize);
    byte ySpawnPos = random(0, mapSize);

    // Randomly selects a spawn location for the player, avoiding enemies
    if (notInEnemyArea(xSpawnPos, ySpawnPos) && gameMap[xSpawnPos][ySpawnPos] == EMPTY) {

      player -> xPos = xSpawnPos;
      player -> yPos = ySpawnPos;
      player -> hasPressed = false;

      gameMap[xSpawnPos][ySpawnPos] = PLAYER;

      getView(player);
      Serial.print("Player Spawned X: ");
      Serial.print(xSpawnPos);
      Serial.print("   Y: ");
      Serial.println(ySpawnPos);
      Serial.print("Your Spawn View X: ");
      Serial.print(xSpawnPos - (player -> viewLUx));
      Serial.print("   Y: ");
      Serial.println(ySpawnPos - (player -> viewLUy));

      Serial.print("View X: ");
      Serial.print(player -> viewLUx);
      Serial.print(" Y: ");
      Serial.println(player -> viewLUy);
      search = false;
    }
  }

}

void getView(Player * p) {

  int xPos = p -> xPos;
  int yPos = p -> yPos;

  int matrixIndex = (xPos / 8) * 2 + (yPos / 8);

  // Calculate the first LED coordinates of that matrix in the 16x16 grid
  switch (matrixIndex) {
  case 0: {
    player -> viewLUx = 0;
    player -> viewLUy = 0;
  } // Top-left matrix

  break;
  case 1: {
    player -> viewLUx = 0;
    player -> viewLUy = 8;
  } // Top-right matrix

  break;
  case 2: {

    player -> viewLUx = 8;
    player -> viewLUy = 0;
  }
  // Bottom-left matrix
  break;
  case 3: {
    player -> viewLUx = 8;
    player -> viewLUy = 8;
  } // Bottom-right matrix
  break;
  default:
    Serial.println("ERROR MATR INDEX");

  }
}

bool notInEnemyArea2(byte row, byte col, byte distance, byte index) {
  // Function to check if a position is not within a certain distance of existing enemies.
  for (int i = 0; i < index; i++) {
    //up area
    if (row <= enemies[i].xPos && row >= enemies[i].xPos - distance && col == enemies[i].yPos)
      return false;

    // upper-right area
    if (row >= enemies[i].xPos - distance && row <= enemies[i].xPos &&
      col >= enemies[i].yPos && col <= enemies[i].yPos + distance)
      return false;

    // upper-left area
    if (row >= enemies[i].xPos - distance && row <= enemies[i].xPos &&
      col >= enemies[i].yPos - distance && col <= enemies[i].yPos)
      return false;

    //down area
    if (row >= enemies[i].xPos && row <= enemies[i].xPos + distance && col == enemies[i].yPos)
      return false;

    if (row >= enemies[i].xPos && row <= enemies[i].xPos + distance &&
      col >= enemies[i].yPos - distance && col <= enemies[i].yPos)
      return false;

    // lower-right area
    if (row >= enemies[i].xPos && row <= enemies[i].xPos + distance &&
      col >= enemies[i].yPos && col <= enemies[i].yPos + distance)
      return false;

    //left area
    if (col <= enemies[i].yPos && col >= enemies[i].yPos - distance && row == enemies[i].xPos)
      return false;

    //right area
    if (col >= enemies[i].yPos && col <= enemies[i].yPos + distance && row == enemies[i].xPos)
      return false;

  }

  return true;

}

bool notInEnemyArea(byte row, byte col) {
  // Function to check if a position is not in the attack area of any enemy.
  for (int i = 0; i < gameLevels[gameLevel].NoEnemies; i++) {

    //up area
    if (row <= enemies[i].xPos && row >= enemies[i].xPos - enemies[i].attackArea && col == enemies[i].yPos) {
      return false;
    }

    //down area
    if (row >= enemies[i].xPos && row <= enemies[i].xPos + enemies[i].attackArea && col == enemies[i].yPos) {
      return false;
    }

    //left area
    if (col <= enemies[i].yPos && col >= enemies[i].yPos - enemies[i].attackArea && row == enemies[i].xPos) {
      return false;
    }

    //right area
    if (col >= enemies[i].yPos && col <= enemies[i].yPos + enemies[i].attackArea && row == enemies[i].xPos) {
      return false;
    }

  }

  return true;

}

void generateGameMap(byte number) {
  // Generates the game map for a given level.
  // Generates walls, enemies, spawns player, and places points based on level configuration.
  generateWalls(gameLevels[number].numberOfWalls);
  generateEnemies(gameLevels[number].NoEnemies, gameLevels[number].minAttackDelay, gameLevels[number].maxAttackDelay, gameLevels[number].minAttackArea, gameLevels[number].maxAttackArea);
  spawnPlayer();
  generatePoints();

}

void generateWalls(byte numberOfWalls) {
  // Randomly places walls on the map, ensuring they don't overlap or block paths excessively.
  unsigned long startGenerate = millis();
  while (numberOfWalls > 0) {
    byte xWall = random(mapSize);
    byte yWall = random(mapSize);

    if (millis() - startGenerate > resetErrorDelay) {
      resetFunc();
    }

    if (isNotWallArea(xWall, yWall)) {
      // Place the wall
      byte x = xWall;
      byte y = yWall;
      for (byte i = 0; i < 3; i++)
        for (byte j = 0; j < 2; j++)
          gameMap[x + i][y + j] = WALL;

      Serial.print("Generated Wall X: ");
      Serial.print(x);
      Serial.print("  Y: ");
      Serial.println(y);
      numberOfWalls--;
    }
  }

}

void updateHighScore() {
  // Compares the player's current score to the stored high scores and updates if necessary.
  for (int i = 0; i < 3; i++) {
    if (player -> score >= highscore[i].score) {
      playerHsPlace = i + 1;
      for (int j = 2; j > i; j--) {
        highscore[j].score = highscore[j - 1].score;
        strcpy(highscore[j].playerName, highscore[j - 1].playerName);
      }
      highscore[i].score = player -> score;
      strcpy(highscore[i].playerName, player -> name);
      break;
    }
  }

  Serial.println("Updated");

  for (int i = 0; i < 3; i++) {
    writeHighScoreToEEPROM(highscore[i]);
    Serial.println(highscore[i].score);
  }

}

bool isNotWallArea(byte startX, byte startY) {
  // Checks if the given area does not have any walls.
  // Ensures that the specified area is free of walls and can be used for placing game elements.
  byte x = startX;
  byte y = startY;

  if (x < 1 || x > mapSize - 4 || y < 1 || y > mapSize - 3)
    return false;

  for (byte row = x - 1; row < x + 4; row++) {
    for (byte col = y - 1; col < y + 3; col++)
      if (gameMap[row][col] == WALL)
        return false;
  }

  return true;

}

void levelWon() {
  // Displays a message, updates high scores, and resets the game state.
  lcd.clear();
  lcd.home();
  lcd.print(F("   LEVEL PASSED  "));
  updateHighScore();
  if (selectedOption == LVL1) {
    level2Locked = false;
    EEPROM.put(level2LockedAddr, level2Locked);
  }

  if (selectedOption == LVL2) {
    level3Locked = false;
    EEPROM.put(level3LockedAddr, level3Locked);
  }

  tempHighScore.score = player -> score;
  strcpy(tempHighScore.playerName, player -> name);
  writeHighScoreToEEPROM(tempHighScore);

  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.write(skeletonIndex);
    delay(200);

  }

  if (!printLock) {
    printLock = true;
    lcd.clear();
    lcd.home();
    if (playerHsPlace < 4) {
      lcd.print("YOUR PLACE");
      lcd.setCursor(0, 1);
      lcd.print("IN HIGHSCORE: ");
      lcd.print(playerHsPlace);
    } else {
      lcd.print("YOU DID NOT BEAT");
      lcd.setCursor(0, 1);
      lcd.print(" THE HIGHSCORE ");
    }

  }

  while (true) {
    joystickDirection direction = joystickControl();
    if (direction == PRESS) {
      //selectedOption = EXIT;
      printLock = false;
      gameStarted = false;
      endGame();
      playerHsPlace = 5;

      setParentOption();

      return;
    }
  }

}

void readPlayerMove() {
  // Reads and processes the player's movement.
  // Interprets joystick input to move the player and updates the game state accordingly.
  joystickDirection move = joystickControl();

  digitalWrite(ledPin, LOW);
  if (micros() - pressStartTime > (longPressedDelay * 1000) && digitalRead(swPin) == LOW) {
    if (player -> keys >= gameLevels[gameLevel].minPoints)
      levelWon();
    else
      levelLost(); // = exit or game lost

  }

  if (millis() - lastMove > moveJoystickDelay) {

    switch (move) {
    case UP: {
      if (player -> xPos > 0 && gameMap[player -> xPos - 1][player -> yPos] != WALL && gameMap[player -> xPos - 1][player -> yPos] != ENEMY) {

        gameMap[player -> xPos][player -> yPos] = EMPTY;
        player -> xPos--;
        liveCameraMove(UP);
      }

    }
    break;

    case DOWN: {
      if (player -> xPos < (mapSize - 1) && gameMap[player -> xPos + 1][player -> yPos] != WALL && gameMap[player -> xPos + 1][player -> yPos] != ENEMY) {
        gameMap[player -> xPos][player -> yPos] = EMPTY;
        player -> xPos++;
        liveCameraMove(DOWN);
      }

    }
    break;

    case LEFT: {
      if (player -> yPos > 0 && gameMap[player -> xPos][player -> yPos - 1] != WALL && gameMap[player -> xPos][player -> yPos - 1] != ENEMY) {
        gameMap[player -> xPos][player -> yPos] = EMPTY;
        player -> yPos--;
        liveCameraMove(LEFT);
      }

    }
    break;

    case RIGHT: {
      if (player -> yPos < (mapSize - 1) && gameMap[player -> xPos][player -> yPos + 1] != WALL && gameMap[player -> xPos][player -> yPos + 1] != ENEMY) {
        gameMap[player -> xPos][player -> yPos] = EMPTY;
        player -> yPos++;
        liveCameraMove(RIGHT);
      }

    }
    break;

    case PRESS: {

    }
    break;

    default: {}
    }

    player -> futureComponent = gameMap[player -> xPos][player -> yPos];
    gameMap[player -> xPos][player -> yPos] = PLAYER;

    lastMove = millis();
  }

}

void liveCameraMove(joystickDirection direction) {
  // Updates the LED matrix to follow the player's movement.
  // Adjusts the portion of the map displayed on the LED matrix based on player movement.
  Serial.print(" X: ");
  Serial.print(player -> xPos);
  Serial.print(" Y: ");
  Serial.println(player -> yPos);

  switch (direction) {

  case LEFT: {
    if (player -> viewLUy > 0 && player -> yPos < 12)
      player -> viewLUy--;
  }
  break;

  case RIGHT: {
    if (player -> viewLUy < 8 && player -> yPos > 3)
      player -> viewLUy++;
  }
  break;

  case UP: {
    if (player -> viewLUx > 0 && player -> xPos < 12)
      player -> viewLUx--;
  }
  break;

  case DOWN: {
    if (player -> viewLUx < 8 && player -> xPos > 3)
      player -> viewLUx++;
  }
  break;

  default: {}
  }

}

void endGame() {
  // Frees allocated memory, resets the game map, and resets game variables.
  free(enemies);
  free(player);
  for (int i = 0; i < 16; i++)
    for (int j = 0; j < 16; j++)
      gameMap[i][j] = 0;

}

void setLevel(byte number) {
  // Initializes player stats, generates the game map, and sets up the level.
  initializeStats(number);
  generateGameMap(number);

  printStats(player -> keys, player -> lives, player -> score, gameLevels[gameLevel].minPoints, gameLevel + 1);
  printMap();

}

void initializeStats(byte number) {
  // Sets initial values for the player's lives, score, and other stats.
  player = (Player * ) malloc(sizeof(Player));
  player -> keys = 0;
  player -> lives = gameLevels[gameLevel].lives;

  if (selectedOption == LVL1)
    player -> score = 0;
  else {
    player -> score = tempHighScore.score;
  }
  strcpy(player -> name, tempHighScore.playerName);

}

void printStats(byte keys, byte lives, byte score, byte minimumKeys, byte level) {
  // Prints the current game stats on the LCD.
  // Displays player's current keys, lives, score, and other relevant information.
  lcd.clear();

  lcd.home();
  lcd.write(livesIndex);
  lcd.setCursor(0, 1);
  lcd.print(lives);

  lcd.setCursor(2, 0);
  lcd.write(keyIndex);
  lcd.setCursor(2, 1);
  lcd.print(keys);
  lcd.print(F("/"));
  lcd.print(minimumKeys);

  lcd.setCursor(6, 0);
  lcd.print(F("SCORE : "));
  lcd.print(score);

  Serial.print(player -> keys);

  lcd.setCursor(6, 1);
  if (player -> keys >= gameLevels[gameLevel].minPoints) {
    lcd.print(F("PRESS BTN"));
  } else {
    lcd.print(F("LEVEL : "));
    lcd.print(level);
  }

}

void updateStats() {
  // Adjusts player's stats like keys and lives based on game interactions and events.
  if (player -> futureComponent == POINT) {
    Serial.print("Got a key");

    if (millis() - lastUpdate > moveJoystickDelay) {
      player -> keys++;
      player -> score += player -> lives;
      printStats(player -> keys, player -> lives, player -> score, gameLevels[gameLevel].minPoints, gameLevel + 1);
      lastUpdate = millis();
    }

  }

  if (player -> lives == 0)
    levelLost();

}

void levelLost() {
  // Displays a message and resets the game state.
  lcd.clear();
  lcd.home();
  lcd.print(F("   LEVEL LOST  "));
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.write(skeletonIndex);
    delay(200);
  }

  gameStarted = false;
  endGame();
  setParentOption();

}

void generateEnemies(byte number, int minDelay, int maxDelay, byte minArea, byte maxArea) {
  // Generates enemy characters on the game map.
  // Places a specified number of enemies on the map with given attack characteristics.
  enemies = (enemy * ) malloc(number * sizeof(enemy));
  Serial.print("ENEMIES: ");
  Serial.println(number);

  for (int i = 0; i < number; i++) {

    bool search = true;

    if (i == 0) {

      while (search) {
        enemies[i].xPos = random(mapSize);
        enemies[i].yPos = random(mapSize);

        if (gameMap[enemies[i].xPos][enemies[i].xPos] != WALL) {
          enemies[i].attackArea = random(minArea, maxArea + 1);
          enemies[i].attackDelay = random(minDelay, maxDelay + 1);
          enemies[i].lastAttack = 0;
          enemies[i].lastBlink = 0;
          enemies[i].bulletArea = 1;
          gameMap[enemies[i].xPos][enemies[i].yPos] = 2;
          Serial.print("- Enemy generated X: ");
          Serial.print(enemies[i].xPos);
          Serial.print(" Y: ");
          Serial.println(enemies[i].yPos);
          search = false;
        }

      }

    } else {

      while (search) {
        enemies[i].xPos = random(mapSize);
        enemies[i].yPos = random(mapSize);

        if (notInEnemyArea2(enemies[i].xPos, enemies[i].yPos, distanceBetweenEnemies, i) && gameMap[enemies[i].xPos][enemies[i].xPos] != WALL) {
          enemies[i].attackArea = random(minArea, maxArea + 1);
          enemies[i].attackDelay = random(minDelay, maxDelay + 1);
          enemies[i].lastAttack = 0;
          enemies[i].lastBlink = 0;
          enemies[i].bulletArea = 1;
          gameMap[enemies[i].xPos][enemies[i].yPos] = 2;
          Serial.print("Enemy generated X: ");
          Serial.print(enemies[i].xPos);
          Serial.print(" Y: ");
          Serial.println(enemies[i].yPos);
          search = false;
        }
      }

    }

  }

}

void enemyAttackAnimation(enemy & _enemy) {
  // Handles the attack animation of enemies.

  // Check if the enemy is currently attacking (bulletArea is non-zero)
  if (_enemy.bulletArea != 0) {

    //Up
    if (_enemy.xPos - _enemy.bulletArea >= 0 && enemyShoot(_enemy.xPos - _enemy.bulletArea, _enemy.yPos)) {
      gameMap[_enemy.xPos - _enemy.bulletArea][_enemy.yPos] = BULLET;
    }
    // Down
    if (_enemy.xPos + _enemy.bulletArea < mapSize && enemyShoot(_enemy.xPos + _enemy.bulletArea, _enemy.yPos)) {
      gameMap[_enemy.xPos + _enemy.bulletArea][_enemy.yPos] = BULLET;
    }
    // Left
    if (_enemy.yPos - _enemy.bulletArea >= 0 && enemyShoot(_enemy.xPos, _enemy.yPos - _enemy.bulletArea)) {
      gameMap[_enemy.xPos][_enemy.yPos - _enemy.bulletArea] = BULLET;
    }
    // Right
    if (_enemy.yPos + _enemy.bulletArea < mapSize && enemyShoot(_enemy.xPos, _enemy.yPos + _enemy.bulletArea)) {
      gameMap[_enemy.xPos][_enemy.yPos + _enemy.bulletArea] = BULLET;
    }
  }

  // Check if it's time for the enemy to attack again based on the attack delay
  if (millis() - _enemy.lastAttack > _enemy.attackDelay) {

    _enemy.bulletArea++; // Increase the bullet area for the next attack

    // Reset attack if bullet area exceeds the maximum attack area
    if (_enemy.bulletArea > _enemy.attackArea) {

      // Clear the bullets from the map for each direction

      for (int i = 0; i <= _enemy.attackArea; i++) {
        if (_enemy.xPos - i >= 0 && enemyAttack(_enemy.xPos - i, _enemy.yPos)) {
          gameMap[_enemy.xPos - i][_enemy.yPos] = EMPTY;
        }
        // Down
        if (_enemy.xPos + i < mapSize && enemyAttack(_enemy.xPos + i, _enemy.yPos)) {
          gameMap[_enemy.xPos + i][_enemy.yPos] = EMPTY;
        }
        // Left
        if (_enemy.yPos - i >= 0 && enemyAttack(_enemy.xPos, _enemy.yPos - i)) {
          gameMap[_enemy.xPos][_enemy.yPos - i] = EMPTY;
        }
        // Right
        if (_enemy.yPos + i < mapSize && enemyAttack(_enemy.xPos, _enemy.yPos + i)) {
          gameMap[_enemy.xPos][_enemy.yPos + i] = EMPTY;
        }
      }
      attacked = false; // Reset attacked status
      _enemy.bulletArea = 0; // Reset bullet area for next attack

    }

    _enemy.lastAttack = millis(); // Update the time of the last attack
  }

}

bool enemyAttack(byte row, byte col) {
  // Checks if an enemy can attack a position without hitting a player or point.

  if (gameMap[row][col] != PLAYER && gameMap[row][col] != POINT) {
    return true;
  }

  return false;
}

bool enemyShoot(byte row, byte col) {
  // Determines if an enemy can shoot at a given position.
  // Checks if an enemy's shot would hit the player or a free space.

  if (gameMap[row][col] == PLAYER) {
    if (attacked == false) {
      player -> lives--;
      printStats(player -> keys, player -> lives, player -> score, gameLevels[gameLevel].minPoints, gameLevel + 1);
      attacked = true;
    }

  }
  if (gameMap[row][col] != ENEMY && gameMap[row][col] != PLAYER) {
    return true;
  }

  return false;
}

void blink(byte row, byte col, unsigned long & last, int delay, components state1, components state2) {

  if (millis() - last > delay) {

    if (gameMap[row][col] == state1)
      gameMap[row][col] = state2;
    else if (gameMap[row][col] == state2)
      gameMap[row][col] = state1;

    last = millis();
  }
}

void enemyBlinkAnimtion(enemy & _enemy) {
  // Handles the blinking animation of enemies.
  // Toggles the display of an enemy on the game map to create a blinking effect.
  blink(_enemy.xPos, _enemy.yPos, _enemy.lastBlink, enemyBlinkDelay, ENEMY, EMPTY);
}

void introMessage() {
  // Displaying intro text and a loading-like animation on LCD
  lcd.clear();
  lcd.home();
  lcd.print(introText);

  for (int i = 0; i < lcdColSize; i++) {
    lcd.setCursor(i, 1); //set the cursor to the next row/line;
    lcd.write(skeletonIndex);
    playSong();
    delay(200);

  }
}

joystickDirection joystickControl() {
  // Reads joystick inputs and returns the direction of movement
  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);

  if (pressed) {
    pressed = false;

    return PRESS;
  }

  if (xValue > maxThreshold) {

    return RIGHT;
  }

  if (xValue < minThreshold) {

    return LEFT;
  }

  if (yValue > maxThreshold) {

    return DOWN;
  }

  if (yValue < minThreshold) {

    return UP;
  }

  return -1;

}

void displayScrollingText(char * text) {
  // Displays scrolling text on the LCD, used for longer messages.
  int textLength = strlen(text);
  int totalLines = (textLength / 16) + 1;

  char copyText[totalLines][16];
  int index = 0;

  for (int i = 0; i < totalLines; i++)
    for (int j = 0; j < 16; j++) {
      if (text[index] == '\0')
        break;
      copyText[i][j] = text[index++];

    }

  lcd.home();
  lcd.write(copyText[txtLine], 16);
  lcd.setCursor(0, 1);
  lcd.write(copyText[txtLine + 1], 16);

  joystickDirection direction = joystickControl(); // Get joystick direction

  if (direction == PRESS) {
    selectedOption = EXIT;
    return;
  }

  if (millis() - lastMove > moveJoystickDelay) {
    lastMove = millis();

    if (direction == DOWN && txtLine < totalLines - 2) {
      txtLine++;
      lcd.clear();
    } else if (direction == UP && txtLine > 0) {
      txtLine--;
      lcd.clear();
    }

  }

}

void buttonPressed() {
  // Interrupt service routine for joystick button press
  // Debouncing and setting the 'pressed' flag for joystick button
  if (micros() - lastDebounce > (debounceDelay * 1000)) {
    pressed = true;

    pressStartTime = micros();
    lastDebounce = micros();
  }

}

void menuPrint(byte SORow, char * options[]) {
  // Function to print menu options on the LCD
  lcd.clear();
  byte col = 0;
  byte row = 0;
  byte option = (SORow % 2 == 0) ? SORow : (SORow - 1);

  byte position = SORow % 2;

  lcd.home();
  lcd.print(options[option]);

  lcd.setCursor(col, row + 1);
  lcd.print(options[option + 1]);

  lcd.setCursor(col, position);
  lcd.write(selectedOptionIndex);

}

void generateOption(byte value) {
  // Function to generate menu option based on joystick input
  // Modifying the selectedOption based on the menu item selected
  if (selectedOption < 10)
    selectedOption = (value + 1) * 10;
  else if (selectedOption % 10 == 0)
    selectedOption += (value + 1);

}

void setParentOption() {
  // Adjusting selectedOption to return to the parent menu
  selectedOption -= selectedOption % 10;
}

void updateMenuPrint(byte & SORow, byte length) {
  // Updating the menu display based on joystick movement
  joystickDirection direction = joystickControl();

  if (direction == PRESS) {
    if (state == 2) {
      printLock = false;
    }
    generateOption(SORow);
    state = 2;
    return;
  }

  if (SORow < length - 2 && direction == DOWN) {
    if (state == 2) {
      printLock = false;
    } else
      state = 0;
    SORow++;
  }

  if (SORow > 0 && direction == UP) {
    if (state == 2) {
      printLock = false;
    } else
      state = 0;
    SORow--;
  }

}

void settingsMenu() {
  // Settings menu logic
  // Handling the display and navigation of the settings menu
  byte options = sizeof(settingsOptionsMessage) / sizeof(settingsOptionsMessage[0]);

  if (!printLock) {

    menuPrint(submenuPointerRow, settingsOptionsMessage);
    printLock = true;
  }

  if (millis() - lastMove > moveJoystickDelay) {
    updateMenuPrint(submenuPointerRow, options);
    menuSound();
    lastMove = millis();
  }

}

void aboutMenu() {
  // Displaying the about message with scrolling effect
  lcd.setCursor(15, 1);
  lcd.print(aboutMessage);

  const int scrollDelay = 800;
  unsigned long lastScroll = 0;
  while (true) {

    if (millis() - lastScroll > scrollDelay) {
      lcd.scrollDisplayLeft();
      lastScroll = millis();
    }

    if (millis() - lastMove > moveJoystickDelay) {
      joystickDirection direction = joystickControl();

      if (direction == PRESS) {
        selectedOption = EXIT;
        return;
      }

      lastMove = millis();
    }

  }

}

void lcdBrightness() {
  // Adjusting the brightness of the LCD display
  if (!printLock) {
    lcd.clear();
    lcd.home();
    lcd.print(F("LCD Light Level"));
    lcd.setCursor(0, 1);
    lcd.print(F("Value: "));
    lcd.print(F("- "));
    printLock = true;
    lcd.setCursor(9, 1);
    lcd.print(lcdBrightnessVal);
    lcd.setCursor(10, 1);
    lcd.print(F(" +"));
  }

  int xValue = analogRead(xPin);

  if (millis() - lastMove > moveJoystickDelay) {
    lastMove = millis();

    if (xValue > maxThreshold || xValue < minThreshold) {
      if (xValue > maxThreshold && lcdBrightnessVal < brightnessSteps)
        lcdBrightnessVal += 1;

      if (xValue < minThreshold && lcdBrightnessVal > 0)
        lcdBrightnessVal -= 1;

      setLCDBrightness(lcdBrightnessVal);
      EEPROM.put(lcdBrightnessAddr, lcdBrightnessVal);
      lcd.setCursor(9, 1);
      lcd.print(lcdBrightnessVal);

    }

    joystickDirection direction = joystickControl();

    if (direction == PRESS) {
      selectedOption = SETTINGS;
      printLock = false;
      return;
    }

    lastMove = millis();
  }

}

void matrixBrightness() {
  // Adjusting the brightness of the LED matrix display
  lc.setIntensity(0, map(matrixBrightnessVal, 0, brightnessSteps, 0, 15));
  byte matrix[][8] = {
    {
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0
    },
    {
      0,
      1,
      1,
      1,
      1,
      1,
      1,
      0
    },
    {
      0,
      1,
      0,
      0,
      0,
      0,
      1,
      0
    },
    {
      0,
      1,
      0,
      0,
      0,
      0,
      1,
      0
    },
    {
      0,
      1,
      0,
      0,
      0,
      0,
      1,
      0
    },
    {
      0,
      1,
      0,
      0,
      0,
      0,
      1,
      0
    },
    {
      0,
      1,
      1,
      1,
      1,
      1,
      1,
      0
    },
    {
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0
    }
  };

  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      lc.setLed(0, row, col, matrix[row][col]);
    }
  }

  if (!printLock) {
    lcd.clear();
    lcd.home();
    lcd.print(F("Matrix Light Lvl"));
    lcd.setCursor(0, 1);
    lcd.print(F("Value: "));
    lcd.print(F("- "));
    printLock = true;
    lcd.setCursor(9, 1);
    lcd.print(matrixBrightnessVal);
    lcd.setCursor(10, 1);
    lcd.print(F(" +"));
  }

  int xValue = analogRead(xPin);

  if (millis() - lastMove > moveJoystickDelay) {
    lastMove = millis();

    if (xValue > maxThreshold || xValue < minThreshold) {
      if (xValue > maxThreshold && matrixBrightnessVal < brightnessSteps)
        matrixBrightnessVal += 1;

      if (xValue < minThreshold && matrixBrightnessVal > 0)
        matrixBrightnessVal -= 1;

      lc.setIntensity(0, map(matrixBrightnessVal, 0, brightnessSteps, 0, 15));
      EEPROM.put(matrixBrightnessAddr, matrixBrightnessVal);
      lcd.setCursor(9, 1);
      lcd.print(matrixBrightnessVal);

    }

    joystickDirection direction = joystickControl();

    if (direction == PRESS) {
      selectedOption = SETTINGS;
      printLock = false;
      return;
    }

    lastMove = millis();
  }

}

void menuPrintStartGame(byte SORow, char * options[]) {
  // Function to print menu options on the LCD
  lcd.clear();
  byte col = 0;
  byte row = 0;
  byte option = (SORow % 2 == 0) ? SORow : (SORow - 1);

  byte position = SORow % 2;

  lcd.home();

  lcd.print(options[option]);

  if (option == 2 && level3Locked == true) {
    lcd.setCursor(3, 0);
    lcd.write(lockerIndex);
  }

  lcd.setCursor(col, row + 1);
  lcd.print(options[option + 1]);

  if (option == 0 && level2Locked == true) {
    lcd.setCursor(3, 1);
    lcd.write(lockerIndex);
  }

  lcd.setCursor(col, position);
  lcd.write(selectedOptionIndex);

}

void startGameMenu() {
  // Start menu logic
  // Handling the display and navigation of the settings menu
  byte options = sizeof(startOptionsMessage) / sizeof(startOptionsMessage[0]);

  if (!printLock) {

    menuPrintStartGame(submenuPointerRow, startOptionsMessage);

    printLock = true;
  }

  if (millis() - lastMove > moveJoystickDelay) {
    updateMenuPrint(submenuPointerRow, options);
    menuSound();
    lastMove = millis();
  }
}

void Menu() {
  // Main function to handle the game menu interactions and state changes
  if (state == 2) {
    // Switch statement to handle different menu options based on user selection
    switch (selectedOption) {

    case LVL1: {
      if (!gameStarted) {
        printLock = false;
        gameStarted = true;
        setLevel(gameLevel = 0);
      }

      updateLevel();

    }
    break;

    case LVL2: {
      if (level2Locked == true) {
        selectedOption = START;
      } else {
        if (!gameStarted) {
          gameStarted = true;
          printLock = false;
          setLevel(gameLevel = 1);
        }

        updateLevel();
      }

    }
    break;

    case ENTERNAME:
      enterNameMenu();
      break;

    case RESETHIGHSCORE:
      resetHighScore();
      break;

    case LVL3: {
      if (level3Locked == true) {
        selectedOption = START;
      } else {
        if (!gameStarted) {
          printLock = false;
          gameStarted = true;
          setLevel(gameLevel = 2);
        }

        updateLevel();
      }

    }
    break;

    case HOWTOPLAY:
      displayScrollingText(howToPlayMsg);
      break;

    case ALERTS:
      alertLedToggle();
      break;

    case START: {
      startGameMenu();
    }
    break;

    case S_EXIT:
      selectedOption = EXIT;
      break;

    case HIGHSCORE:
      highScoreMenu();
      break;

    case SETTINGS:
      settingsMenu();
      break;

    case ABOUT:
      //aboutMenu();
      displayScrollingText(aboutMessage);
      break;

    case LCD_BRIGHTNESS:
      lcdBrightness();
      break;

    case SOUNDS:
      soundsToggle();
      break;

    case MATRIX_BRIGHTNESS:
      matrixBrightness();
      break;

    case EXIT: {
      setParentOption();
      printLock = false;
      submenuPointerRow = 0;
      state = 0;
    }
    break;

    default: {
      lcd.clear();
      lcd.home();
      lcd.print(F("ERROR MENU"));
      lcd.setCursor(0, 1);
      lcd.print(F("INVALID OPTION"));
    }

    }

  } else {
    // Handle the initial state of the menu
    if (state == 0) {
      selectedOption = 0;
      menuPrint(pointerRow, parentOptionsMessage); // Print the main menu
      state = -1;
      arrayComponents = sizeof(parentOptionsMessage) / sizeof(parentOptionsMessage[0]);
    }

    // Handle joystick movements for menu navigation
    if (millis() - lastMove > moveJoystickDelay) {
      updateMenuPrint(pointerRow, arrayComponents);
      menuSound();
      lastMove = millis();
    }

  }

}

void soundsToggle() {
  // Toggle sound settings and save the preference in EEPROM
  sounds = !sounds;
  EEPROM.put(soundsAddr, sounds);
  if (sounds)
    strcpy(soundsMsg, "  Sounds [ON] ");
  else
    strcpy(soundsMsg, "  Sounds [OFF] ");

  setParentOption();
}

void resetHighScore() {

  for (int i = 0; i < 3; i++) {
    highscore[i].score = 0;
    strcpy(highscore[i].playerName, "_____");
    writeHighScoreToEEPROM(highscore[i]);
  }

  selectedOption = SETTINGS;
}

void alertLedToggle() {
  // Toggle the state of alert LED and save the preference in EEPROM
  alertsLed = !alertsLed;

  EEPROM.put(alertsLedAddr, alertsLed);

  if (alertsLed)
    strcpy(alertLedMsg, "  AlertKey [ON] ");
  else
    strcpy(alertLedMsg, "  AlertKey [OFF] ");

  setParentOption();

}

void enterNameMenu() {

  if (!printLock) {
    lcd.clear();
    lcd.home();
    lcd.print(F(" Game  "));
    lcd.setCursor(0, 1);
    lcd.print(F(" Name :"));

    lcd.setCursor(letterIndex + 8, 0);
    lcd.write(arrowIndex);
    lcd.setCursor(8, 1);
    lcd.print(pName);
    printLock = true;

  }

  // Handling input from joystick for name entry
  joystickDirection direction = joystickControl();
  if (direction == PRESS) {
    // Save the name if it's not the default placeholder
    if (strcmp(pName, "_____") != 0) {
      strcpy(tempHighScore.playerName, pName);
      tempHighScore.score = 0;

      // Save the name and reset score
      writeHighScoreToEEPROM(tempHighScore);
      // Lock higher levels as a new player is entering their name
      EEPROM.put(level2LockedAddr, level2Locked = 1);
      EEPROM.put(level3LockedAddr, level3Locked = 1);
      strcpy(pName, "_____"); // Reset name to default
    }
    selectedOption = EXIT;
    printLock = false;
    return;
  }

  if (millis() - lastMove > moveJoystickDelay) {

    menuSound();
    if (direction == LEFT) {
      if (letterIndex > 0) {
        printLock = false;
        letterIndex--;
      }

    }

    if (direction == RIGHT) {
      if (letterIndex < playerNameLength - 2) {
        printLock = false;
        letterIndex++;
      }

    }

    if (direction == DOWN) {
      if (pName[letterIndex] == 'Z') // If current char is 'Z', change to 'a'
        pName[letterIndex] = 'a';
      else if (pName[letterIndex] == 'z') // If current char is 'z', change to '_'
        pName[letterIndex] = '_';
      else if (pName[letterIndex] == '_') // If current char is '_', wrap around to 'A'
        pName[letterIndex] = 'A';
      else if ((pName[letterIndex] >= 'A' && pName[letterIndex] < 'Z') ||
        (pName[letterIndex] >= 'a' && pName[letterIndex] < 'z'))
        pName[letterIndex]++; // Increment if within valid letter range
      else
        pName[letterIndex] = 'A'; // Default to 'A' if outside valid range

      printLock = false;
    }

    if (direction == UP) {
      if (pName[letterIndex] == 'A') // If current char is 'A', change to '_'
        pName[letterIndex] = '_';
      else if (pName[letterIndex] == '_') // If current char is '_', change to 'z'
        pName[letterIndex] = 'z';
      else if (pName[letterIndex] == 'a') // If current char is 'a', wrap around to 'Z'
        pName[letterIndex] = 'Z';
      else if ((pName[letterIndex] > 'A' && pName[letterIndex] <= 'Z') ||
        (pName[letterIndex] > 'a' && pName[letterIndex] <= 'z'))
        pName[letterIndex]--; // Decrement if within valid letter range
      else
        pName[letterIndex] = 'Z'; // Default to 'Z' if outside valid range

      printLock = false;
    }

    lastMove = millis();
  }

}

void highScoreMenu() {

  if (!printLock) {
    lcd.clear();
    lcd.home();
    lcd.print(F("  NAME    SCORE"));
    lcd.setCursor(0, 1);
    Serial.println(highscore[highscoreIndex].score);
    lcd.print(highscoreIndex + 1);
    lcd.print(F("."));
    lcd.setCursor(2, 1);
    lcd.print(highscore[highscoreIndex].playerName);
    lcd.setCursor(10, 1);
    lcd.print(highscore[highscoreIndex].score);
    printLock = true;
  }

  joystickDirection direction = joystickControl();
  if (direction == PRESS) {
    selectedOption = EXIT;
    printLock = false;
    return;
  }

  if (millis() - lastMove > moveJoystickDelay) {

    menuSound();

    if (direction == DOWN) {
      printLock = false;
      highscoreIndex++;
      if (highscoreIndex == 3)
        highscoreIndex = 0;
    }

    if (direction == UP) {
      printLock = false;
      highscoreIndex--;
      if (highscoreIndex > 250)
        highscoreIndex = 2;
    }

    lastMove = millis();
  }
}

void setCustomCharacters() {
  lcd.createChar(skeletonIndex, skeleton);
  lcd.createChar(selectedOptionIndex, selectedOptionCharacter);
  lcd.createChar(lockerIndex, lockerCharacter);
  lcd.createChar(keyIndex, keyCharacter);
  lcd.createChar(livesIndex, livesCharacter);
  lcd.createChar(arrowIndex, arrowCharacter);
}

void getEEPROMVals() {
  EEPROM.get(lcdBrightnessAddr, lcdBrightnessVal);
  EEPROM.get(matrixBrightnessAddr, matrixBrightnessVal);
  getHighScores();
  EEPROM.get(soundsAddr, sounds);
  EEPROM.get(alertsLedAddr, alertsLed);

  if (alertsLed)
    strcpy(alertLedMsg, "  AlertKey [ON] ");
  else
    strcpy(alertLedMsg, "  AlertKey [OFF] ");

  if (sounds)
    strcpy(soundsMsg, "  Sounds [ON] ");
  else
    strcpy(soundsMsg, "  Sounds [OFF] ");

  EEPROM.get(level2LockedAddr, level2Locked);
  EEPROM.get(level3LockedAddr, level3Locked);

  EEPROM.get(tempHighScore.addr, tempHighScore);

}

void setLCDBrightness(byte lcdBrightnessVal) {
  analogWrite(lcdBPin, map(lcdBrightnessVal, 0, brightnessSteps, 0, 255));
}

void matrixConfiguration() {
  lc.shutdown(0, false); // turn off power saving, enables display
  lc.setIntensity(0, map(matrixBrightnessVal, 0, brightnessSteps, 0, 15)); // sets brightness (0~15 possible values)
  lc.clearDisplay(0); // clear screen
}

void menuSound() {
  // Play different sounds based on the game state and interactions
  if (sounds == 1) {

    int sound;
    // Determine which sound to play based on game state
    if (player -> futureComponent == POINT)
      sound = gotkeyFreq;
    else
    if (gameStarted == 1)
      sound = menuFreq;
    else
      sound = gameFreq;

    if (joystickControl() == -1)
      noTone(soundPin);
    else
      tone(soundPin, sound, volume);

  }

}