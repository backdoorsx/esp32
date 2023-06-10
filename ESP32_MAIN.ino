#include <WiFi.h>
#include <esp_now.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_VL53L0X.h"
#include "bmm150.h"
#include "bmm150_defs.h"

#ifdef __cplusplus
extern "C" {
    #endif
    uint8_t temprature_sens_read();
    #ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();
float temp_cpu = 0;
float temp_cpu_bms = 0;

//#define DEBUG

BMM150 bmm = BMM150();
bmm150_mag_data value_offset;

Adafruit_VL53L0X tof = Adafruit_VL53L0X();

Adafruit_MPU6050 mpu; //0x68

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire); //0x3c

//vl 0x29 - 0x7f
//mag 0x13
 
// MY MAC: 10:52:1C:74:63:1C

uint8_t broadcastAddress1[] = {0x10,0x52,0x1C,0x76,0x10,0x68}; // MOTOR L 10:52:1C:76:10:68
uint8_t broadcastAddress2[] = {0xA4,0xCF,0x12,0x44,0xF4,0x14}; // BMS A4:CF:12:44:F4:14 = 164:207:18:68:244:20

/* LED */
const int LED = 2;    //LED on the board ESP32

// Define variables to store incoming readings
float indata1;
float indata2;
float indata3;
float indata4;
float indata5;
float indata6;

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    float d1;
    float d2;
    float d3;
    float d4;
    float d5;
    float d6;
} struct_message;

// Create a struct_message called BME280Readings to hold sensor readings
struct_message Readings;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

esp_now_peer_info_t peerInfo;

// Variables to store info for show screen
float VOLTS = 0.0;
String MAC = "";
String I2CS = "";
int COMPASS = 0;
String GPS = "";
int TOF = 0;
float AX = 0; //MPU accel x
float AY = 0; //MPU accel y
float AT = 0; //MPU temperature
int CX = 0; //BMM axis x
int CY = 0; //BMM axis y
int CZ = 0; //BMM axis z

bool STAT_TOF = false;
bool STAT_MPU = false;
bool STAT_BMM = false;

// menu tags
bool MENU_MAIN = true;
bool MENU_MPU = false;
bool MENU_BMM = false;
bool MENU_MOTORS = false;
bool MENU_SYS = false;

//Distance from TOF for MENU 
int TOF_MAIN[] = {0, 50};
int TOF_MPU[] = {50, 120};
int TOF_BMM[] = {120, 190};
int TOF_MOTORS[] = {190, 260};
int TOF_SYS[] = {260, 360};

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
    if (status ==0){
        success = "Delivery Success :)";
    }
    else{
        success = "Delivery Fail :(";
    }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
    Serial.print("Bytes received: ");
    Serial.println(len);
    indata1 = incomingReadings.d1;
    indata2 = incomingReadings.d2;
    indata3 = incomingReadings.d3;
    indata4 = incomingReadings.d4;
    indata5 = incomingReadings.d5;
    indata6 = incomingReadings.d6;

    String mac_str = String(mac[0]);
    for(int i=1; i<6; i++)
        mac_str += ":" + String(mac[i]);

    String bms_str = "164:207:18:68:244:20";
    if(bms_str == mac_str)
        VOLTS = battery_monitoring_system(indata1, indata2, indata3, indata4, indata5, indata6);
        temp_cpu_bms = indata6;
}


String i2c_scan(){
    
    byte error, address;
    int nDevices;
    String ret = "";
 
    Serial.println("Scanning I2C...");
 
    nDevices = 0;
    for(address = 1; address < 127; address++ ){
 
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
 
        if (error == 0){
            Serial.print("I2C device found at address 0x");
            if (address<16) 
                Serial.print("0");
            ret = ret + String(address,HEX) + " ";
            Serial.print(address,HEX);
            Serial.println("  !");
 
            nDevices++;
        }
        else if(error==4){
            Serial.print("Unknow error at address 0x");
            if(address<16) 
                Serial.print("0");
                
            Serial.println(address,HEX);
        }    
    }
    
    if(nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done\n");

    return ret;
}

void setup(){
    pinMode(LED, OUTPUT);
    
    Serial.begin(115200);
    
    display.begin(0x3C, true);
    display.display();
    delay(500);

    // Clear the buffer.
    display.clearDisplay();
    display.display();
    display.setTextSize(1);
    display.setRotation(1);
    display.setTextColor(SH110X_WHITE);
    display.cp437(true);

    Serial.print("MAIN ADDRESS: ");
    Serial.println(WiFi.macAddress());
    
    MAC = WiFi.macAddress();
    display.print("MAC=");
    display.println(MAC);
    display.display();

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        
        display.println("Error initializing ESP-NOW");
        display.display();
        return;
    }

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);

    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
  
    // Register peer
    memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer 1");
        return;
    }

    // Register peer
    memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer 2");
        return;
    }
    
    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(OnDataRecv);

    I2CS = i2c_scan();
    display.print("I2C=");
    display.println(I2CS);
    display.display();

    if(!mpu.begin())
        Serial.println("Failed to find MPU6050 chip");
    else{
        STAT_MPU = true;
        mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
        mpu.setGyroRange(MPU6050_RANGE_500_DEG);
        mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    }

    if(bmm.initialize() == BMM150_E_ID_NOT_CONFORM)
        Serial.println("Failed to boot BMM150!");
    else{
        STAT_BMM = true;
        int xcalib[] = {-6, -6, -4, 0, -5, -3, -9, -4};
        int ycalib[] = {-6, 4, 2, -2, 0, -1, -2, 7};
        int zcalib[] = {-66, -82, -64, -74, -64, -67, -73, -69};
        int array_length = sizeof(xcalib)/sizeof(xcalib[0]);

        for(int i=0; i<array_length; i++){
            value_offset.x += xcalib[i];
            value_offset.y += ycalib[i];
            value_offset.z += zcalib[i];
        }
        value_offset.x = value_offset.x/array_length;
        value_offset.y = value_offset.y/array_length;
        value_offset.z = value_offset.z/array_length;
    }

    if(!tof.begin())
        Serial.println(F("Failed to boot VL53L0X"));
    else{
        STAT_TOF = true;
        tof.startRangeContinuous();
    }
}

float battery_monitoring_system(float S1, float S2, float S3, float S4, float cap, float bms_temp_cpu){
    #if defined(DEBUG)
    Serial.println("[BMS]");
    Serial.print("  Cell1: ");
    Serial.print(S1);
    Serial.print(" | Cell2: ");
    Serial.print(S2-S1);
    Serial.print(" | Cell3: ");
    Serial.print(S3-S2);
    Serial.print(" | Cell4: ");
    Serial.print(S4-S3);
    Serial.print(" = ");
    Serial.print(S4);
    Serial.print("V = ");
    Serial.print(cap);
    Serial.print("%  |  ");
    Serial.print("Temp cpu: ");
    Serial.println(bms_temp_cpu);
    
    #endif

    return S4;
}

void compass_bmm150(){
    bmm150_mag_data value;
    bmm.read_mag_data();

    value.x = bmm.raw_mag_data.raw_datax - value_offset.x;
    value.y = bmm.raw_mag_data.raw_datay - value_offset.y;
    value.z = bmm.raw_mag_data.raw_dataz - value_offset.z;

    #if defined(DEBUG)
    Serial.print("Compass X:");
    Serial.print(value.x);
    Serial.print("; Y:");
    Serial.print(value.y);
    Serial.print("; Z:");
    Serial.println(value.z);
    #endif

    CX = value.x;
    CY = value.y;
    CZ = value.z;

    float xyHeading = atan2(value.x, value.y);
    float zxHeading = atan2(value.z, value.x);
    float heading = xyHeading;

    if(heading < 0)
        heading += 2*PI;
    if(heading > 2*PI)
        heading -= 2*PI;
  
    float headingDegrees = heading * 180/M_PI; 
    float xyHeadingDegrees = xyHeading * 180 / M_PI;
    float zxHeadingDegrees = zxHeading * 180 / M_PI;

    COMPASS = headingDegrees;
}

void tof_vl53l0x(){
    /*
    VL53L0X_RangingMeasurementData_t measure;
    tof.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

    if (measure.RangeStatus != 4){
        Serial.println(measure.RangeMilliMeter);
        Serial.print("Distance mm: ");
        TOF = measure.RangeMilliMeter;
        Serial.println(TOF);
    }
    */
    if (tof.isRangeComplete()) {
        TOF = tof.readRange();
        #if defined(DEBUG)
        Serial.print("Distance mm: ");
        Serial.println(TOF);
        #endif
    }
}

void mpu_6050(){
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    AX = a.acceleration.x;
    AY = a.acceleration.y;
    AT = temp.temperature;
    
    #if defined(DEBUG)
    Serial.print("Acc X: ");
    Serial.print(AX);
    Serial.print(", Y: ");
    Serial.print(AY);
    Serial.print(", Z: ");
    Serial.print(a.acceleration.z);
    Serial.println(" m/s^2");
    
    Serial.print("Rot X: ");
    Serial.print(g.gyro.x);
    Serial.print(", Y: ");
    Serial.print(g.gyro.y);
    Serial.print(", Z: ");
    Serial.print(g.gyro.z);
    Serial.println(" rad/s");

    Serial.print("Temp: ");
    Serial.print(AT);
    Serial.println(" degC");
    #endif
}

void screen_main(){
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("MAC=");
    display.println(MAC);
    display.print("I2C=");
    display.println(I2CS);
    display.println("--------------------");
    display.print("Battery=");
    display.print(VOLTS);
    display.println("V");
    display.print("Compass=");
    display.print(COMPASS);
    display.write(0xf8);
    display.println("");
    display.print("Distance=");
    display.print(TOF);
    display.println("mm");
    display.print("Accel=");
    display.print(AX);
    display.print("|");
    display.print(AY);
    display.print("m/s");
    display.write(0xfd);
    display.display();
}

void screen_mpu(){

    display.clearDisplay();
    
    if(TOF > TOF_MPU[0] && TOF <= TOF_MPU[1]){
        int slider = map(TOF, TOF_MPU[0], TOF_MPU[1], 0, 64);
        display.setCursor(0,slider);
        display.write(0x10);
    }
    
    display.setCursor(16,0);
    display.print("Acceleration m/s");
    display.write(0xfd);

    display.setCursor(2,56);
    display.print(AX);

    display.setCursor(40,56);
    display.print(AT);
    display.write(0xf8);
    display.write(0x43);
    
    display.setCursor(90,56);
    display.print(AY);

    display.drawRect((128/2)-5, (64/2)-5, 10, 10, SH110X_WHITE);

    //(128,64)
    int x = 128/2;
    int y = 64/2;
    byte x_offs = 2;
    byte y_offs = 2;
    
    if(AX >= 0){
        y = map(AX*1000, 0, 9*1000, 64/2, 64);
    }
    else{
        y = map(AX*1000, -9*1000, 0, 0, 64/2);
    }

    if(AY >= 0){
        x = map(AY*1000, 0, 9*1000, 128/2, 128);
    }
    else{
        x = map(AY*1000, -9*1000, 0, 0, 128/2);
    }

    display.setCursor(x-x_offs,y-y_offs);
    display.write(0xfe);

    display.display();
    
}

void screen_bmm(){
    display.clearDisplay();

    if(TOF > TOF_BMM[0] && TOF <= TOF_BMM[1]){
        int slider = map(TOF, TOF_BMM[0], TOF_BMM[1], 0, 64);
        display.setCursor(0,slider);
        display.write(0x10);
    }

    display.setCursor(22,0);
    display.print("Compass ");
    display.print(COMPASS);
    display.write(0xf8);
    
    display.setCursor(2,56);
    display.print("X:");
    display.print(CX);

    display.setCursor(48,56);
    display.print("Y:");
    display.print(CY);
    
    display.setCursor(90,56);
    display.print("Z:");
    display.print(CZ);

    display.drawCircle(display.width()/2, display.height()/2, 20, SH110X_WHITE);
    display.drawCircle(display.width()/2, display.height()/2, 14, SH110X_WHITE);

    int dx = (20*cos((COMPASS-90)*PI/180))+display.width()/2;
    int dy = (20*sin((COMPASS-90)*PI/180))+display.height()/2;
    display.drawLine(display.width()/2, display.height()/2, dx, dy, SH110X_WHITE);
    display.fillCircle(display.width()/2, display.height()/2, 12, SH110X_BLACK);
    display.setCursor(display.width()/2-2, display.height()/2-10);
    display.print("N");
    
    display.display();
}

void screen_motors(){
    display.clearDisplay();

    if(TOF > TOF_MOTORS[0] && TOF <= TOF_MOTORS[1]){
        int slider = map(TOF, TOF_MOTORS[0], TOF_MOTORS[1], 0, 64);
        display.setCursor(0,slider);
        display.write(0x10);
    }

    display.setCursor(32, 0);
    display.print("Motors");
    
    display.setCursor(0, 16);
    display.print("  L=");
    display.println(MAC);
    display.print("  R=");
    display.println(MAC);
    
    display.setCursor(0, 35);
    display.print("  L: ");
    display.println("STATUS");
    display.print("  R: ");
    display.println("STATUS");

    

    display.display();
}

void screen_sys(){
    display.clearDisplay();

    if(TOF > TOF_SYS[0] && TOF <= TOF_SYS[1]){
        int slider = map(TOF, TOF_SYS[0], TOF_SYS[1], 0, 64);
        display.setCursor(0,slider);
        display.write(0x10);
    }

    display.setCursor(16,0);
    display.print("System and power");
    
    display.setCursor(0, 16);
    display.print("  CPU    : ");
    display.print(temp_cpu);
    display.write(0xf8);
    display.write(0x43);

    display.setCursor(0, 26);
    display.print("  CPU BMS: ");
    display.print(temp_cpu_bms);
    display.write(0xf8);
    display.write(0x43);
    
    display.display();
}

void menu(){
    
    tof_vl53l0x();

    if(TOF <= TOF_MAIN[1]){
        MENU_MAIN = true;
        MENU_MPU = false;
        MENU_BMM = false;
        MENU_MOTORS = false;
        MENU_SYS = false;
    }       
    else if(TOF > TOF_MPU[0] && TOF <= TOF_MPU[1]){
        MENU_MAIN = false;
        MENU_MPU = true;
        MENU_BMM = false;
        MENU_MOTORS = false;
        MENU_SYS = false;
    }
    else if(TOF > TOF_BMM[0] && TOF <= TOF_BMM[1]){
        MENU_MAIN = false;
        MENU_MPU = false;
        MENU_BMM = true;
        MENU_MOTORS = false;
        MENU_SYS = false;
    }
    else if(TOF > TOF_MOTORS[0] && TOF <= TOF_MOTORS[1]){
        MENU_MAIN = false;
        MENU_MPU = false;
        MENU_BMM = false;
        MENU_MOTORS = true;
        MENU_SYS = false;
    }
    else if(TOF > TOF_SYS[0] && TOF <= TOF_SYS[1]){
        MENU_MAIN = false;
        MENU_MPU = false;
        MENU_BMM = false;
        MENU_MOTORS = false;
        MENU_SYS = true;
    }
    
    if(MENU_MAIN)
        screen_main();
    else if(MENU_MPU)
        screen_mpu();
    else if(MENU_BMM)
        screen_bmm();
    else if(MENU_MOTORS)
        screen_motors();
    else if(MENU_SYS)
        screen_sys();
}

void loop(){

    if((temprature_sens_read()) != 128 )
        temp_cpu = (temprature_sens_read() - 32) / 1.8;

    menu();
    compass_bmm150();
    mpu_6050();
    /*
    Readings.d1 = 9.1;
    Readings.d2 = 9.2;
    Readings.d3 = 9.3;
    Readings.d4 = 9.4;
    Readings.d5 = 9.5;
    Readings.d6 = 9.6;
  
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(0, (uint8_t *) &Readings, sizeof(Readings));
    //esp_err_t result2 = esp_now_send(broadcastAddress2, (uint8_t *) &Readings, sizeof(Readings));

    if(result == ESP_OK)
        Serial.println("Sent with success");
    else
        Serial.println("Error sending the data");
*/
    delay(100);
}
