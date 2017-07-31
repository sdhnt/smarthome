/*********************************************************
IDP Lab 5 Image processing
Usage of ILI9341 TFT LCD (www.adafruit.com/products/2090)
with TTL JPEG Camera (http://www.adafruit.com/product/397)

1. connect "Adafruit 2090" though SPI port
2. connect "Adafruit 397" though TX1/RX1
3. Load the Library "Adafruit_ILI9341.h" and "Adafruit_GFX.h" for LCD display
   *** all color is RGB565 color, 16 bit value for RGB
4. Load the Library "Adafruit_FT6206.h" for LCD Touch sensor
5. Load the Library "Adafruit_VC0706.h" for CMOS sensor
6. Load the Library "JPEGDecoder.h" for JPEG decode
7. Create user menu as Landscape display: use setRotation(3)
7.1 draw User keyboard by fillRect(x,y,width,height,color) comment
7.2 Take picture and display on LCD
*/

//=======================================================================================
#include <SPI.h>       // this is needed for display
#include <SD.h>
#include <Wire.h>    //I2C for touch
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // TFT by SPI library
#include <Adafruit_FT6206.h>  //touch
#include <Adafruit_VC0706.h>  //CMOS sensor
#include <JPEGDecoder.h>
#include <Servo.h>
Servo door;

#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

int getFingerprintIDez();

// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial3);

// On Leonardo/Micro or others with hardware serial, use those! #0 is green wire, #1 is white
//Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);

//---------------------------------------------------------------------------------------
//TFT display definition
#define SCREEN_Y  240
#define SCREEN_X  320
#define TFT_DC  49  //data/command pin of TFT
#define TFT_CS  53  //chip select pin of SPI
Adafruit_ILI9341 tft=Adafruit_ILI9341(TFT_CS,TFT_DC);  //object define
//---------------------------------------------------------------------------------------
// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ctp = Adafruit_FT6206();
#define IRQ_CTP 3       //use mega PWM3 pin for touchscreen IRQ
  int px,py;
  int ctpMODE = 0;
  int buttonKEY = 0;
//---------------------------------------------------------------------------------------
// Using hardware serial on Mega: camera TX conn. to RX1, camera RX to TX1, no Software Serial object is required:
#define SD_CS 48
#define LED 13
int int_jpegNo;  //jpg file name
//---------------------------------------------------------------------------------------
//Variable for BMP file
byte by_bmpdata[40*3];
const byte bmpheader1[18]={0x42,0x4D, 0x36,0x84,0x03,0, 0,0,0,0, 0x36,0,0,0,0x28,0,0,0}; //follow by width, height
const byte bmpheader2[28]={0x01,0,0x18,0,0,0,0,0,0x10,0,0,0,0x13,0x0b,0,0,0x13,0x0b,0,0,0,0,0,0,0,0,0,0};

//---------------------------------------------------------------------------------------
//Define button location and size
#define  Xc1 42
#define  Yc1 34

#define  Xc2 123
#define  Yc2 34

#define  Xc3 198
#define  Yc3 34

#define  Xc4 271
#define  Yc4 34

#define  Xc5 42
#define  Yc5 114

#define  Xc6 123
#define  Yc6 114

#define  Xc7 198
#define  Yc7 114

#define  Xc8 271
#define  Yc8 114

#define  Xc9 42
#define  Yc9 185

#define  Xc0 123
#define  Yc0 185

#define  Xcf 198
#define  Ycf 185

#define  Xcf1 271
#define  Ycf1 185



#define  BUTTON_SIZE 65
#define  BUTTON_RAD 10


//=======================================================================================
void setup(void) 
{
// put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);
  pinMode(IRQ_CTP,INPUT);
  attachInterrupt(digitalPinToInterrupt(IRQ_CTP), INT1_ctp, FALLING);
  

//-------------------------------------------------------------------------------------
  Serial.print("Initializing TFT screen...");
  tft.begin();         // tft screen setup
  tft.setRotation(3);  //landscape (wide) mode with 0,0 at the botton left corner
  tft.fillScreen(ILI9341_BLUE); 
    if (!ctp.begin(40))  // check touchscreen driver touchscreen, pass in 'sensitivity' coefficient
  {  
    Serial.print("Couldn't start FT6206 touchscreen controller"); return;
  }
  else
  {
    Serial.println(" start FT6206 touchscreen controller... Done");
  }
  
//-------------------------------------------------------------------------------------
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS)) // SD card detection
  {
    Serial.println(" failed!"); return;
  }
  else
  {
    Serial.println(" OK");
  }

//-------------------------------------------------------------------------------------
drawSdJpeg("main1.jpg",0,0);   // draw welcome screen. i.e. HKU logo by default
draw_grid();  
   door.attach(8);
  door.write(0);
  int_jpegNo=0;

while (!Serial);  // For Yun/Leo/Micro/Zero/...

  Serial.println("Adafruit finger detect test");

  // set the data rate for the sensor serial port
  //finger.begin(57600);
  
 // if (finger.verifyPassword()) {
 //   Serial.println("Found fingerprint sensor!");
 // } else {
 //   Serial.println("Did not find fingerprint sensor :(");
    //while (1);
 // }
 // Serial.println("Waiting for valid finger...");
}


//=======================================================================================
void INT1_ctp()  // touchscreen interrupt
{ 
    ctpMODE = 1;      //ctpMODE = 1, touchxcreen interrupted, do check key and so on...    
}
//=======================================================================================

  ///////////////////////////////////////////
int total=0;
int prevbuttonKEY=-1;
void loop() 
{
  // put your main code here, to run repeatedly:
 //getFingerprintIDez();

Serial.println("LOL");
  TS_Point p=ctp.getPoint();     
  px=p.y;  
  py=map(p.x,0,240,240,0); 

  if ((px > Xc1-BUTTON_SIZE/2) && (px < Xc1+BUTTON_SIZE/2) && (py> Yc1-BUTTON_SIZE/2) && (py < Yc1+BUTTON_SIZE/2)) 
  { buttonKEY = 1; 
    tft.fillRoundRect(Xc1-BUTTON_SIZE/2, Yc1-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_WHITE);
    //tft.fillCircle(Xc1,Yc1,15,ILI9341_RED);
    tft.setCursor(Xc1-10,Yc1-10);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(1);
  }
  else if ((px > Xc2-BUTTON_SIZE/2) && (px < Xc2+BUTTON_SIZE/2) && (py > Yc2-BUTTON_SIZE/2) && (py < Yc2+BUTTON_SIZE/2))
  {buttonKEY = 2;tft.fillRoundRect(Xc2-BUTTON_SIZE/2, Yc2-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_WHITE); 
    //tft.fillCircle(Xc2,Yc2,15,ILI9341_RED);
    tft.setCursor(Xc2-10,Yc2-10);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(2);}
  else if ((px > Xc3-BUTTON_SIZE/2) && (px < Xc3+BUTTON_SIZE/2) && (py > Yc3-BUTTON_SIZE/2) &&(py < Yc3+BUTTON_SIZE/2)) 
  {buttonKEY = 3;tft.fillRoundRect(Xc3-BUTTON_SIZE/2, Yc3-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_WHITE); 
    //tft.fillCircle(Xc3,Yc3,15,ILI9341_RED);
    tft.setCursor(Xc3-10,Yc3-10);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(3);}
  else if ((px > Xc4-BUTTON_SIZE/2) && (px < Xc4+BUTTON_SIZE/2) && (py > Yc4-BUTTON_SIZE/2) &&(py < Yc4+BUTTON_SIZE/2)) 
  {buttonKEY = 4;tft.fillRoundRect(Xc4-BUTTON_SIZE/2, Yc4-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_WHITE); 
    //tft.fillCircle(Xc4,Yc4,15,ILI9341_RED);
    tft.setCursor(Xc4-10,Yc4-10);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(4);}
  else if ((px > Xc5-BUTTON_SIZE/2) && (px < Xc5+BUTTON_SIZE/2) && (py > Yc5-BUTTON_SIZE/2) &&(py < Yc5+BUTTON_SIZE/2)) 
  {buttonKEY = 5;tft.fillRoundRect(Xc5-BUTTON_SIZE/2, Yc5-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_WHITE);
    //tft.fillCircle(Xc5,Yc5,15,ILI9341_RED);
    tft.setCursor(Xc5-10,Yc5-10);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(5);}
  else if ((px > Xc6-BUTTON_SIZE/2) && (px < Xc6+BUTTON_SIZE/2) && (py > Yc6-BUTTON_SIZE/2) &&(py < Yc6+BUTTON_SIZE/2)) 
  {buttonKEY = 6;tft.fillRoundRect(Xc6-BUTTON_SIZE/2, Yc6-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_WHITE);  
    //tft.fillCircle(Xc6,Yc6,15,ILI9341_RED);
    tft.setCursor(Xc6-10,Yc6-10);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(6);}
  else if ((px > Xc7-BUTTON_SIZE/2) && (px < Xc7+BUTTON_SIZE/2) && (py > Yc7-BUTTON_SIZE/2) &&(py < Yc7+BUTTON_SIZE/2)) 
  {buttonKEY = 7;tft.fillRoundRect(Xc7-BUTTON_SIZE/2, Yc7-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_WHITE); 
    //tft.fillCircle(Xc7,Yc7,15,ILI9341_RED);
    tft.setCursor(Xc7-10,Yc7-10);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(7);}
  else if ((px > Xc8-BUTTON_SIZE/2) && (px < Xc8+BUTTON_SIZE/2) && (py > Yc8-BUTTON_SIZE/2) &&(py < Yc8+BUTTON_SIZE/2)) 
  {buttonKEY = 8;tft.fillRoundRect(Xc8-BUTTON_SIZE/2, Yc8-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_WHITE);
    //tft.fillCircle(Xc8,Yc8,15,ILI9341_RED);
    tft.setCursor(Xc8-10,Yc8-10);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(8);}
  else if ((px > Xc9-BUTTON_SIZE/2) && (px < Xc9+BUTTON_SIZE/2) && (py > Yc9-BUTTON_SIZE/2) &&(py < Yc9+BUTTON_SIZE/2)) 
  {buttonKEY = 9;tft.fillRoundRect(Xc9-BUTTON_SIZE/2, Yc9-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_WHITE);
    //tft.fillCircle(Xc9,Yc9,15,ILI9341_RED);
    tft.setCursor(Xc9-10,Yc9-10);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(9);}
  else if ((px > Xc0-BUTTON_SIZE/2) && (px < Xc0+BUTTON_SIZE/2) && (py > Yc0-BUTTON_SIZE/2) &&(py < Yc0+BUTTON_SIZE/2)) 
  {buttonKEY = 0;tft.fillRoundRect(Xc0-BUTTON_SIZE/2, Yc0-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_WHITE); 
    tft.fillRoundRect(Xc0-BUTTON_SIZE/2, Yc0-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_WHITE);
    //tft.fillCircle(Xc0,Yc0,15,ILI9341_RED);
    tft.setCursor(Xc0-10,Yc0-10);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(0);}
  else if ((px > Xcf-BUTTON_SIZE/2) && (px < Xcf+BUTTON_SIZE/2) && (py > Ycf-BUTTON_SIZE/2) &&(py < Ycf+BUTTON_SIZE/2)) 
  {buttonKEY = 10;}
  
  else if ((px > Xcf1-BUTTON_SIZE/2) && (px < Xcf1+BUTTON_SIZE/2) && (py > Ycf1-BUTTON_SIZE/2) &&(py < Ycf1+BUTTON_SIZE/2)) {buttonKEY = 10;}
  else { buttonKEY = -1; } 

  
  if(buttonKEY==10 && total != 1234)
  {
    total=0; draw_grid();
  }
  else if(buttonKEY==10 && total == 1234)
  {
    drawSdJpeg("success.jpg",0,0);opendoor();drawSdJpeg("main1.jpg",0,0);closedoor();total=0; draw_grid();
  }
  else if((buttonKEY!=-1) && prevbuttonKEY!=buttonKEY && buttonKEY!=10 )
  {
    total=(total*10)+buttonKEY; Serial.println(total);
  }
  prevbuttonKEY=buttonKEY;
   Serial.println(total);
  
} 

//=======================================================================================
void bmpDraw(char *filename, uint8_t x, uint16_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  buffidx = sizeof(by_bmpdata); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft.width()) || (y >= tft.height())) return;

 // Serial.print(F("Loading image '"));
 // Serial.print(filename);
 // Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    //Serial.print(F("File not found"));
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    //Serial.print(F("File size: ")); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    //Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
   // Serial.print(F("Header size: ")); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
     // Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
       // Serial.print(F("Image size: "));
      //  Serial.print(bmpWidth);
      //  Serial.print('x');
      //  Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(by_bmpdata); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(by_bmpdata)) { // Indeed
              bmpFile.read(by_bmpdata, sizeof(by_bmpdata));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = by_bmpdata[buffidx++];
            g = by_bmpdata[buffidx++];
            r = by_bmpdata[buffidx++];
            tft.pushColor(tft.color565(r,g,b));
          } // end pixel
        } // end scanline
       // Serial.print(F("Loaded in "));
     //   Serial.print(millis() - startTime);
      //  Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  //if(!goodBmp) Serial.println(F("BMP format not recognized."));
}


//=======================================================================================
// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
//=======================================================================================




//#######################################################################################################################################################################################################################################################################################################################
//############################################################################################################################################################################################################################################################################################################################################################################################
// Draw a JPEG on the TFT pulled from SD Card
//####################################################################################################
// xpos, ypos is top left corner of plotted image
void drawSdJpeg(char *filename, int xpos, int ypos) {
  JpegDec.decodeFile(filename);
  renderJPEG(xpos, ypos);
}

//####################################################################################################
// Draw a JPEG on the TFT, images will be cropped on the right/bottom sides if they do not fit
//####################################################################################################
// This function assumes xpos,ypos is a valid screen coordinate. For convenience images that do not
// fit totally on the screen are cropped to the nearest MCU size and may leave right/bottom borders.
void renderJPEG(int xpos, int ypos) {



  uint16_t  *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;    // Width of MCU
  uint16_t mcu_h = JpegDec.MCUHeight;   // Height of MCU
  uint32_t mcu_pixels = mcu_w * mcu_h;  // Total number of pixels in an MCU


  // Fetch data from the file, decode and display
  while (JpegDec.read()) {    // While there is more data in the file
    pImg = JpegDec.pImage ;   // Decode a MCU (Minimum Coding Unit, typically a 8x8 or 16x16 pixel block)

    int mcu_x = JpegDec.MCUx * mcu_w + xpos;  // Calculate coordinates of top left corner of current MCU
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    if ((mcu_x + mcu_w) <= tft.width() && (mcu_y + mcu_h) <= tft.height())
    {
      // Now set a MCU bounding window on the TFT to push pixels into (x, y, x + width - 1, y + height - 1)
      tft.setAddrWindow(mcu_x, mcu_y, mcu_x + mcu_w - 1, mcu_y + mcu_h - 1);

      // Push all MCU pixels to the TFT window
      uint32_t count = mcu_pixels;
      while (count--) {
        // Push each pixel to the TFT MCU area
        tft.pushColor(*pImg++);
      }

      // Push all MCU pixels to the TFT window, ~18% faster to pass an array pointer and length to the library
      // tft.pushColor16(pImg, mcu_pixels); //  To be supported in HX8357 library at a future date

    }
    else if ((mcu_y + mcu_h) >= tft.height()) JpegDec.abort(); // Image has run off bottom of screen so abort decoding
  }


}

//####################################################################################################
// Print image information to the serial port (optional)
//####################################################################################################
// JpegDec.decodeFile(...) or JpegDec.decodeArray(...) must be called before this info is available!


//####################################################################################################
// Show the execution time (optional)
//####################################################################################################
// WARNING: for UNO/AVR legacy reasons printing text to the screen with the Mega might not work for
// sketch sizes greater than ~70KBytes because 16 bit address pointers are used in some libraries.

// The Due will work fine with the HX8357_Due library.



//####################################################################################################
// Pull a jpeg file off the SD Card and send it as a "C" formatted as an array to the serial port
//####################################################################################################
// The array can be cut and pasted from the Serial Monitor window into jpegX.h attached to this sketch
void createArray(const char *filename) {
  File jpgFile;
  uint8_t  sdbuffer[32];   // SD read pixel buffer (16 bits per pixel)

  // Check file exists and open it
  if ((jpgFile = SD.open(filename)) == NULL) {

    return;
  }

  uint8_t data;
  byte line_len = 0;

  Serial.print("const uint8_t ");
  // Make the array the same as the file name with the .(file extension) removed
  while (*filename != '.') Serial.print(*filename++);

  Serial.println("[] PROGMEM = {"); // PROGMEM added for AVR processors, it is ignored by Due
  // Pull all data falues from file and print in the array format
  while (jpgFile.available()) {
    data = jpgFile.read();
    Serial.print("0x");                        // Add hexadecimal prefix
    if (abs(data) < 16) Serial.print("0");     // Add a leading zero to create a neater array
    Serial.print(data, HEX); Serial.print(",");// Add value and comma
    line_len++;
    // Add a newline every 32 bytes
    if (line_len >= 32) {
      line_len = 0;
      Serial.println();
    }
  }
  Serial.println("};");
  Serial.println();
  // close the file:
  jpgFile.close();
}


// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  openandclosedoor();
  return finger.fingerID; 
  
}

void openandclosedoor()
{
    for(int i=0; i< 120;i++)
  {
    door.write(i); delay(20);
  }
  for(int i=120; i>0;i--)
  {
    door.write(i); delay(20);
  }

}
void opendoor()
{
    for(int i=0; i< 120;i++)
  {
    door.write(i); delay(20);
  }
  
}
void closedoor()
{

  for(int i=120; i>0;i--)
  {
    door.write(i); delay(20);
  }

}
void draw_grid()
{
    tft.fillRoundRect(Xc1-BUTTON_SIZE/2, Yc1-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_BLACK);
    //tft.fillCircle(Xc1,Yc1,15,ILI9341_RED);
    tft.setCursor(Xc1-10,Yc1-10);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(1);
    
    tft.fillRoundRect(Xc2-BUTTON_SIZE/2, Yc2-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_BLACK);
    //tft.fillCircle(Xc2,Yc2,15,ILI9341_RED);
    tft.setCursor(Xc2-10,Yc2-10);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(2);
    tft.fillRoundRect(Xc3-BUTTON_SIZE/2, Yc3-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_BLACK);
    //tft.fillCircle(Xc3,Yc3,15,ILI9341_RED);
    tft.setCursor(Xc3-10,Yc3-10);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(3);
    tft.fillRoundRect(Xc4-BUTTON_SIZE/2, Yc4-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_BLACK);
    //tft.fillCircle(Xc4,Yc4,15,ILI9341_RED);
    tft.setCursor(Xc4-10,Yc4-10);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(4);
    tft.fillRoundRect(Xc5-BUTTON_SIZE/2, Yc5-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_BLACK);
    //tft.fillCircle(Xc5,Yc5,15,ILI9341_RED);
    tft.setCursor(Xc5-10,Yc5-10);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(5);tft.fillRoundRect(Xc6-BUTTON_SIZE/2, Yc6-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_BLACK);
    //tft.fillCircle(Xc6,Yc6,15,ILI9341_RED);
    tft.setCursor(Xc6-10,Yc6-10);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(6);
    tft.fillRoundRect(Xc7-BUTTON_SIZE/2, Yc7-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_BLACK);
    //tft.fillCircle(Xc7,Yc7,15,ILI9341_RED);
    tft.setCursor(Xc7-10,Yc7-10);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(7);
    tft.fillRoundRect(Xc8-BUTTON_SIZE/2, Yc8-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_BLACK);
    //tft.fillCircle(Xc8,Yc8,15,ILI9341_RED);
    tft.setCursor(Xc8-10,Yc8-10);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(8);
    tft.fillRoundRect(Xc9-BUTTON_SIZE/2, Yc9-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_BLACK);
    //tft.fillCircle(Xc9,Yc9,15,ILI9341_RED);
    tft.setCursor(Xc9-10,Yc9-10);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(9);
    tft.fillRoundRect(Xc0-BUTTON_SIZE/2, Yc0-BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE, BUTTON_RAD, ILI9341_BLACK);
    //tft.fillCircle(Xc0,Yc0,15,ILI9341_RED);
    tft.setCursor(Xc0-10,Yc0-10);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(4);
    tft.setTextWrap(false);
    tft.println(0);




}  


