#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RFID Pins
#define SS_PIN  5
#define RST_PIN 4
MFRC522 rfid(SS_PIN, RST_PIN);

// Joystick Analog Pins
#define PIN_VRX 34
#define PIN_VRY 35

// Game Constants
#define MAX_SNAKE_LENGTH 50
#define SNAKE_SIZE 4

// --- ACCESS CONTROL VARIABLES ---
bool gameUnlocked = false;
// Put your card's UID code inside these quotes if you want it strictly locked!
String targetUID = "49 94 3A 2A"; 

// Snake Directions
enum Direction { UP, RIGHT, DOWN, LEFT };

// Game Variables
int snakeX[MAX_SNAKE_LENGTH];
int snakeY[MAX_SNAKE_LENGTH];
int snakeLength = 3;
Direction currentDir = RIGHT;

int foodX, foodY;
bool gameOver = false;
int score = 0;

void spawnFood() {
  foodX = (random(0, (SCREEN_WIDTH / SNAKE_SIZE) - 1)) * SNAKE_SIZE;
  foodY = (random(0, (SCREEN_HEIGHT / SNAKE_SIZE) - 1)) * SNAKE_SIZE;
}

void resetGame() {
  snakeLength = 3;
  currentDir = RIGHT;
  score = 0;
  gameOver = false;
  
  // 1. Fill the ENTIRE array with a dummy coordinate (like -100) 
  // so new tail parts don't spawn at 0,0 and crash the game!
  for (int i = 0; i < MAX_SNAKE_LENGTH; i++) {
    snakeX[i] = -100;
    snakeY[i] = -100;
  }

  // 2. Set up your starting 3 pieces safely
  for (int i = 0; i < snakeLength; i++) {
    snakeX[i] = 64 - (i * SNAKE_SIZE);
    snakeY[i] = 32;
  }
  spawnFood();
}

void setup() {
  Serial.begin(115200);
  
  // Initialize SPI and RFID
  SPI.begin();
  rfid.PCD_Init();
  
  pinMode(PIN_VRX, INPUT);
  pinMode(PIN_VRY, INPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  randomSeed(analogRead(32)); 
  resetGame();
}

void loop() {
  // --- STATE 1: LOCKED SECURITY SCREEN ---
  if (!gameUnlocked) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(25, 10);
    display.print("LOCKED");
    
    display.setTextSize(1);
    display.setCursor(20, 42);
    display.print("Scan RFID Badge");
    display.display();

    // Check for an RFID Card
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      // Build the scanned Card's ID string
      String scannedUID = "";
      for (byte i = 0; i < rfid.uid.size; i++) {
        scannedUID += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
        scannedUID += String(rfid.uid.uidByte[i], HEX);
        if(i < rfid.uid.size - 1) scannedUID += " ";
      }
      scannedUID.toUpperCase();

      // Print to Serial Monitor
      Serial.print("SCANNED CARD UID: ");
      Serial.println(scannedUID);

      // Access Check
      if (targetUID == "" || scannedUID == targetUID) {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(25, 15);
        display.print("ACCESS");
        display.setCursor(20, 35);
        display.print("GRANTED!");
        display.display();
        delay(2000);
        
        gameUnlocked = true;
        resetGame();
      } else {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(25, 25);
        display.print("DENIED!");
        display.display();
        delay(1500);
      }
      rfid.PICC_HaltA();
    }
    return; 
  }

  // --- STATE 2: GAME OVER SCREEN ---
  if (gameOver) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 15);
    display.print("GAME OVER");
    display.setTextSize(1);
    display.setCursor(35, 40);
    display.print("Score: ");
    display.print(score);
    display.setCursor(15, 52);
    display.print("Re-locking system...");
    display.display();
    
    delay(3000); // Show score for 3 seconds, then kick back to lock screen
    gameUnlocked = false; 
    return;
  }

  // --- STATE 3: ACTIVE GAMEPLAY ---
  int xVal = analogRead(PIN_VRX); 
  int yVal = analogRead(PIN_VRY); 
  
  if (xVal > 3600 && currentDir != DOWN) {
    currentDir = UP;
  } else if (xVal < 400 && currentDir != UP) {
    currentDir = DOWN;
  }
  
  if (yVal > 3600 && currentDir != LEFT) {
    currentDir = RIGHT;
  } else if (yVal < 400 && currentDir != RIGHT) {
    currentDir = LEFT;
  }

  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  switch (currentDir) {
    case UP:    snakeY[0] -= SNAKE_SIZE; break;
    case DOWN:  snakeY[0] += SNAKE_SIZE; break;
    case LEFT:  snakeX[0] -= SNAKE_SIZE; break;
    case RIGHT: snakeX[0] += SNAKE_SIZE; break;
  }

  if (snakeX[0] < 0 || snakeX[0] >= SCREEN_WIDTH || snakeY[0] < 0 || snakeY[0] >= SCREEN_HEIGHT) {
    gameOver = true;
  }

  for (int i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      gameOver = true;
    }
  }

  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    score++;
    if (snakeLength < MAX_SNAKE_LENGTH) {
      snakeLength++;
    }
    spawnFood();
  }

  display.clearDisplay();
  display.fillRect(foodX, foodY, SNAKE_SIZE, SNAKE_SIZE, SSD1306_WHITE);
  for (int i = 0; i < snakeLength; i++) {
    display.fillRect(snakeX[i], snakeY[i], SNAKE_SIZE, SNAKE_SIZE, SSD1306_WHITE);
  }
  display.display();
  
  delay(130); 
}
