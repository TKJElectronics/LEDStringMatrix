#include "SPI.h"
#include "Adafruit_WS2801.h"

#include <PS3BT.h>
USB Usb;
BTD Btd(&Usb); // You have to create the Bluetooth Dongle instance like so
/* You can create the instance of the class in two ways */
PS3BT PS3(&Btd); // This will just create the instance
//PS3BT PS3(&Btd,0x00,0x15,0x83,0x3D,0x0A,0x57); // This will also store the bluetooth address - this can be obtained from the dongle when running the sketch

int dataPin  = 2;    // Yellow wire on Adafruit Pixels
int clockPin = 3;    // Green wire on Adafruit Pixels

// Set the first variable to the NUMBER of pixels. 25 = 25 pixels in a row
Adafruit_WS2801 strip = Adafruit_WS2801(50, dataPin, clockPin);


byte oldSnakeItems = 0;
byte oldSnakeItemPosX[50];
byte oldSnakeItemPosY[50];
byte SnakeItems = 0;
byte SnakeItemPosX[50];
byte SnakeItemPosY[50];
byte ApplePosX;
byte ApplePosY;
byte AppleMoveCountDown;
#define APPLE_COUNTDOWN_STEPS_MIN 3
#define APPLE_COUNTDOWN_STEPS_MAX 7

#define BaneGridXmax 7 // Bredden på banen
#define BaneGridYmax 7 // Højden på banen
#define BLANK 0
#define SNAKE 1
#define APPLE 2
byte Playfield[BaneGridXmax+1][BaneGridYmax+1];

	
byte SnakeHeadID = 0; // Array ID på forreste Snake item (Hoved)
byte SnakeBackID = 0; // Array ID på bagerste Snake item

byte AppleCount = 0;
	
#define SNAKE_LEFT 0
#define SNAKE_RIGHT 1
#define SNAKE_UP 2
#define SNAKE_DOWN 3
byte movingDirection = SNAKE_RIGHT; // Vores nuværende retning
byte snakeDirection = SNAKE_RIGHT; // Piletasternes/vores næste retning

byte AddSnakeItem = 0; // Tilføj et snake item næste gang vi flytter os
byte SnakeItemsToAddAtApple = 1; // Hvor mange Snake objekter der skal tilføjes når der spises et æble

byte Score = 0;
boolean GameRunning = false;

void setup()
{  
  Serial.begin(9600);
    
  strip.begin();

  // Update LED contents, to start they are all 'off'
  strip.show();  
  
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while(1); //halt
  }
  Serial.print(F("\r\nPS3 Bluetooth Library Started"));     
}

long timeBefore = millis();
void loop()
{
  Usb.Task();
       
  if (PS3.PS3Connected || PS3.PS3NavigationConnected) { 
    if (PS3.buttonPressed && PS3.getButton(PS)) 
     NewGame();
    
    if ((millis() >= (timeBefore+500)) && (GameRunning == true)) {
      moveSnake();
      timeBefore = millis();
    }
    
  // Read directional keys here  
    if ((PS3.getAnalogHat(LeftHatX) > 150) && ((PS3.getAnalogHat(LeftHatY) < 150) && (PS3.getAnalogHat(LeftHatY) > 80)) && (movingDirection != LEFT)) // Joystick right
      snakeDirection = SNAKE_RIGHT;
    if ((PS3.getAnalogHat(LeftHatX) < 80) && ((PS3.getAnalogHat(LeftHatY) < 150) && (PS3.getAnalogHat(LeftHatY) > 80)) && (movingDirection != RIGHT)) // Joystick left
      snakeDirection = SNAKE_LEFT;      
  
    if ((PS3.getAnalogHat(LeftHatY) > 150) && ((PS3.getAnalogHat(LeftHatX) < 150) && (PS3.getAnalogHat(LeftHatX) > 80)) && (movingDirection != UP)) // Joystick down
      snakeDirection = SNAKE_DOWN;
    if ((PS3.getAnalogHat(LeftHatY) < 80) && ((PS3.getAnalogHat(LeftHatX) < 150) && (PS3.getAnalogHat(LeftHatX) > 80)) && (movingDirection != DOWN)) // Joystick up
      snakeDirection = SNAKE_UP;  
      
    if (PS3.buttonPressed) {
      if (PS3.getButton(RIGHT))
        snakeDirection = SNAKE_RIGHT;
      if (PS3.getButton(LEFT))
        snakeDirection = SNAKE_LEFT;
      
      if (PS3.getButton(DOWN))
        snakeDirection = SNAKE_DOWN;
      if (PS3.getButton(UP))
        snakeDirection = SNAKE_UP;
    }        
  }
}

void NewGame(void)
{
  byte x, y;
  
  SnakeItems = 1;
  SnakeHeadID = 1;
  SnakeItemPosX[1] = 1;
  SnakeItemPosY[1] = 1;
  movingDirection = SNAKE_RIGHT;
  snakeDirection = SNAKE_RIGHT;

  for (y = 1; y <= BaneGridYmax; y++)
  {
    for (x = 1; x <= BaneGridXmax; x++)
    {
      Playfield[x][y] = BLANK;
    }
  }
		
  AddSnakeItem = 0;
  AppleCount = 0;
  placeRandomApple();
  
  GameRunning = true; 
  render(); 
  timeBefore = millis();
}

void setXYpixel(char x, char y, unsigned char R, unsigned char G, unsigned char B)
{ 
  if ((y % 2) > 0)
    strip.setPixelColor((((6-y+1) * 7) - (6-x+1)), R, G, B);
  else
    strip.setPixelColor((((6-y) * 7) + (6-x)), R, G, B);
}

void placeApple(byte x, byte y) {
  if (x > 0 && y > 0 && x <= BaneGridXmax && y <= BaneGridYmax) {
    Playfield[x][y] = APPLE;
    ApplePosX = x;
    ApplePosY = y;
    AppleMoveCountDown = random(APPLE_COUNTDOWN_STEPS_MIN, APPLE_COUNTDOWN_STEPS_MAX);
			
    AppleCount++;
  }
}

void removeApple(void) {
  Playfield[ApplePosX][ApplePosY] = BLANK;
  AppleCount = 0; 
}

void render(void) { // Render de forskellige snake Items
  byte i, x, y;
  
  for (i=1; i <= oldSnakeItems; i++) {
    if (oldSnakeItemPosX[i] > 0 && oldSnakeItemPosY[i] > 0 && oldSnakeItemPosX[i] <= BaneGridXmax && oldSnakeItemPosY[i] <= BaneGridYmax) {
      Playfield[oldSnakeItemPosX[i]][oldSnakeItemPosY[i]] = BLANK;
    }
  }
		
  for (i=1; i <= SnakeItems; i++) {
    if (SnakeItemPosX[i] > 0 && SnakeItemPosY[i] > 0 && SnakeItemPosX[i] <= BaneGridXmax && SnakeItemPosY[i] <= BaneGridYmax) {			
      Playfield[SnakeItemPosX[i]][SnakeItemPosY[i]] = SNAKE;
      oldSnakeItemPosX[i] = SnakeItemPosX[i];
      oldSnakeItemPosY[i] = SnakeItemPosY[i];			
    }
  }
  oldSnakeItems = SnakeItems;
  
  if (AppleCount > 0 && AppleMoveCountDown == 0) {
    removeApple();
    placeRandomApple();
  } else if (AppleCount > 0) {
    AppleMoveCountDown--;
  }
  
  for (y = 1; y <= BaneGridYmax; y++)
  {
    for (x = 1; x <= BaneGridXmax; x++)
    {
      switch (Playfield[x][y]) {
        case BLANK:       
          setXYpixel((x-1), (y-1), 0, 0, 0);
          break;       
        case SNAKE:
          if (SnakeItemPosX[SnakeHeadID] == x && SnakeItemPosY[SnakeHeadID] == y)
            setXYpixel((x-1), (y-1), 255, 255, 0); // Yellow snake head
          else
            setXYpixel((x-1), (y-1), 0, 255, 0); // Green snake body
          break;  
        case APPLE:
          setXYpixel((x-1), (y-1), 255, 0, 0);
          break;   
        default:   
          setXYpixel((x-1), (y-1), 0, 0, 0);
          break; 
      }          
    }
  }
  strip.show();
}

void moveSnake(void) {
  byte i;
  movingDirection = snakeDirection; // Sæt movingDirection til den retning vi har valgt med piletasterne

  if (AddSnakeItem == 0) { // Flyt det bagerste Snake Objekt til fronten, og sæt SnakeHeadID til dette objekts ID
    SnakeBackID = SnakeHeadID - 1;
    if (SnakeBackID == 0) SnakeBackID = SnakeItems;
    SnakeItemPosX[SnakeBackID] = SnakeItemPosX[SnakeHeadID];
    SnakeItemPosY[SnakeBackID] = SnakeItemPosY[SnakeHeadID];
    switch (movingDirection) {
      case SNAKE_RIGHT:
        SnakeItemPosX[SnakeBackID] += 1;
	break;
      case SNAKE_LEFT:
	SnakeItemPosX[SnakeBackID] -= 1;
	break;
      case SNAKE_DOWN:
	SnakeItemPosY[SnakeBackID] += 1;
	break;
      case SNAKE_UP:
	SnakeItemPosY[SnakeBackID] -= 1;
	break;
    }			
    SnakeHeadID = SnakeBackID;
  } else { // Skal vi tilføje et Snake objekt (AddSnakeItem > 0), da skal vi tilføje én foran, UDEN at fjerne den bagved
    for (i = SnakeItems; i >= SnakeHeadID; i--) {
      SnakeItemPosX[i+1] = SnakeItemPosX[i];
      SnakeItemPosY[i+1] = SnakeItemPosY[i];
    }
    SnakeItemPosX[SnakeHeadID] = SnakeItemPosX[SnakeHeadID+1];
    SnakeItemPosY[SnakeHeadID] = SnakeItemPosY[SnakeHeadID+1];
    switch (movingDirection) {
      case SNAKE_RIGHT:
        SnakeItemPosX[SnakeHeadID] += 1;
        break;
      case SNAKE_LEFT:
	SnakeItemPosX[SnakeHeadID] -= 1;
	break;
      case SNAKE_DOWN:
	SnakeItemPosY[SnakeHeadID] += 1;
	break;
      case SNAKE_UP:
	SnakeItemPosY[SnakeHeadID] -= 1;
	break;
    }	
			
    SnakeItems++;			
    AddSnakeItem--;
  }
		
    // Befinder vi os inden for banen?						
    if (SnakeItemPosX[SnakeHeadID] > 0 && SnakeItemPosX[SnakeHeadID] <= BaneGridXmax && SnakeItemPosY[SnakeHeadID] > 0 && SnakeItemPosY[SnakeHeadID] <= BaneGridYmax) {
      if (Playfield[SnakeItemPosX[SnakeHeadID]][SnakeItemPosY[SnakeHeadID]] != SNAKE) { // Er hovedets position på et blankt eller æble felt?
	if (Playfield[SnakeItemPosX[SnakeHeadID]][SnakeItemPosY[SnakeHeadID]] == APPLE) { // Er hovedets position på et æble felt
	  Score++;
	  AddSnakeItem += SnakeItemsToAddAtApple; // Tilføj x-antal snake items (bliver tilføjet til fronten af snake løbende)
	  AppleCount--; // Fjern et æble fra RAM'en
	}			
	if (AppleCount == 0) { // Hvis der ikke er flere æbler på banen
	  placeRandomApple(); // placer da et æble et tilfældigt sted
	}
	render(); // Render Snake objekterne i de rigtige felter
      } else { // Game over da vi ramte ind i os selv (snake felt)
	GameOver();				
      }
    } else { // Game over da vi ramte ind i kanten
      GameOver();
    }		
  }


void placeRandomApple(void) {
  byte x, y;
  x = random(1, BaneGridXmax);
  y = random(1, BaneGridYmax);
  while (Playfield[x][y] != BLANK) {
    x = random(1, BaneGridXmax);
    y = random(1, BaneGridYmax);  
  }
  placeApple(x, y);
}

void GameOver(void) {
  if (SnakeItemPosX[SnakeHeadID] == 0) SnakeItemPosX[SnakeHeadID]++;
  else if (SnakeItemPosX[SnakeHeadID] > BaneGridXmax) SnakeItemPosX[SnakeHeadID]--;
  if (SnakeItemPosY[SnakeHeadID] == 0) SnakeItemPosY[SnakeHeadID]++;
  else if (SnakeItemPosY[SnakeHeadID] > BaneGridYmax) SnakeItemPosY[SnakeHeadID]--;	
  
  if (SnakeHeadID < SnakeItems)	
    setXYpixel(SnakeItemPosX[SnakeHeadID+1]-1, SnakeItemPosY[SnakeHeadID+1]-1, 0, 255, 0); // Set second snake object to green (from yellow head color)
  else
    setXYpixel(SnakeItemPosX[1]-1, SnakeItemPosY[1]-1, 0, 255, 0); // Set second snake object to green (from yellow head color)
    
  setXYpixel(SnakeItemPosX[SnakeHeadID]-1, SnakeItemPosY[SnakeHeadID]-1, 255, 100, 0); // Dark orange if dead snake head
  strip.show();
  
  GameRunning = false;
}
