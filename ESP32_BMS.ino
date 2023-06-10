#include <WiFi.h>
#include <esp_now.h>

#ifdef __cplusplus
extern "C" {
    #endif
    uint8_t temprature_sens_read();
    #ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();
float temp_cpu = 0;

// MY MAC: A4:CF:12:44:F4:14

/* SETTINGS ESP-NOW */
uint8_t broadcastAddress[] = {0x10,0x52,0x1C,0x74,0x63,0x1C}; // main 10:52:1C:74:63:1C

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
/* EOL ESP-NOW */

/* LIPO 4S */
const int V1 = 39;      //ADC1
const int V2 = 33;      //ADC1
const int V3 = 34;      //ADC1
const int V4 = 35;      //ADC1

/* LED */
const int LED = 2;      //LED on the board ESP32
//const int leds[] = {27,25,32,22,21,17,16,4};

long lastMsg = 0;       //MILLIS()

/* Callback when data is sent */
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

/* Callback when data is received */
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
}

void setup() {
    pinMode(LED, OUTPUT);
    //for(int i=0; i<8; i++){
    //    pinMode(leds[i], OUTPUT);
    //}
    
    pinMode(V1, INPUT);
    pinMode(V2, INPUT);
    pinMode(V3, INPUT);
    pinMode(V4, INPUT);
    
    Serial.begin(115200);

    Serial.print("BMS ADDRESS: ");
    Serial.println(WiFi.macAddress());

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);
  
    // Register peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }
    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(OnDataRecv);
}

float voltage(int ADC_INPUT){

    float R1 = 30000;
    float R1_V1 = 47000; //volatage sensor with 47k for last battery cell
    float R2 = 7500;

    if(ADC_INPUT == 39)
        R1 = R1_V1;

    int sensorValue = analogRead(ADC_INPUT);
    float v = sensorValue * (3.3 / 4096.0);
    Serial.print(" RAW(");
    Serial.print(v);
    Serial.print(") ");
    v = v + (0.22/2); // na prazdno ma hodnotu 0.2
    v = v / (R2 / (R1 + R2));

    if (sensorValue < 1 || sensorValue >= 4095)
        v = 0.01;

    return v;
}

float re_map(float value, float min_in, float max_in, float min_out, float max_out){
    return ((value-min_in)*(max_out-min_out)/(max_in-min_in)+min_out);
}


void led_up(int c){
    for(int i=0; i<c; i++){
        //digitalWrite(leds[i], HIGH);
        digitalWrite(LED, HIGH);
        delay(150);
        digitalWrite(LED, LOW);
        delay(150);
    }
    digitalWrite(LED, LOW);
}

/*
void led_down(int c){
    for(int i=0; i<c; i++){
        digitalWrite(leds[i], LOW);
        delay(50);
    }
}
*/

void loop() {

    if((temprature_sens_read()) != 128 )
        temp_cpu = (temprature_sens_read() - 32) / 1.8;
    
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    digitalWrite(LED, LOW);
    
    float S1 = voltage(V4);
    Serial.println(S1);

    float S2 = voltage(V2);
    Serial.println(S2);

    float S3 = voltage(V3);
    Serial.println(S3);

    float S4 = voltage(V1);
    Serial.println(S4);

    float cap = re_map(S4, (3.27*4), (4.2*4), 0.1, 100.1);
    Serial.print(cap);
    Serial.println("%");

    Serial.print("Temp CPU: ");
    Serial.println(temp_cpu);

    Readings.d1 = S1;
    Readings.d2 = S2;
    Readings.d3 = S3;
    Readings.d4 = S4;
    Readings.d5 = cap;
    Readings.d6 = temp_cpu;
  
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &Readings, sizeof(Readings));
   
    if(result == ESP_OK)
        Serial.println("Sent with success");
    else
        Serial.println("Error sending the data");

    Serial.println("\nCell voltage:");
    Serial.print(" | Cell1: ");
    Serial.print(S1);
    Serial.print(" | Cell2: ");
    Serial.print(S2 - S1);
    Serial.print(" | Cell3: ");
    Serial.print(S3 - S2);
    Serial.print(" | Cell4: ");
    Serial.println(S4 - S3);

    cap = re_map(S4, (3.27*4), (4.2*4), 1, 8); //4S to 8LED
    //led_up(8);
    //led_down(8);
    led_up(int(round(cap)));
  }
}
