// Galaxy-Man added ftp to animated gif 
// https://github.com/Galaxy-Man?tab=repositories

// Kudos goes to
// mrfaptastic https://github.com/mrfaptastic
// Brian Lough https://github.com/witnessmenow
// Marc Merlin https://github.com/marcmerlin

// and for the FM612x reset routine
// Bob Davis https://github.com/bobdavis321
// Bob Davis https://bobdavis321.blogspot.com/search?q=matrix

#include <Arduino.h>
#include <WiFi.h>

#define FILESYSTEM SPIFFS
#include <FS.h>
#include <SPIFFS.h>

#include "esp32FtpServer.h"

FtpServer ftpSrv; 

// Example sketch which shows how to matrix a 64x32
// animated GIF image stored in FLASH memory
// on a 64x32 LED matrix
//
// To matrix a GIF from memory, a single callback function
// must be provided - GIFDRAW
// This function is called after each scan line is decoded
// and is passed the 8-bit pixels, RGB565 palette and info
// about how and where to matrix the line. The palette entries
// can be in little-endian or big-endian order; this is specified
// in the begin() method.
//
// The AnimatedGIF class doesn't allocate or free any memory, but the
// instance data occupies about 22.5K of RAM.
//
// Credits: https://github.com/bitbank2/AnimatedGIF/tree/master/examples/ESP32_LEDMatrix_I2S
//

#include <SPI.h>
#include <Wire.h>
#include "AnimatedGIF.h"
#include "ESP32-RGB64x32MatrixPanel-I2S-DMA.h"

//////////////////////////////////////////////////////////////////////////////
const char *ssid = "testreserved";
const char *password = "Pa55w0rd!6101";
////////////////////////////////////////////////////////////

RGB64x32MatrixPanel_I2S_DMA matrix;
AnimatedGIF gif;
File f;
int x_offset, y_offset;

// https://www.amazon.co.uk/gp/product/B06ZYYDK3B/ref=ppx_yo_dt_b_asin_title_o03_s00?ie=UTF8&psc=1
// RGB LED Matrix panel FM6124/FM6126 chip set
// P3-6432-2121-16S-D1.0
// 6432 = 64 pixels x 32 pixels
// 16S = 1/16 scan rate
// P3 = 3mm pitch centre to centre of LED
// 2121 = LED size 2.1mm x 2.1mm

// Module: P3 Full Color LED Module
// Pixel type: Real pixel
// Pixel pitch : 3 mm
// refresh frequency :>=400HZ
// Module Size: 192X96 mm
// Horizontal View Angel: > 110 degree
// Model Pixel: 64X32 Pixels
// **Working Environment
// Temp: -20°C ~ ± 60°C / -4℉~±140℉
// Humidity : 10%-90%
// lattice density ： 111111 dots/square meter
// Working voltage： 5 Volts DC - 4 pin Molex connector
// Connector between MCU and panel: HUB75E
// Pixel form： 1R1PG1GB SMD2121
// Lifetime： >=100000 hours
// Best viewing distance： 3m~30 m
// Gray scale： 12 bit/ 1 color
// Max power consumption：1400W/square meter
// Color matrix： manual adjust
// Ave power consumption： 600W/square meter
// Brightness： 2200 cd/square meter
// Blind spot rate: <0.00001
// Drive mode( Optional): Constant current 1/16 scan
// Waterproof Class: IP43
// Control system: DVI card+Full color control card
// Software: Support window series system
// Indoor Full Color LED matrix Function:

//  HUB75E connector on panel
//  R1 | G1
//  B1 | GND
//  R2 | G2
//  B2 | E
//  A  | B
//  C  | D
//  CLK| LAT
//  OE | GND
//////////////////////////////////////////////////////////////////////////////
// pinout for ESP38 pin module yellow header pins.
// http://arduinoinfo.mywikis.net/wiki/Esp32#KS0413_keyestudio_ESP32_Core_Board
//
/*--------------------- RGB matrix PINS -------------------------*/
#define R1 25
#define G1 26
#define BL1 27
#define R2 5  // 21 SDA
#define G2 19 // 22 SDL
#define BL2 23
#define CH_A 12
#define CH_B 16
#define CH_C 17
#define CH_D 18
#define CH_E -1 // assign to pin 14 if using two panels
#define CLK 15
#define LAT 32
#define OE 33

 
//RGB64x32MatrixPanel_I2S_DMA matrix; // RGB Panel
//////////////////////////////////////////////////////////////////////////////
// how many pixels wide if you chain panels
// 4 panels of 64x32 is 256 wide.
int MaxLed = MATRIX_WIDTH;

int C12[16] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int C13[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0};

void resetPanel()
{
  pinMode(CLK, OUTPUT);
  pinMode(LAT, OUTPUT);
  pinMode(OE, OUTPUT);
  pinMode(R1, OUTPUT);
  pinMode(G1, OUTPUT);
  pinMode(BL1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(G2, OUTPUT);
  pinMode(BL2, OUTPUT);
  pinMode(CH_A, OUTPUT);
  pinMode(CH_B, OUTPUT);
  pinMode(CH_C, OUTPUT);
  pinMode(CH_D, OUTPUT);
  pinMode(CH_E, OUTPUT);

  // Send Data to control register 11
  digitalWrite(OE, HIGH); // matrix reset
  digitalWrite(LAT, LOW);
  digitalWrite(CLK, LOW);
  for (int l = 0; l < MaxLed; l++)
  {
    int y = l % 16;
    digitalWrite(R1, LOW);
    digitalWrite(G1, LOW);
    digitalWrite(BL1, LOW);
    digitalWrite(R2, LOW);
    digitalWrite(G2, LOW);
    digitalWrite(BL2, LOW);
    if (C12[y] == 1)
    {
      digitalWrite(R1, HIGH);
      digitalWrite(G1, HIGH);
      digitalWrite(BL1, HIGH);
      digitalWrite(R2, HIGH);
      digitalWrite(G2, HIGH);
      digitalWrite(BL2, HIGH);
    }
    if (l > MaxLed - 12)
    {
      digitalWrite(LAT, HIGH);
    }
    else
    {
      digitalWrite(LAT, LOW);
    }
    digitalWrite(CLK, HIGH);
    digitalWrite(CLK, LOW);
  }
  digitalWrite(LAT, LOW);
  digitalWrite(CLK, LOW);
  // Send Data to control register 12
  for (int l = 0; l < MaxLed; l++)
  {
    int y = l % 16;
    digitalWrite(R1, LOW);
    digitalWrite(G1, LOW);
    digitalWrite(BL1, LOW);
    digitalWrite(R2, LOW);
    digitalWrite(G2, LOW);
    digitalWrite(BL2, LOW);
    if (C13[y] == 1)
    {
      digitalWrite(R1, HIGH);
      digitalWrite(G1, HIGH);
      digitalWrite(BL1, HIGH);
      digitalWrite(R2, HIGH);
      digitalWrite(G2, HIGH);
      digitalWrite(BL2, HIGH);
    }
    if (l > MaxLed - 13)
    {
      digitalWrite(LAT, HIGH);
    }
    else
    {
      digitalWrite(LAT, LOW);
    }
    digitalWrite(CLK, HIGH);
    digitalWrite(CLK, LOW);
  }
  digitalWrite(LAT, LOW);
  digitalWrite(CLK, LOW);
}

// End of default setup for RGB Matrix 64x32 panel

// Draw a line of image directly on the LED Matrix
void GIFDraw(GIFDRAW *pDraw)
{
  uint8_t *s;
  uint16_t *d, *usPalette, usTemp[320];
  int x, y, iWidth;

  iWidth = pDraw->iWidth;
  if (iWidth > MATRIX_WIDTH)
    iWidth = MATRIX_WIDTH;

  usPalette = pDraw->pPalette;
  y = pDraw->iY + pDraw->y; // current line

  s = pDraw->pPixels;
  if (pDraw->ucDisposalMethod == 2) // restore to background color
  {
    for (x = 0; x < iWidth; x++)
    {
      if (s[x] == pDraw->ucTransparent)
        s[x] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }
  // Apply the new pixels to the main image
  if (pDraw->ucHasTransparency) // if transparency used
  {
    uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
    int x, iCount;
    pEnd = s + pDraw->iWidth;
    x = 0;
    iCount = 0; // count non-transparent pixels
    while (x < pDraw->iWidth)
    {
      c = ucTransparent - 1;
      d = usTemp;
      while (c != ucTransparent && s < pEnd)
      {
        c = *s++;
        if (c == ucTransparent) // done, stop
        {
          s--; // back up to treat it like transparent
        }
        else // opaque
        {
          *d++ = usPalette[c];
          iCount++;
        }
      }           // while looking for opaque pixels
      if (iCount) // any opaque pixels?
      {
        for (int xOffset = 0; xOffset < iCount; xOffset++)
        {
          matrix.drawPixelRGB565(x + xOffset, y, usTemp[xOffset]);
        }
        x += iCount;
        iCount = 0;
      }
      // no, look for a run of transparent pixels
      c = ucTransparent;
      while (c == ucTransparent && s < pEnd)
      {
        c = *s++;
        if (c == ucTransparent)
          iCount++;
        else
          s--;
      }
      if (iCount)
      {
        x += iCount; // skip these
        iCount = 0;
      }
    }
  }
  else // does not have transparency
  {
    s = pDraw->pPixels;
    // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
    for (x = 0; x < pDraw->iWidth; x++)
    {
      matrix.drawPixelRGB565(x, y, usPalette[*s++]);
    }
  }
} /* GIFDraw() */

void *GIFOpenFile(char *fname, int32_t *pSize)
{
  f = FILESYSTEM.open(fname);
  if (f)
  {
    *pSize = f.size();
    return (void *)&f;
  }
  return NULL;
} /* GIFOpenFile() */

void GIFCloseFile(void *pHandle)
{
  File *f = static_cast<File *>(pHandle);
  if (f != NULL)
    f->close();
} /* GIFCloseFile() */

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
  int32_t iBytesRead;
  iBytesRead = iLen;
  File *f = static_cast<File *>(pFile->fHandle);
  // Note: If you read a file all the way to the last byte, seek() stops working
  if ((pFile->iSize - pFile->iPos) < iLen)
    iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
  if (iBytesRead <= 0)
    return 0;
  iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
  pFile->iPos = f->position();
  return iBytesRead;
} /* GIFReadFile() */

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
{
  int i = micros();
  File *f = static_cast<File *>(pFile->fHandle);
  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  i = micros() - i;
  Serial.printf("Seek time = %d us\n", i);
  return pFile->iPos;
} /* GIFSeekFile() */

unsigned long start_tick = 0;

void ShowGIF(char *name)
{
  start_tick = millis();

  if (gif.open(name, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw))
  {
    x_offset = (MATRIX_WIDTH - gif.getCanvasWidth()) / 2;
    if (x_offset < 0)
      x_offset = 0;
    y_offset = (MATRIX_HEIGHT - gif.getCanvasHeight()) / 2;
    if (y_offset < 0)
      y_offset = 0;
    Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
    Serial.flush();
    while (gif.playFrame(true, NULL))
    {
      ftpSrv.handleFTP(); //make sure in loop you call handleFTP()!!

      if ((millis() - start_tick) > 8000)
      { // we'll get bored after about 8 seconds of the same looping gif
        break;
      }
    }
    gif.close();
  }

} /* ShowGIF() */

void deleteFile(fs::FS &fs, const char *path)
{
  Serial.printf("Deleting file: %s\r\n", path);
  if (fs.remove(path))
  {
    Serial.println("- file deleted");
  }
  else
  {
    Serial.println("- delete failed");
  }
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}
/************************* Arduino Sketch Setup and Loop() *******************************/
void setup()
{

  Serial.begin(115200);
  // Start filesystem
  Serial.println(" * Loading SPIFFS");
  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS Mount Failed");
  }
  resetPanel(); // do this before matrix.begin
  Serial.println("...Starting matrix");
  matrix.setPanelBrightness(10);
  matrix.setMinRefreshRate(200);

  Serial.println("Starting AnimatedGIFs Sketch");

  matrix.begin(R1, G1, BL1, R2, G2, BL2, CH_A, CH_B, CH_C, CH_D, CH_E, LAT, OE, CLK);

  matrix.fillScreen(matrix.color565(0, 0, 0));
  matrix.drawPixelRGB565(0, 0, matrix.color565(64, 0, 0));
  gif.begin(LITTLE_ENDIAN_PIXELS);

  // start wifi
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // FTP Setup, ensure SPIFFS is started before ftp

  // esp32 we send true to format spiffs if cannot mount
  if (SPIFFS.begin(true))
  {

    Serial.println("SPIFFS opened!");
    ftpSrv.begin("admin", "esp32"); //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)
  }
}

void loop()
{
  // SPIFFS
  /*
  One important thing to consider about SPIFFS is that it does not support 
  directories [1][2]. Thus, it means that if we create a file with the path 
  “/dir/test.txt“, it will actually create a file with name “/dir/test.txt” 
  instead of a file called “test.txt” inside a “/dir” folder.

  So, when using the Arduino IDE SPIFFS plugin, if we create files inside 
  folders, they will be uploaded but their name will correspond to their 
  full path inside the sketch data folder.

  Another important point to consider is that the filename can only have 
  a maximum of 31 characters [3]. Thus, since the full path will be used as 
  the name of the file in the SPIFFS file system, we need to be careful to 
  avoid exceeding this limit. 
  */

  char szDir[] = "/"; // play all GIFs in this directory on the SD card
  char fname[256];
  File root, temp;

  listDir(SPIFFS, "/", 0);
  // listDir(SPIFFS, "/gifs", 0);
  // different methods of getting information
  Serial.print("Total bytes:    ");
  Serial.println(SPIFFS.totalBytes());
  Serial.print("Used bytes:     ");
  Serial.println(SPIFFS.usedBytes());
  Serial.print("Free bytes:     ");
  Serial.println(SPIFFS.totalBytes() - SPIFFS.usedBytes());

  while (1) // run forever
  {

    root = FILESYSTEM.open(szDir);
    if (root)
    {
      //Serial.println("inIf");
      //Serial.println(root.openNextFile());
      temp = root.openNextFile();
      Serial.println(temp);
      while (temp)
      {
        //Serial.println("inWhile");
        if (!temp.isDirectory()) // play it
        {
          // Serial.println("inIsDir");
          strcpy(fname, temp.name());

          Serial.printf("Playing %s\n", temp.name());
          Serial.flush();
          ShowGIF((char *)temp.name());
        }
        temp.close();
        temp = root.openNextFile();
      }
      root.close();
    } // root
    //delay(4000); // pause before restarting
  } // while
}
