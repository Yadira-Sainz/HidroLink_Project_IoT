// ELECTRONIVEL TINACO
#include <esp_now.h>
#include <WiFi.h>
#include <esp_sleep.h>

#define SWITCH_PIN_ELECTRONIVEL 32
#define DEMO_MODE 15
const int ledPin = 23;

uint8_t broadcastAddress[] = {0xe8, 0x6b, 0xea, 0xdf, 0xd9, 0xac};

typedef struct struct_message
{
  bool switchState;
} struct_message;

struct_message myData;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup()
{  
  Serial.begin(115200);
  Serial.println("Starting setup");   

  //Builtin led setup, Turn on = execution mode 
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  //Setting Electronivel pin
  pinMode(SWITCH_PIN_ELECTRONIVEL, OUTPUT);
  digitalWrite(SWITCH_PIN_ELECTRONIVEL, LOW);
  delay(100);  
  pinMode(SWITCH_PIN_ELECTRONIVEL, INPUT_PULLDOWN);

  //Setting DEMO Mode
  pinMode(DEMO_MODE, OUTPUT);
  digitalWrite(DEMO_MODE, LOW);
  delay(100);  
  pinMode(DEMO_MODE, INPUT_PULLDOWN);


  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop()
{
  Serial.println("Starting loop");
  myData.switchState = digitalRead(SWITCH_PIN_ELECTRONIVEL);
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  digitalWrite(ledPin, HIGH);

  if (result == ESP_OK)
  {
    Serial.println("Sent with success");
  }
  else
  {
    Serial.println("Error sending the data");
  }

  if(myData.switchState == HIGH && digitalRead(DEMO_MODE) == HIGH ){ // Need Water in Demo Mode
    Serial.println("Need Water (Demo Mode)");
    delay(2000); 
  }else if(myData.switchState == HIGH && digitalRead(DEMO_MODE) == LOW){ // Need Water in Normal Mode
   Serial.println("Need Water (Normal Mode)");
    delay(10000); // wait 10 seconds before sending status
  }
  else if (myData.switchState == LOW && digitalRead(DEMO_MODE) == LOW ) // No need water Normal mode
  {
    //Deep sleep and wakeeup afeter 10 minutes
    Serial.println("No need water (Normal mode)");
    Serial.println("Deep Sleep ...");
    for(int i = 0; i < 5; i++)
    {
      digitalWrite(ledPin, LOW);
      delay(1000);
      digitalWrite(ledPin, HIGH);
      delay(1000);
    }
    esp_sleep_enable_timer_wakeup(10 * 60 * 1000000);
    esp_deep_sleep_start();
  }else if (myData.switchState == LOW && digitalRead(DEMO_MODE) == HIGH ) { // No need water in demo mode
  Serial.println("No need water (demo mode)");
    delay(2000);
  }
}