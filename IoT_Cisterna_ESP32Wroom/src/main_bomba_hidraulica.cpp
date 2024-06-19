// BOMBA HIDRÁULICA
#include <esp_now.h>
#include <WiFi.h>

#define SWITCH_PIN_ELECTRONIVEL 32
const int relayPin = 16;
const int ledPin = 23;

int status = 0;

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
#define LOST_COMUNICATION 1000 * 60 * 30

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
  Serial.println(digitalRead(SWITCH_PIN_ELECTRONIVEL));
  if (myData.switchState == HIGH && digitalRead(SWITCH_PIN_ELECTRONIVEL) == LOW)
  {
    digitalWrite(relayPin, HIGH);
  }
  else
  {
    digitalWrite(relayPin, LOW);
  }
}

void checkTimeout()
{
  // Check if a timeout occurred
  unsigned long currentTime = millis();
  if(currentTime - myData.lastReceivedTime > LOST_COMUNICATION) // 30 minutes
  {
    digitalWrite(relayPin, LOW);
    // Blink the LED 3 times to indicate lost communication
    for(int i = 0; i < 3; i++)
    {
      digitalWrite(ledPin, LOW);
      delay(2000);
      digitalWrite(ledPin, HIGH);
      delay(2000);
    }
  }
  else if (currentTime - myData.lastReceivedTime > TIMEOUT_PERIOD) // 10 seconds
  {
    //  turn off the pump
    digitalWrite(relayPin, LOW);
    Serial.println("Timeout: No data received from float switch. Turning off the pump.");
  }
  delay(2000);
}


void setup()
{
  Serial.begin(115200);
  
  // Set the local switch pin as input
  pinMode(SWITCH_PIN_ELECTRONIVEL, OUTPUT);
  digitalWrite(SWITCH_PIN_ELECTRONIVEL, LOW);
  delay(100);  
  pinMode(SWITCH_PIN_ELECTRONIVEL, INPUT_PULLDOWN);
  
  // Relay pin Setup
  pinMode(relayPin, OUTPUT);

  //Builtin led setup, Turn on = execution mode 
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);  

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

}

void loop()
{
  checkTimeout(); // Check for timeout
}