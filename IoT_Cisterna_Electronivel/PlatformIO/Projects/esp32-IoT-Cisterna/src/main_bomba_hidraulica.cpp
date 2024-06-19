//BOMBA HIDRÁULICA
#include <esp_now.h>
#include <WiFi.h>
#include <FastLED.h>


#define NUM_LEDS 1
#define DATA_PIN 48
#define SWITCH_PIN 12


int status = 0;
// Define the array of leds
CRGB leds[NUM_LEDS];


// Structure example to receive data
// Must match the sender structure
typedef struct struct_message
{
 bool switchState;
 unsigned long lastReceivedTime; // Timestamp for the last received data
} struct_message;


// Create a struct_message called myData
struct_message myData;


// Timeout period in milliseconds
#define TIMEOUT_PERIOD 10000 // 10 seconds


// Variable to keep track of the current LED state
bool currentLEDState = LOW;


// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
 memcpy(&myData, incomingData, sizeof(myData));
 myData.lastReceivedTime = millis(); // Update the timestamp
 Serial.print("Bytes received: ");
 Serial.println(len);
 Serial.print("switchState:");
 Serial.println(myData.switchState);
 if (myData.switchState == HIGH && digitalRead(SWITCH_PIN) == HIGH)
 {
   leds[0] = CRGB::Red;
   FastLED.show();
   currentLEDState = HIGH; // Update the current LED state
 }
 else
 {
   leds[0] = CRGB::Black;
   FastLED.show();
   currentLEDState = LOW; // Update the current LED state
 }
}


void checkTimeout()
{
 // Check if a timeout occurred
 unsigned long currentTime = millis();
 if (currentTime - myData.lastReceivedTime > TIMEOUT_PERIOD)
 {
   // Handle the timeout, for example, turn off the pump or take necessary actions
   leds[0] = CRGB::Black; // Turn off the LED
   FastLED.show();
   currentLEDState = LOW; // Update the current LED state
   Serial.println("Timeout: No data received from float switch. Turning off the pump.");
 }
}


void setup()
{
 // Initialize Serial Monitor
 Serial.begin(115200);
 FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);


 // Set device as a Wi-Fi Station
 WiFi.mode(WIFI_STA);


 // Init ESP-NOW
 if (esp_now_init() != ESP_OK)
 {
   Serial.println("Error initializing ESP-NOW");
   return;
 }


 // Once ESPNow is successfully Init, we will register for recv CallBack to
 // get recv packet info
 esp_now_register_recv_cb(OnDataRecv);


 // Set the local switch pin as input
 pinMode(SWITCH_PIN, INPUT);
}


void loop()
{
 checkTimeout(); // Check for timeout
}