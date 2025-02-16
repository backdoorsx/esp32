/*
 BOARD = Nologo ESP32C3 Super Mini
 OLED = GME64128-02 VER:1.0 I2C ADDRESS:0X3C

 INSTALL:
 FILE > PERFORMANCES > Additional boards URLs > https://espressif.github.io/arduino-esp32/package_esp32_index.json
 LIBRARY MANAGER > SH110X (by Adafruit ver:2.1.11)
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h> 

//odkomentuj #define SH110X_NO_SPLASH v kniznici Arduino/libraries/Adafruit_SH110X/Adafruit_SH110X.h
//uprava kniznice pre nologo_esp32c3_super_mini pre I2C piny:
/*
.arduino15/packages/esp32/hardware/esp32/3.1.0/variants/nologo_esp32c3_super_mini/pins_arduino.h
  static const uint8_t SDA = 7;
  static const uint8_t SCL = 8;
  static const uint8_t SS = 9;
*/

#define SCREEN_WIDTH 64  // OLED display width, in pixels
#define SCREEN_HEIGHT 128 // OLED display height, in pixels
#define OLED_RESET -1     // can set an oled reset pin if desired
Adafruit_SH1107 display = Adafruit_SH1107(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 100000);

float HALL_THRESHOLD = 100.0;

void setup() {
  Serial.begin(115200);

  //set the resolution to 12 bits (0-4095)
  analogReadResolution(12);

  display.begin(0x3C, true);
 
  display.display();
  delay(100);

  // Clear the buffer.
  display.clearDisplay();
  display.setRotation(3);

  // draw a single pixel
  display.drawPixel(10, 10, SH110X_WHITE);
  display.display();
  delay(1000);

  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SH110X_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Hello, tacho!"));

  display.setTextColor(SH110X_BLACK, SH110X_WHITE); // Draw 'inverse' text
  display.println(3.14159265358979);

  display.setTextSize(2);             // Draw 2X-scale text
  display.setTextColor(SH110X_WHITE);
  display.print(F("0x")); display.println(0xDEADBEEF, HEX);

  display.display();
}

void loop() {

  float count = 1.0;
  float timer_start = micros();
  bool state = false;

  while(true){
    //Serial.printf("ADC analog value = %d\n", analogRead(3));
    if(analogRead(3) <= 200){ // analog threshold
      if(state == false){
        state = true;
        count += 1.0;
      }
    }
    else{
      state = false;
    }
    
    if (count >= HALL_THRESHOLD){
      break;
    }
  }
  
  //There are 1,000 microseconds in a millisecond and 1,000,000 microseconds in a second.
  float timer = ((micros()-timer_start)/1000000.0);
  Serial.print("Timer: ");
  Serial.print(timer);
  Serial.println("s");
  float rpm = (count/timer)*60.0;
  Serial.print(rpm);
  Serial.println(" RPM");

  display.clearDisplay();
  display.setTextSize(2);             // Draw 2X-scale text
  display.setTextColor(SH110X_WHITE);
  display.setCursor(10,10);
  display.println("RPM:");
  display.setCursor(10,28);
  display.println(rpm);

  display.setTextSize(1);
  display.setCursor(10,55);
  display.print("Timer 100: ");
  display.print(timer);
  display.println("s");

  display.display();
  delay(1);        // delay in between reads for stability

}
