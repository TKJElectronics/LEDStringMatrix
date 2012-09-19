#include "SPI.h"
#include "Adafruit_WS2801.h"

// Choose which 2 pins you will use for output.
// Can be any valid output pins.
// The colors of the wires may be totally different so
// BE SURE TO CHECK YOUR PIXELS TO SEE WHICH WIRES TO USE!
int dataPin  = 2;    // Yellow wire on Adafruit Pixels
int clockPin = 3;    // Green wire on Adafruit Pixels

// Don't forget to connect the ground wire to Arduino ground,
// and the +5V wire to a +5V supply

// Set the first variable to the NUMBER of pixels. 25 = 25 pixels in a row
Adafruit_WS2801 strip = Adafruit_WS2801(50, dataPin, clockPin);

//
// tables for plasma
//
// 7 by 7 -> 49 bytes
// 16 by 16 -> 256 bytes
// color table 256 bytes
//

#define PLASMA_W 7
#define PLASMA_H 7

#define TABLE_W 16
#define TABLE_H 16

unsigned char gPlasma[PLASMA_W*PLASMA_H];
unsigned char gTable1[TABLE_W*TABLE_H];
unsigned char gTable2[TABLE_W*TABLE_H];
unsigned char gColorTable[256];
float   gCircle1, gCircle2, gCircle3, gCircle4, gCircle5, gCircle6, gCircle7, gCircle8;
int    gRoll;

void Plasma_CalcTable1 ()
{
  for (int i=0; i< TABLE_H; i++)
  {
    for (int j=0; j< TABLE_W; j++)
    {
	int index = (i*TABLE_W)+j;
	gTable1[index] = (unsigned char) ((sqrt(16.0+(PLASMA_H-i)*(PLASMA_H-i)+(PLASMA_W-j)*(PLASMA_W-j))-4) *5 );
    }
  }
}

void Plasma_CalcTable2 ()
{
  for (int i=0; i< TABLE_H; i++)
  {
    for (int j=0; j< TABLE_W; j++)
    {
	int index = (i*TABLE_W)+j;
	float temp = sqrt(16.0+(PLASMA_H-i)*(PLASMA_H-i)+(PLASMA_W-j)*(PLASMA_W-j))-4;
	gTable2[index] = (sin(temp/9.5)+1)*90;
    }
  }
}

void Plasma_SetColor (int index, unsigned char red, unsigned char green, unsigned char blue)
{
  unsigned char new_color = (red & 0xE0) + ((green & 0xE0) >> 3) + ((blue & 0xC0) >> 6);
  gColorTable [index] = new_color;
}

double gRed, gGreen, gBlue;

#define color(u,a) (cos((u)+(a))+1)*127

void BuildColorTable()
{
  double u;
  int i;
  for (i=0; i<256; i++)
  {
    u=2*PI/256*i;
    Plasma_SetColor(i,color(u,gRed),color(u,gGreen),color(u,gBlue));
  }

  gRed+=0.05;
  gGreen-=0.05;
  gBlue+=0.1;
}

void Plasma_Setup ()
{
  gCircle1 = 0;
  gCircle2 = 0;
  gCircle3 = 0;
  gCircle4 = 0;
  gCircle5 = 0;
  gCircle6 = 0;
  gCircle7 = 0;
  gCircle8 = 0;
  gRoll = 0;

  for (int i=0; i<PLASMA_W*PLASMA_H; i++)
    gPlasma[i] = 0;

  Plasma_CalcTable1 ();
  Plasma_CalcTable2 ();

  gRed=1.0/6.0*PI;
  gGreen=3.0/6.0*PI;
  gBlue=5.0/6.0*PI;

  BuildColorTable ();
}

void ComputePlasma (int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4,int roll)
{
  int i, j;

  for (i=0; i< PLASMA_H; i++)
  {
    for (j=0; j< PLASMA_W; j++)
    {
	int index = (i*PLASMA_W)+j;

	unsigned int new_height = gTable1[TABLE_W*(i+y1)+j+x1];
	new_height = new_height + roll;
	new_height = new_height + gTable2[TABLE_W*(i+y2)+j+x2];
	new_height = new_height + gTable2[TABLE_W*(i+y3)+j+x3]; // + gTable2[TABLE_W*(i+y4)+j+x4];

	new_height = new_height & 0xFF;

	new_height = gColorTable[new_height];

	gPlasma[index] = new_height;
    }
  }
}

void SendToLEDs ()
{
  unsigned char x, y;
  int i = 0;
  //Send the color buffer to the RGB Matrix
  for (y = 0; y < PLASMA_H; y++)
  {
	for (x = 0; x < PLASMA_W; x++)
	{
		setXYpixel(x, y, (gPlasma[i] & 0xE0), ((gPlasma[i] & 0x1C) << 3), ((gPlasma[i] & 0x03) << 6));
		i++;
	}	
  }  
  strip.show();
}


void SetColor (int index)
{
  index = index % 256;

  unsigned char new_color = gColorTable[index];
  for(int i=0; i<PLASMA_H*PLASMA_W; i++)
  {
    gPlasma[i] = new_color;
  }
}

void setup()
{
  Serial.begin(9600);
    
  Plasma_Setup();  
    
  strip.begin();

  // Update LED contents, to start they are all 'off'
  strip.show();     
}

void loop()
{
  int x1,y1,x2,y2,x3,y3,x4,y4;

  float ratio = 2;

  gCircle1 = gCircle1 + (ratio * 0.085/6);
  gCircle2 = gCircle2 - (ratio * 0.1/6);
  gCircle3 = gCircle3 + (ratio * 0.3/6);
  gCircle4 = gCircle4 - (ratio * 0.2/6);
  gCircle5 = gCircle5 + (ratio * 0.4/6);
  gCircle6 = gCircle6 - (ratio * 0.15/6);
  gCircle7 = gCircle7 + (ratio * 0.35/6);
  gCircle8 = gCircle8 - (ratio * 0.05/6);

  x2=(PLASMA_W/2)+(PLASMA_W/2)*sin(gCircle1);
  y2=(PLASMA_H/2)+(PLASMA_H/2)*cos(gCircle2);

  x1=(PLASMA_W/2)+(PLASMA_W/2)*cos(gCircle3);
  y1=(PLASMA_H/2)+(PLASMA_H/2)*sin(gCircle4);

  x3=(PLASMA_W/2)+(PLASMA_W/2)*cos(gCircle5);
  y3=(PLASMA_H/2)+(PLASMA_H/2)*sin(gCircle6);

  x4=(PLASMA_W/2)+(PLASMA_W/2)*cos(gCircle7);
  y4=(PLASMA_H/2)+(PLASMA_H/2)*sin(gCircle8);

  ComputePlasma(x1,y1,x2,y2,x3,y3,x4,y4,gRoll);

   gRoll = gRoll + 1;

  //    SetColor (gRoll);

  SendToLEDs();

  delay (20);
}

void setXYpixel(char x, char y, unsigned char R, unsigned char G, unsigned char B)
{ 
  if ((y % 2) > 0)
    strip.setPixelColor((((6-y+1) * 7) - (6-x+1)), R, G, B);
  else
    strip.setPixelColor((((6-y) * 7) + (6-x)), R, G, B); 
}
