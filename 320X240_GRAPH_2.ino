/* VER2*/

/*
 * TFT     ESP
 * VCC    = 3V3
 * GND    = GND
 * LED    = 3V3
 * MOSI   = IO23
 * SCK    = IO18
 * CS     = IO15(TD0)
 * RESET  = IO04
 * DC     = IO02
 * T_DO   = IO19
 * T_DIN  = IO23 (MOSI on FTF)
 * T_CS   = IO21
 * T_CLK  = IO18 (SCK on TFT)
 *
 * Board ESP32 D1 MINI : DOIT ESP32 DEVKIT V1
 * https://dl.espressif.com/dl/package_esp32_index.json
 * TFT_eSPI

*/

/*
*Used Arduino/libraries/TFT_eSPI/User_Setup.h:
*
* #define USER_SETUP_INFO "User_Setup"
*
* #define ILI9341_DRIVER       // Generic driver for common displays
*
* #define TFT_MISO 19
* #define TFT_MOSI 23
* #define TFT_SCLK 18
* #define TFT_CS   15  // Chip select control pin
* #define TFT_DC    2  // Data Command control pin
* #define TFT_RST   4  // Reset pin (could connect to RST pin)
*
* #define TOUCH_CS 21     // Chip select pin (T_CS) of touch screen
*
* #define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
* #define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
* #define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
* #define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
* #define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
* #define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
* #define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts
* #define SMOOTH_FONT
*
* #define SPI_FREQUENCY  27000000
* #define SPI_READ_FREQUENCY  20000000
* #define SPI_TOUCH_FREQUENCY  2500000
*
*/

#include "FS.h"       // Calibration data is stored in SPIFFS so we need to include it

#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

TFT_eSprite graph1 = TFT_eSprite(&tft); // Sprite object graph1
TFT_eSprite stext1 = TFT_eSprite(&tft); // Sprite object stext2

// This is the file name used to store the touch coordinate
// calibration data. Cahnge the name to start a new calibration.
#define CALIBRATION_FILE "/TouchCalData3"

#define DATA_FILE "/data_prog0"
//#define DATA_FILE_1 "/data_prog1"
//#define DATA_FILE_2 "/data_prog2"
//#define DATA_FILE_3 "/data_prog3"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false
bool TOUCH_CALIBRATE = false;

// Width & hifgt LCD
#define WIDTH 319
#define HIGHT 239

// Buttons size
#define BUTTON_W WIDTH
#define BUTTON_H 40
#define RADIUS 0

// Buttons size
#define BTN_LS_W WIDTH/3
#define BTN_LS_H 40

// Buttons size in program for on_off
#define BTN_PRG_W 80
#define BTN_PRG_H 40

// Buttons INT
#define BTN_INT_W 44
#define BTN_INT_X 176
  
// ButtonS on/off X
#define BTN_PRG_P_X 235

// Button1 on/off Y
#define BTN1_PRG_PM_Y 4

// Button2 on/off Y
#define BTN2_PRG_PM_Y 52 //38+20

// Button3 on/off Y
#define BTN3_PRG_PM_Y 100 //38*2+20

// Button4 on/off Y
#define BTN4_PRG_PM_Y 148 //38*3+20 

// Button1 zone size
#define BUTTON1_X 0
#define BUTTON1_Y 4

// Button2 zone size
#define BUTTON2_X 0
#define BUTTON2_Y 52 //48+4

// Button3 zone size
#define BUTTON3_X 0
#define BUTTON3_Y 100 // 48*2+4

// Button4 zone size
#define BUTTON4_X 0
#define BUTTON4_Y 148 // 48*3+4

// Button5 zone size
#define BUTTON5_X 0
#define BUTTON5_Y 196 // 48*4+4

// List of programs FRAME
#define LIST_LS_W 200
#define LIST_LS_H 180
#define LIST_LS_X 118
#define LIST_LS_Y 2

// KEYBOARD_FRAME
#define KEYBOARD_FRAME_W 316
#define KEYBOARD_FRAME_H 236
#define KEYBOARD_FRAME_X 2
#define KEYBOARD_FRAME_Y 2
// KEYBOARD BUTTON
#define KEY_SPACE 0
#define KEY_W (KEYBOARD_FRAME_W/4) //-(KEY_SPACE*5)
#define KEY_H (KEYBOARD_FRAME_H/3) //-(KEY_SPACE*4)


// Graph zone size
#define GRAPH_X 51
#define GRAPH_Y 309

int offs = 48;

#define GREY_SPOT    // Comment out to stop drawing black spots

unsigned long out1prevMs = 0;
unsigned long out2prevMs = 0;

bool out1 = 0;
bool out2 = 0;
int out1_line = 44; // Yelow
int out2_line = 48; // Blue
int grid = 0;
int tcount = 0;
unsigned long previousMillis = 0;
unsigned long timer = 0;

bool start_prog = 0;

bool in_menu = 1;
bool in_settings = 0;
bool in_program = 0;
byte in_keyboard = 0; //0=keyboard off and 1,2,3,4=keyboard for io
bool in_load_save = 0;

String input_keyboard = "";

long io1 = 1000;         //ms
long io2 = 1000;         //ms
long io3 = 1000;         //ms
long io4 = 1000;         //ms

//==========================================================================================
void setup() {

  Serial.begin(115200);
  
  tft.init();
    
  // Set the rotation before we calibrate
  tft.setRotation(3);
  // call screen calibration
  touch_calibrate();

  // clear screen
  tft.fillScreen(TFT_BLACK);

  // Draw button (this example does not use library Button class)
  menuButtons();

  // Create a sprite for the graph
  graph1.setColorDepth(8);
  graph1.createSprite(GRAPH_Y, GRAPH_X);
  graph1.fillSprite(TFT_BLACK); // Note: Sprite is filled with black when created
  //tft.drawRect(0, 0, WIDTH, HIGHT, TFT_CYAN);
  tft.drawRect(4, HIGHT-GRAPH_X-6, WIDTH-8,GRAPH_X+2, TFT_DARKGREY);

  // Create a sprite
  stext1.setColorDepth(8);
  stext1.createSprite(WIDTH-7, 7);
  stext1.fillSprite(TFT_BLACK);
  stext1.setScrollRect(0, 0, WIDTH-7, 7, TFT_BLACK); // Scroll the text
  stext1.setTextColor(TFT_WHITE); // White text, no background

  if (SPIFFS.begin()){
    tft.setTextColor(TFT_GREEN,TFT_BLUE);
    tft.drawCentreString("SPIFFS mounted correctly.", 160, 100,2);
  }
  else{
    tft.setTextColor(TFT_GREEN,TFT_BLUE);
    tft.drawCentreString("!An error occurred during SPIFFS mounting", 160, 100,2);
  }
  //SPIFFS.remove("/TouchCalData1");
  Serial.println("[+] Run");
  
  ls();
}


//==========================================================================================
// SHOW Lvl 1 MAIN MENU
//==========================================================================================
void menuButtons(){
  
  tft.fillScreen(TFT_BLACK);                                                                // clear screen
  tft.drawRect(4, HIGHT-GRAPH_X-6, WIDTH-8,GRAPH_X+2, TFT_DARKGREY);                        // Create rectange for graph

  tft.setTextColor(TFT_SKYBLUE);
  
  String strBtn1 = "Start";
  tft.fillRect(BUTTON1_X, BUTTON1_Y, BUTTON_W, BUTTON_H, TFT_BLACK);                          // BACKGROUND BTN
  tft.drawRoundRect(BUTTON1_X, BUTTON1_Y-2, 41, BUTTON_H+4, RADIUS, TFT_DARKCYAN);            //OUT FRAME LEFT
  tft.fillRect(BUTTON1_X+2, BUTTON1_Y, 5, BUTTON_H, TFT_DARKCYAN);                            //OUT FRAME LEFT
  tft.drawRoundRect(BUTTON1_X+278, BUTTON1_Y+2, 37, BUTTON_H-4, RADIUS, TFT_DARKCYAN);        //IN FRAME RIGHT
  tft.fillRect(BUTTON1_X+310, BUTTON1_Y+2, 5, BUTTON_H-4, TFT_DARKCYAN);                      //IN FRAME RIGHT
  tft.drawRect(BUTTON1_X+40, BUTTON1_Y-2, BUTTON_W-80, BUTTON_H+4, TFT_BLACK);                //out
  tft.drawRoundRect(BUTTON1_X+2, BUTTON1_Y, BUTTON_W-4, BUTTON_H, RADIUS, TFT_DARKCYAN);      // FRAME BTN
  tft.drawCentreString(strBtn1, BUTTON1_X + (BUTTON_W / 2), BUTTON1_Y + (BUTTON_H/2-10),4);

  String strBtn2 = "Settings";
  tft.fillRect(BUTTON2_X, BUTTON2_Y, BUTTON_W, BUTTON_H, TFT_BLACK);                          // BACKGROUND BTN
  tft.drawRoundRect(BUTTON2_X, BUTTON2_Y-2, 41, BUTTON_H+4, RADIUS, TFT_DARKCYAN);            //OUT FRAME LEFT
  tft.fillRect(BUTTON2_X+2, BUTTON2_Y, 5, BUTTON_H, TFT_DARKCYAN);                            //OUT FRAME LEFT
  tft.drawRoundRect(BUTTON2_X+278, BUTTON2_Y+2, 37, BUTTON_H-4, RADIUS, TFT_DARKCYAN);        //IN FRAME RIGHT
  tft.fillRect(BUTTON2_X+310, BUTTON2_Y+2, 5, BUTTON_H-4, TFT_DARKCYAN);                      //IN FRAME RIGHT
  tft.drawRect(BUTTON2_X+40, BUTTON2_Y-2, BUTTON_W-80, BUTTON_H+4, TFT_BLACK);                //out
  tft.drawRoundRect(BUTTON2_X+2, BUTTON2_Y, BUTTON_W-4, BUTTON_H, RADIUS, TFT_DARKCYAN);      // FRAME BTN
  tft.drawCentreString(strBtn2, BUTTON2_X + (BUTTON_W / 2), BUTTON2_Y + (BUTTON_H/2-10),4);
  /*String strBtn2 = "Settings";
  tft.fillRect(BUTTON2_X, BUTTON2_Y, BUTTON_W, BUTTON_H, TFT_BLACK);
  tft.drawRoundRect(BUTTON2_X, BUTTON2_Y-2, BUTTON_W, BUTTON_H+4, RADIUS, TFT_DARKCYAN);    //out
  tft.drawRect(BUTTON2_X+40, BUTTON2_Y-2, BUTTON_W-80, BUTTON_H+4, TFT_BLACK); //out
  tft.drawRoundRect(BUTTON2_X+2, BUTTON2_Y, BUTTON_W-4, BUTTON_H, RADIUS, TFT_DARKCYAN);    //in
  tft.drawCentreString(strBtn2, BUTTON2_X + (BUTTON_W / 2), BUTTON2_Y + (BUTTON_H/2-10),4);*/

  // Blend from white to black (32 grey levels)
  /*for (uint16_t a = 0; a < 255; a++) // Alpha 0 = 100% background, alpha 255 = 100% foreground
  {
    tft.drawFastVLine(a+2, BUTTON1_Y, BUTTON_H, tft.alphaBlend(a, TFT_BLACK, TFT_WHITE));
    tft.drawFastVLine(a+2, BUTTON2_Y, BUTTON_H, tft.alphaBlend(a, TFT_BLACK, TFT_WHITE));
  }*/

  tft.setTextDatum(CC_DATUM);                                                               // Center, center enumerate the text plotting alignment
}


//==========================================================================================
// SHOW Lvl 2 SETTINGS MENU
//==========================================================================================
void settingsButtons(){

  // clear screen
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_SKYBLUE);

  String strBtn1 = "Program";
  tft.fillRect(BUTTON1_X, BUTTON1_Y, BUTTON_W, BUTTON_H, TFT_BLACK);                          // BACKGROUND BTN
  tft.drawRoundRect(BUTTON1_X, BUTTON1_Y-2, 41, BUTTON_H+4, RADIUS, TFT_DARKCYAN);            //OUT FRAME LEFT
  tft.fillRect(BUTTON1_X+2, BUTTON1_Y, 5, BUTTON_H, TFT_DARKCYAN);                            //OUT FRAME LEFT
  tft.drawRoundRect(BUTTON1_X+278, BUTTON1_Y+2, 37, BUTTON_H-4, RADIUS, TFT_DARKCYAN);        //IN FRAME RIGHT
  tft.fillRect(BUTTON1_X+310, BUTTON1_Y+2, 5, BUTTON_H-4, TFT_DARKCYAN);                      //IN FRAME RIGHT
  tft.drawRect(BUTTON1_X+40, BUTTON1_Y-2, BUTTON_W-80, BUTTON_H+4, TFT_BLACK);                //out
  tft.drawRoundRect(BUTTON1_X+2, BUTTON1_Y, BUTTON_W-4, BUTTON_H, RADIUS, TFT_DARKCYAN);      // FRAME BTN
  tft.drawCentreString(strBtn1, BUTTON1_X + (BUTTON_W / 2), BUTTON1_Y + (BUTTON_H/2-10),4);

  String strBtn2 = "Load & Save";
  tft.fillRect(BUTTON2_X, BUTTON2_Y, BUTTON_W, BUTTON_H, TFT_BLACK);                          // BACKGROUND BTN
  tft.drawRoundRect(BUTTON2_X, BUTTON2_Y-2, 41, BUTTON_H+4, RADIUS, TFT_DARKCYAN);            //OUT FRAME LEFT
  tft.fillRect(BUTTON2_X+2, BUTTON2_Y, 5, BUTTON_H, TFT_DARKCYAN);                            //OUT FRAME LEFT
  tft.drawRoundRect(BUTTON2_X+278, BUTTON2_Y+2, 37, BUTTON_H-4, RADIUS, TFT_DARKCYAN);        //IN FRAME RIGHT
  tft.fillRect(BUTTON2_X+310, BUTTON2_Y+2, 5, BUTTON_H-4, TFT_DARKCYAN);                      //IN FRAME RIGHT
  tft.drawRect(BUTTON2_X+40, BUTTON2_Y-2, BUTTON_W-80, BUTTON_H+4, TFT_BLACK);                //out
  tft.drawRoundRect(BUTTON2_X+2, BUTTON2_Y, BUTTON_W-4, BUTTON_H, RADIUS, TFT_DARKCYAN);      // FRAME BTN
  tft.drawCentreString(strBtn2, BUTTON2_X + (BUTTON_W / 2), BUTTON2_Y + (BUTTON_H/2-10),4);

  String strBtn3 = "Touch calibrate";
  tft.fillRect(BUTTON3_X, BUTTON3_Y, BUTTON_W, BUTTON_H, TFT_BLACK);                          // BACKGROUND BTN
  tft.drawRoundRect(BUTTON3_X, BUTTON3_Y-2, 41, BUTTON_H+4, RADIUS, TFT_DARKCYAN);            //OUT FRAME LEFT
  tft.fillRect(BUTTON3_X+2, BUTTON3_Y, 5, BUTTON_H, TFT_DARKCYAN);                            //OUT FRAME LEFT
  tft.drawRoundRect(BUTTON3_X+278, BUTTON3_Y+2, 37, BUTTON_H-4, RADIUS, TFT_DARKCYAN);        //IN FRAME RIGHT
  tft.fillRect(BUTTON3_X+310, BUTTON3_Y+2, 5, BUTTON_H-4, TFT_DARKCYAN);                      //IN FRAME RIGHT
  tft.drawRect(BUTTON3_X+40, BUTTON3_Y-2, BUTTON_W-80, BUTTON_H+4, TFT_BLACK);                //out
  tft.drawRoundRect(BUTTON3_X+2, BUTTON3_Y, BUTTON_W-4, BUTTON_H, RADIUS, TFT_DARKCYAN);      // FRAME BTN
  tft.drawCentreString(strBtn3, BUTTON3_X + (BUTTON_W / 2), BUTTON3_Y + (BUTTON_H/2-10),4);

  String strBtn4 = "";
  tft.fillRect(BUTTON4_X, BUTTON4_Y, BUTTON_W, BUTTON_H, TFT_BLACK);                          // BACKGROUND BTN
  tft.drawRoundRect(BUTTON4_X, BUTTON4_Y-2, 41, BUTTON_H+4, RADIUS, TFT_BLACK);            //OUT FRAME LEFT
  tft.fillRect(BUTTON4_X+2, BUTTON4_Y, 5, BUTTON_H, TFT_BLACK);                            //OUT FRAME LEFT
  tft.drawRoundRect(BUTTON4_X+278, BUTTON4_Y+2, 37, BUTTON_H-4, RADIUS, TFT_BLACK);        //IN FRAME RIGHT
  tft.fillRect(BUTTON4_X+310, BUTTON4_Y+2, 5, BUTTON_H-4, TFT_BLACK);                      //IN FRAME RIGHT
  tft.drawRect(BUTTON4_X+40, BUTTON4_Y-2, BUTTON_W-80, BUTTON_H+4, TFT_BLACK);                //out
  tft.drawRoundRect(BUTTON4_X+2, BUTTON4_Y, BUTTON_W-4, BUTTON_H, RADIUS, TFT_BLACK);      // FRAME BTN
  tft.drawCentreString(strBtn4, BUTTON4_X + (BUTTON_W / 2), BUTTON4_Y + (BUTTON_H/2-10),4);

  String strBtn5 = "Exit";
  tft.fillRect(BUTTON5_X, BUTTON5_Y, BUTTON_W, BUTTON_H, TFT_BLACK);                          // BACKGROUND BTN
  tft.drawRoundRect(BUTTON5_X, BUTTON5_Y-2, 41, BUTTON_H+4, RADIUS, TFT_DARKCYAN);            //OUT FRAME LEFT
  tft.fillRect(BUTTON5_X+2, BUTTON5_Y, 5, BUTTON_H, TFT_DARKCYAN);                            //OUT FRAME LEFT
  tft.drawRoundRect(BUTTON5_X+278, BUTTON5_Y+2, 37, BUTTON_H-4, RADIUS, TFT_DARKCYAN);        //IN FRAME RIGHT
  tft.fillRect(BUTTON5_X+310, BUTTON5_Y+2, 5, BUTTON_H-4, TFT_DARKCYAN);                      //IN FRAME RIGHT
  tft.drawRect(BUTTON5_X+40, BUTTON5_Y-2, BUTTON_W-80, BUTTON_H+4, TFT_BLACK);                //out
  tft.drawRoundRect(BUTTON5_X+2, BUTTON5_Y, BUTTON_W-4, BUTTON_H, RADIUS, TFT_DARKCYAN);      // FRAME BTN
  tft.drawCentreString(strBtn5, BUTTON5_X + (BUTTON_W / 2), BUTTON5_Y + (BUTTON_H/2-10),4);

  tft.setTextDatum(CC_DATUM); // Center, center enumerate the text plotting alignment
}


//==========================================================================================
// RUN & SHOW graph in MAIN MENU
//==========================================================================================
void run_and_graph() {
  unsigned long currentMillis = millis();

  /* IO1 CASOVACE io1 & io2*/
  if ( (io1 > 0) && (io2 > 0) ){
    if ( (out1 == 1) && (currentMillis - out1prevMs >= io1) ){
      out1prevMs = currentMillis;
      out1 = 0;
      graph1.drawFastVLine(GRAPH_Y-1,4,44,TFT_YELLOW); // draw 2 pixel point on graph
      graph1.drawFastVLine(GRAPH_Y-2,4,44,TFT_YELLOW); // draw 2 pixel point on graph
    }
    else if ( (out1 == 0) && (currentMillis - out1prevMs >= io2) ){
      out1prevMs = currentMillis;
      out1 = 1;
      graph1.drawFastVLine(GRAPH_Y-1,4,44,TFT_YELLOW); // draw vertical line on graph
      graph1.drawFastVLine(GRAPH_Y-2,4,44,TFT_YELLOW); // draw vertical line on graph
    }
  }
  else if ( (io1 == 0) && (io2 == 0) ){   //stale vypnute
    out1 = 0;
  }
  else if ( (io1 > 0) && (io2 == 0) ){    //stale zapnute
    out1 = 1;
  }

  /* IO2 CASOVACE io3 & io4*/
  if ( (io3 > 0) && (io4 > 0) ){
    if ( (out2 == 1) && (currentMillis - out2prevMs >= io3) ){
      out2prevMs = currentMillis;
      out2 = 0;
      graph1.drawFastVLine(GRAPH_Y-1,0,48,TFT_RED); // draw 2 pixel point on graph
    }
    else if ( (out2 == 0) && (currentMillis - out2prevMs >= io4) ){
      out2prevMs = currentMillis;
      out2 = 1;
      graph1.drawFastVLine(GRAPH_Y-1,0,48,TFT_RED);; // draw vertical line on graph
    }
  }
  else if ( (io3 == 0) && (io4 == 0) ){   //stale vypnute
    out2 = 0;
  }
  else if ( (io3 > 0) && (io4 == 0) ){    //stale zapnute
    out2 = 1;
  }
  

  if (out1 == 0){
    out1_line = 44;
  }
  else{
    out1_line = 4;
  }

  if (out2 == 0){
    out2_line = 48;
  }
  else{
    out2_line = 0;
  }
  
  // Draw point in graph1 sprite at far right edge (this will scroll left later)
  graph1.drawFastVLine(GRAPH_Y-1,out1_line,4,TFT_YELLOW); // draw 2 pixel point on graph
  graph1.drawFastVLine(GRAPH_Y-1,out2_line,4,TFT_RED); // draw 2 pixel point on graph

  // Push the sprites onto the TFT at specied coordinates
  graph1.pushSprite(5, HIGHT-GRAPH_X-5); // pozicia 5px od lava, 239-60-5px od zhora
  stext1.pushSprite(4, HIGHT-GRAPH_X-15); // pozicia 4px od lava, 172px od zhora

  // Scroll the sprites scroll(dt, dy)
  graph1.scroll(-1, 0); // scroll graph 1 pixel left, 0 up/down
  stext1.scroll(-1, 0);     // scroll stext 1 pixel right, up/down default is 0


  // GRID
  // Draw the grid on far right edge of sprite as graph has now moved 1 pixel left
  grid++;
  if (grid >= 10)
  { // Draw a vertical line if we have scrolled 10 times (10 pixels)
    grid = 0;
    graph1.drawFastVLine(GRAPH_Y-1, 0, GRAPH_X, TFT_DARKGREY); // draw line on graph
  }
  else
  { // Otherwise draw points spaced 10 pixels for the horizontal grid lines
    for (int p = 0; p <= GRAPH_X-1; p += 10){
      if (p != 0){
        graph1.drawPixel(GRAPH_Y-1, p, TFT_DARKGREY);
      }
    }
  }

  // SHOW 1s
  if (currentMillis - previousMillis >= 996) { // 1second = 996
    stext1.drawString("1s", WIDTH-20, 0, 1); // draw at 6,0 in sprite, font 2
    previousMillis = currentMillis;
    timer++;

    Serial.print("out1: ");
    Serial.print(out1);
    Serial.print(" out2: ");
    Serial.println(out2); 
  }
  
  tft.setTextColor(TFT_GREEN,TFT_BLUE);
  tft.drawCentreString("Runtime: (msecs)", 160, 100,2);
  tft.setTextDatum(TC_DATUM);
  tft.drawNumber(timer, 160, 130,2);

  
  
}

//==========================================================================================
// SHOW in PROGRAM MENU
//==========================================================================================
void program(){

  in_keyboard = 0;
  in_program = 1;
  in_load_save = 0;
  in_settings = 0;
  in_menu = 0;
  
  // clear screen
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_SKYBLUE);

  // SIGNAL IO1 ------------------------------------
  // Create labels and on\off buttons
  tft.drawRect(BTN_PRG_P_X, BTN1_PRG_PM_Y, BTN_PRG_W, BTN_PRG_H, TFT_DARKCYAN);
  tft.drawCentreString("+", BTN_PRG_P_X+(BTN_PRG_W/2), BTN1_PRG_PM_Y + (BTN_PRG_H/2-10), 4);

  tft.setTextColor(TFT_YELLOW);
  tft.drawCentreString(" IO1 signal 1", BTN_PRG_W+25, BTN1_PRG_PM_Y + (BTN_PRG_H/2-10), 4);
  tft.setTextColor(TFT_SKYBLUE);
  tft.drawFloat(float(io1)/1000, 1, 200, 26+offs*0, 4);

  // SIGNAL IO2 ------------------------------------
  // Create labels and on\off buttons
  tft.drawRect(BTN_PRG_P_X, BTN2_PRG_PM_Y, BTN_PRG_W, BTN_PRG_H, TFT_DARKCYAN);
  tft.drawCentreString("+", BTN_PRG_P_X+(BTN_PRG_W/2), BTN2_PRG_PM_Y + (BTN_PRG_H/2-10), 4);

  tft.setTextColor(TFT_YELLOW);
  tft.drawCentreString(" IO1 signal 2", BTN_PRG_W+25, BTN2_PRG_PM_Y + (BTN_PRG_H/2-10), 4);
  tft.setTextColor(TFT_SKYBLUE);
  tft.drawFloat(float(io2)/1000, 1, 200, 26+offs*1, 4);

  // SIGNAL IO3 ------------------------------------
  // Create labels and on\off buttons
  tft.drawRect(BTN_PRG_P_X, BTN3_PRG_PM_Y, BTN_PRG_W, BTN_PRG_H, TFT_DARKCYAN);
  tft.drawCentreString("+", BTN_PRG_P_X+(BTN_PRG_W/2), BTN3_PRG_PM_Y + (BTN_PRG_H/2-10), 4);

  tft.setTextColor(TFT_RED);
  tft.drawCentreString(" IO2 signal 1", BTN_PRG_W+25, BTN3_PRG_PM_Y + (BTN_PRG_H/2-10), 4);
  tft.setTextColor(TFT_SKYBLUE);
  tft.drawFloat(float(io3)/1000, 1, 200, 26+offs*2, 4);
  // SIGNAL IO4 ------------------------------------
  // Create labels and on\off buttons
  tft.drawRect(BTN_PRG_P_X, BTN4_PRG_PM_Y, BTN_PRG_W, BTN_PRG_H, TFT_DARKCYAN); // BTN1_PRG_X 235,8
  tft.drawCentreString("+", BTN_PRG_P_X+(BTN_PRG_W/2), BTN4_PRG_PM_Y + (BTN_PRG_H/2-10), 4);

  tft.setTextColor(TFT_RED);
  tft.drawCentreString(" IO2 signal 2", BTN_PRG_W+25, BTN4_PRG_PM_Y + (BTN_PRG_H/2-10), 4);
  tft.setTextColor(TFT_SKYBLUE);
  tft.drawFloat(float(io4)/1000, 1, 200, 26+offs*3, 4);

  // Create exit button on the end
  String strBtn5 = "Exit";
  tft.fillRect(BUTTON5_X, BUTTON5_Y, BUTTON_W, BUTTON_H, TFT_BLACK);
  tft.drawRoundRect(BUTTON5_X, BUTTON5_Y-2, BUTTON_W, BUTTON_H+4, RADIUS, TFT_DARKCYAN);    //out frame
  tft.drawRect(BUTTON5_X+40, BUTTON5_Y-2, BUTTON_W-80, BUTTON_H+4, TFT_BLACK);              //out frame for cuting 
  tft.drawRoundRect(BUTTON5_X+2, BUTTON5_Y, BUTTON_W-4, BUTTON_H, RADIUS, TFT_DARKCYAN);    //in frame
  tft.drawCentreString(strBtn5, BUTTON5_X + (BUTTON_W / 2), BUTTON5_Y + (BUTTON_H/2-10),4);

  tft.setTextDatum(CC_DATUM); // Center, center enumerate the text plotting alignment
}

//==========================================================================================
// SHOW in LOAD & SAVE MENU
//==========================================================================================
void load_save(){
  in_load_save = 1;
  in_program = 0;
  in_settings = 0;
  in_menu = 0;

  // clear screen
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_SKYBLUE);

  // List of programs
  tft.drawRect(LIST_LS_X, LIST_LS_Y, LIST_LS_W, LIST_LS_H, TFT_DARKCYAN); 

  String strBtn1 = "Load";
  tft.fillRect(BUTTON1_X, BUTTON1_Y, BTN_LS_W, BUTTON_H, TFT_BLACK);
  tft.drawRoundRect(BUTTON1_X, BUTTON1_Y-2, BTN_LS_W, BUTTON_H+4, RADIUS, TFT_DARKCYAN);    //out frame
  tft.drawRect(BUTTON1_X+40, BUTTON1_Y-2, BTN_LS_W-80, BUTTON_H+4, TFT_BLACK);              //out frame for cuting
  tft.drawRoundRect(BUTTON1_X+2, BUTTON1_Y, BTN_LS_W-4, BUTTON_H, RADIUS, TFT_DARKCYAN);    //in frame
  tft.drawCentreString(strBtn1, BUTTON1_X + (BTN_LS_W / 2), BUTTON1_Y + (BUTTON_H/2-10),4);

  String strBtn3 = "Save";
  tft.fillRect(BUTTON3_X, BUTTON3_Y, BTN_LS_W, BUTTON_H, TFT_BLACK);
  tft.drawRoundRect(BUTTON3_X, BUTTON3_Y-2, BTN_LS_W, BUTTON_H+4, RADIUS, TFT_DARKCYAN);    //out frame
  tft.drawRect(BUTTON3_X+40, BUTTON3_Y-2, BTN_LS_W-80, BUTTON_H+4, TFT_BLACK);              //out frame for cuting
  tft.drawRoundRect(BUTTON3_X+2, BUTTON3_Y, BTN_LS_W-4, BUTTON_H, RADIUS, TFT_DARKCYAN);    //in frame
  tft.drawCentreString(strBtn3, BUTTON3_X + (BTN_LS_W / 2), BUTTON3_Y + (BUTTON_H/2-10),4);

  // Create exit button on the end
  String strBtn5 = "Exit";
  tft.fillRect(BUTTON5_X, BUTTON5_Y, BUTTON_W, BUTTON_H, TFT_BLACK);
  tft.drawRoundRect(BUTTON5_X, BUTTON5_Y-2, BUTTON_W, BUTTON_H+4, RADIUS, TFT_DARKCYAN);    //out frame
  tft.drawRect(BUTTON5_X+40, BUTTON5_Y-2, BUTTON_W-80, BUTTON_H+4, TFT_BLACK);              //out frame for cuting 
  tft.drawRoundRect(BUTTON5_X+2, BUTTON5_Y, BUTTON_W-4, BUTTON_H, RADIUS, TFT_DARKCYAN);    //in frame
  tft.drawCentreString(strBtn5, BUTTON5_X + (BUTTON_W / 2), BUTTON5_Y + (BUTTON_H/2-10),4);

  tft.setTextDatum(CC_DATUM); // Center, center enumerate the text plotting alignment
}


//==========================================================================================
// Lvl 2 PRESS
//==========================================================================================
void press_Settings_btns(uint16_t x, uint16_t y){
  if ((y > BUTTON1_Y) && (y <= (BUTTON1_Y + BUTTON_H))) {
        program();
        delay(200);
      }
      else if ((y > BUTTON2_Y) && (y < (BUTTON2_Y + BUTTON_H))) {
        load_save();
        delay(200);
      }
      else if ((y > BUTTON3_Y) && (y < (BUTTON3_Y + BUTTON_H))) {
        TOUCH_CALIBRATE = true;
        touch_calibrate();
        settingsButtons();
        delay(200);
      }
      else if ((y > BUTTON4_Y) && (y < (BUTTON4_Y + BUTTON_H))) {
        delay(200);
      }
      else if ((y > BUTTON5_Y) && (y < (BUTTON5_Y + BUTTON_H))) {
        btn5Settings();
        delay(200);
      }
}

//==========================================================================================
// Lvl 3 PRESS in PROGRAM MENU
//==========================================================================================
void press_Program_btns(uint16_t x, uint16_t y){

  if((y > BTN1_PRG_PM_Y) && (y < (BTN1_PRG_PM_Y + BTN_PRG_H))){
    if( (x > BTN_PRG_P_X) ){
      //on_off(&io1, 0);
      Serial.println("[io1] on/off");
    }
    else if((x > BTN_INT_X) && (x < BTN_INT_X + BTN_INT_W)){
      keyboard(1);
      Serial.println("[io1] keyboard");
    }
    delay(200);
  }
      
  else if((y > BTN2_PRG_PM_Y) && (y < (BTN2_PRG_PM_Y + BTN_PRG_H))){
    if( (x > BTN_PRG_P_X) ){
      //on_off(&io2, 1);
      Serial.println("[io2] on/off");
    }
    else if((x > BTN_INT_X) && (x < BTN_INT_X + BTN_INT_W)){
      keyboard(2);
      Serial.println("[io2] keyboard");
    }
    delay(200);
  }
      
  else if((y > BTN3_PRG_PM_Y) && (y < (BTN3_PRG_PM_Y + BTN_PRG_H))){
    if( (x > BTN_PRG_P_X) ){
      //on_off(&io3, 2);
      Serial.println("[io3] on/off");
    }
    else if((x > BTN_INT_X) && (x < BTN_INT_X + BTN_INT_W)){
      keyboard(3);
      Serial.println("[io3] keyboard");
    }
    delay(200);
  }

  else if((y > BTN4_PRG_PM_Y) && (y < (BTN4_PRG_PM_Y + BTN_PRG_H))){
    if( (x > BTN_PRG_P_X) ){
      //on_off(&io4, 3);
      Serial.println("[io4] on/off");
    }
    else if((x > BTN_INT_X) && (x < BTN_INT_X + BTN_INT_W)){
      keyboard(4);
      Serial.println("[io4] keyboard");
    }
    delay(200);
  }
      
  else if ((y > BUTTON5_Y) && (y < (BUTTON5_Y + BUTTON_H))){
    btn5Program();
    delay(200);
  }
}


//==========================================================================================
// Lvl KEYBOARD
//==========================================================================================
void press_Keyboard_btns(uint16_t x, uint16_t y){

  //tft.drawRect(KEYBOARD_FRAME_X+KEY_W*k, KEYBOARD_FRAME_Y+KEY_H*l, KEY_W, KEY_H, TFT_DARKCYAN);
  
  if((y > KEYBOARD_FRAME_Y) && (y < (KEYBOARD_FRAME_Y + KEY_H))){
    Serial.println("1,4,7,0");
    if(x > KEYBOARD_FRAME_X && x < KEYBOARD_FRAME_X + KEY_W){
      Serial.println("1");
      input_keyboard += "1";
      Serial.println(input_keyboard);
    }
    else if(x > KEYBOARD_FRAME_X + KEY_W && x < KEYBOARD_FRAME_X + KEY_W*2){
      Serial.println("4");
      input_keyboard += "4";
      Serial.println(input_keyboard);
    }
    else if(x > KEYBOARD_FRAME_X + KEY_W*2 && x < KEYBOARD_FRAME_X + KEY_W*3){
      Serial.println("7");
      input_keyboard += "7";
      Serial.println(input_keyboard);
    }
    else if(x > KEYBOARD_FRAME_X + KEY_W*3 && x < KEYBOARD_FRAME_X + KEY_W*4){
      Serial.println("0");
      input_keyboard += "0";
      Serial.println(input_keyboard);
    }
    delay(200);
  }
  else if((y > KEYBOARD_FRAME_Y+KEY_H) && (y < (KEYBOARD_FRAME_Y + KEY_H*2))){
    Serial.println("2,5,8,.");
    if(x > KEYBOARD_FRAME_X && x < KEYBOARD_FRAME_X + KEY_W){
      Serial.println("2");
      input_keyboard += "2";
      Serial.println(input_keyboard);
    }
    else if(x > KEYBOARD_FRAME_X + KEY_W && x < KEYBOARD_FRAME_X + KEY_W*2){
      Serial.println("5");
      input_keyboard += "5";
      Serial.println(input_keyboard);
    }
    else if(x > KEYBOARD_FRAME_X + KEY_W*2 && x < KEYBOARD_FRAME_X + KEY_W*3){
      Serial.println("8");
      input_keyboard += "8";
      Serial.println(input_keyboard);
    }
    else if(x > KEYBOARD_FRAME_X + KEY_W*3 && x < KEYBOARD_FRAME_X + KEY_W*4){
      Serial.println(".");
      input_keyboard += ".";
      Serial.println(input_keyboard);
    }
    delay(200);
  }
  else if((y > KEYBOARD_FRAME_Y+KEY_H*2) && (y < (KEYBOARD_FRAME_Y + KEY_H*3))){
    Serial.println("3,6,9,Enter");
    if(x > KEYBOARD_FRAME_X && x < KEYBOARD_FRAME_X + KEY_W){
      Serial.println("3");
      input_keyboard += "3";
      Serial.println(input_keyboard);
    }
    else if(x > KEYBOARD_FRAME_X + KEY_W && x < KEYBOARD_FRAME_X + KEY_W*2){
      Serial.println("6");
      input_keyboard += "6";
      Serial.println(input_keyboard);
    }
    else if(x > KEYBOARD_FRAME_X + KEY_W*2 && x < KEYBOARD_FRAME_X + KEY_W*3){
      Serial.println("9");
      input_keyboard += "9";
      Serial.println(input_keyboard);
    }
    else if(x > KEYBOARD_FRAME_X + KEY_W*3 && x < KEYBOARD_FRAME_X + KEY_W*4){
      Serial.println("Enter");
      Serial.print("Save to: io");
      Serial.println(in_keyboard);
      switch(in_keyboard) {
        case 1:
          io1 = long(input_keyboard.toFloat()*1000);
          break;
        case 2:
          io2 = long(input_keyboard.toFloat()*1000);
          break;
        case 3:
          io3 = long(input_keyboard.toFloat()*1000);
          break;
        case 4:
          io4 = long(input_keyboard.toFloat()*1000);
          break;
        default:
          // if nothing else matches, do the default
          // default is optional
          break;
      }
      Serial.print(io1);
      Serial.print(":");
      Serial.print(io2);
      Serial.print(":");
      Serial.print(io3);
      Serial.print(":");
      Serial.println(io4);
      program();
    }
    delay(200);
  }

}

//==========================================================================================
// SHOW KEYBOARD
//==========================================================================================
void keyboardButtons(){

  //int off_key_x = KEY_W;
   // clear screen
  tft.fillScreen(TFT_BLACK);
  if (in_keyboard >= 1 && in_keyboard <=2){
    tft.setTextColor(TFT_YELLOW);
  }
  else if (in_keyboard >= 3 && in_keyboard <=4){
    tft.setTextColor(TFT_RED);
  }
  else{
    tft.setTextColor(TFT_SKYBLUE);
  }

  // KEYBOARD_FRAME
  //tft.drawRect(KEYBOARD_FRAME_X, KEYBOARD_FRAME_Y, KEYBOARD_FRAME_W, KEYBOARD_FRAME_H, TFT_DARKCYAN);

  byte num_label = 0;
  for(int k=0; k<4; k++){
    for(int l=0; l<3; l++){
      num_label++;
      Serial.println(num_label);
      tft.drawRect(KEYBOARD_FRAME_X+KEY_W*k, KEYBOARD_FRAME_Y+KEY_H*l, KEY_W, KEY_H, TFT_DARKCYAN);
      if (num_label < 10){
        tft.drawCentreString(String(num_label), (KEYBOARD_FRAME_X+KEY_W*k) + (KEY_W/2), (KEYBOARD_FRAME_Y+KEY_H*l) + (KEY_H/2)-10, 4);
      }
      else if (num_label == 10){
        tft.drawCentreString(("0"), (KEYBOARD_FRAME_X+KEY_W*k) + (KEY_W/2), (KEYBOARD_FRAME_Y+KEY_H*l) + (KEY_H/2)-10, 4);
      }
      else if (num_label == 11){
        tft.drawCentreString(("."), (KEYBOARD_FRAME_X+KEY_W*k) + (KEY_W/2), (KEYBOARD_FRAME_Y+KEY_H*l) + (KEY_H/2)-10, 4);
      }
      else if (num_label == 12){
        tft.drawCentreString(("Enter"), (KEYBOARD_FRAME_X+KEY_W*k) + (KEY_W/2), (KEYBOARD_FRAME_Y+KEY_H*l) + (KEY_H/2)-10, 2);
      }
    }
  }
  
  tft.setTextDatum(CC_DATUM); // Center, center enumerate the text plotting alignment
  
}

//==========================================================================================
// Lvl 3 PRESS in LOAD & SAVE MENU
//==========================================================================================
void press_LoadSave_btns(uint16_t x, uint16_t y){

  if ( (y > BUTTON1_Y) && (y < (BUTTON1_Y + BUTTON_H)) ){
    if ( (x > BUTTON1_X) && (x < BTN_LS_W) ){
      load_data();
      delay(200);
    }
  }
  else if ( (y > BUTTON3_Y) && (y < (BUTTON3_Y + BUTTON_H)) ){
    if ( (x > BUTTON3_X) && (x < BTN_LS_W) ){
      save_data();
      delay(200);
    }
  }
  else if ((y > BUTTON5_Y) && (y < (BUTTON5_Y + BUTTON_H))){
    btn5LoadSave(); // EXIT BUTTON
    delay(200);
  }
}


//==========================================================================================
// Lvl 1 PRESS in MAIN MENU
//==========================================================================================
void press_Menu_btns(uint16_t x, uint16_t y){
  if ((y > BUTTON1_Y) && (y <= (BUTTON1_Y + BUTTON_H))) {
        btn1Main();
        delay(200);
      }
      else if ((y > BUTTON2_Y) && (y < (BUTTON2_Y + BUTTON_H))) {
        btn2Main();
        delay(200);
      }
}


//==========================================================================================
//==========================================================================================
//==========================================================================================
void loop(){
  
  uint16_t x, y;

  // See if there's any touch data for us
  if (tft.getTouch(&x, &y)){
    // Draw a block spot to show where touch was calculated to be
    #ifdef GREY_SPOT
      tft.fillCircle(x, y, 2, TFT_DARKGREY);
    #endif

    if ( in_menu == 1 && in_settings == 0 && in_program == 0 && in_load_save == 0 ){            // IN MAIN MENU
      press_Menu_btns(x, y);
    }
    else if ( in_menu == 0 && in_settings == 1 && in_program == 0 && in_load_save == 0 ){       // IN SETTINGS MENU
      press_Settings_btns(x, y);
      Serial.println("[+] IN SETTINGS MENU");
    }
    else if ( in_menu == 0 && in_settings == 0 && in_program == 1 && in_load_save == 0 ){       // IN PROGRAM MENU
      if (in_keyboard == 0){
        press_Program_btns(x, y);
        Serial.println("[+] IN PROGRAM MENU");
      }
      else if(in_keyboard > 0){
        press_Keyboard_btns(x, y);
        Serial.println("[+] IN KEYBOARD");
      }
    }
    else if ( in_menu == 0 && in_settings == 0 && in_program == 0 && in_load_save == 1 ){       // IN LOAD & SAVE MENU
      press_LoadSave_btns(x, y);
      Serial.println("[+] IN LOAD & SAVE MENU");
    } 
  }
  
  if (start_prog == 1){
    run_and_graph(); //16-17ms
  }
  else{
    out1 = 0;
    out2 = 0;
  }
  
}

//==========================================================================================
void btn1Main(){
  // START
  if (start_prog == 0){
    start_prog = 1;
    Serial.println("[+] Start");
    tft.setTextColor(TFT_SKYBLUE);
    tft.fillRect(BUTTON1_X+10, BUTTON1_Y+5, BUTTON_W-30, BUTTON_H-10, TFT_BLACK);
    tft.drawCentreString(" Stop ", BUTTON1_X + (BUTTON_W / 2), BUTTON1_Y + (BUTTON_H/2-10),4);
  }
  else if(start_prog == 1){
    start_prog = 0;
    tft.setTextColor(TFT_SKYBLUE);
    tft.fillRect(BUTTON1_X+10, BUTTON1_Y+5, BUTTON_W-30, BUTTON_H-10, TFT_BLACK);
    Serial.println("[+] Stop");
    tft.drawCentreString(" Start ", BUTTON1_X + (BUTTON_W / 2), BUTTON1_Y + (BUTTON_H/2-10),4);
  }
}

//==========================================================================================
void btn2Main(){
  // SETTINGS
  if (start_prog == 0){
    in_menu = 0;
    in_settings = 1;
    settingsButtons();
  }
}

//==========================================================================================
void btn5Settings(){
  // EXIT
  in_menu = 1;
  in_settings = 0;
  menuButtons();
}

//==========================================================================================
void btn5Program(){
  // EXIT
  in_load_save = 0;
  in_program = 0;
  in_settings = 1;
  in_menu = 0;
  settingsButtons();
}

//==========================================================================================
void btn5LoadSave(){
  // EXIT
  in_load_save = 0;
  in_program = 0;
  in_settings = 1;
  in_menu = 0;
  settingsButtons();
}

//==========================================================================================
void set_on_off(int num){
  ;
}

//==========================================================================================
void keyboard(byte num){
  
  in_keyboard = num;
  input_keyboard = "";
  keyboardButtons(); //(long *io, int num)

}

//==========================================================================================
void save_data(){
  
  File file = SPIFFS.open(DATA_FILE, "w");
  if(!file){
    // File not found
    tft.setTextColor(TFT_GREEN,TFT_BLUE);
    tft.drawCentreString("Failed to open test file", 160, 100,2);
    return;
  }
  else {
    file.println(String(io1));
    file.println(String(io2));
    file.println(String(io3));
    file.println(String(io4));
    tft.setTextColor(TFT_GREEN,TFT_BLUE);
    tft.drawCentreString("Hello From ESP32 :-)", 160, 100,2);
    file.close();
  }

  Serial.println("[!] SAVE DATA");

}

//==========================================================================================
void load_data(){

  char Data_prog_1[128];
  String print_data = "";

  File file = SPIFFS.open(DATA_FILE, "r");
  if(!file){
    // File not found
    tft.setTextColor(TFT_GREEN,TFT_BLUE);
    tft.drawCentreString("Failed to open test file", 160, 100,2);
    return;
  }
  else {
    int i = 0;
    Serial.println("[!] DATA: ");
    while(file.available()){
      Data_prog_1[i] = file.read();
      Serial.print(Data_prog_1[i]);

      if (Data_prog_1[i] == 10){
        Data_prog_1[i] = 58; // ASCII colon >> :
      }
      print_data = print_data + Data_prog_1[i];
      
      i++;
    }

    //unsigned long idata = print_data.toInt();;
    tft.setTextColor(TFT_GREEN,TFT_BLUE);
    tft.drawCentreString(print_data, 160, 100,2);
    //tft.drawNumber(idata, 160, 130,2);

    //io1 = idata;
    
    file.close();
  }

  Serial.println("[!] LOAD DATA");
}

//==========================================================================================
void ls(){

  File root = SPIFFS.open("/");

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
    }
    else{
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

//==========================================================================================
void touch_calibrate(){
  
  uint16_t calData[5];
  uint8_t calDataOK = 0;


  // check file system exists
  if (!SPIFFS.begin()) {
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL || TOUCH_CALIBRATE)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
  delay(500);
  TOUCH_CALIBRATE = false;
}
//==========================================================================================
