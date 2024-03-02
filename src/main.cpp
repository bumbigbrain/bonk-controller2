#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define PUSH_BUTTON 19
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

//define display
// SDA -> GPIO 21
// SCL -> GPIO 22
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
uint8_t Controller1Address[] = {0x3C, 0x61, 0x05, 0x03, 0xA2, 0x74};
esp_now_peer_info_t peerInfoController1;


typedef struct ControllerMessage {
  int gameState;

} ControllerMessage;


ControllerMessage ControllerInfo;
int count = 0;



void displayTesting() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10); 
  display.printf("count : %d\n", count);
  display.display();
}

void displayNotMatch() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.printf("Not match ");
  display.display();
}

void displayMatched() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.printf("Press Start !!");
  display.display();
}




void displayCountdown() {
  
  // Countdown for 5 sec
  int msec = 0, trigger = 5;
  clock_t before = clock();

  do {
    display.clearDisplay(); 
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("Get Ready");
    display.setCursor(11, 20);
    display.printf("%d\n", trigger - msec);
    display.display();
    
    clock_t difference = clock() - before;
    msec = difference * 1000 / CLOCKS_PER_SEC;
    
  } while (msec < trigger);


}


void displayPlaying() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("PLAYING");
  display.display();
}





// void sendResponseMatchingController1() {
//   matchingInfo.matchingState = 1;
//   esp_err_t result = esp_now_send(Controller1Address, (uint8_t *) &matchingInfo, sizeof(matchingInfo));
//   if (result == ESP_OK) {
//     Serial.println("Sent with success");
//   } else {
//     Serial.println("Error sending the data");
//   }
// }


bool isController1(const uint8_t *sentmac) {
  uint8_t Controller1Address[6] = {0x3C, 0x61, 0x05, 0x03, 0xA2, 0x74};
  for (int i=0; i<6; i++) {
    if (Controller1Address[i] != sentmac[i]) {
      return false; }
  }
  return true;
}


void showMac(const uint8_t *sentmac) {
  for (int i=0; i<6; i++) {
    Serial.printf("%x", sentmac[i]);

  }
  Serial.printf("\n");
}

void Playing() {

}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  delay(300);
}


void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  showMac(mac);

  // desperate by mac address if mac == XXX -> memcpy from that Message
  //memcpy(&matchingInfo, incomingData, sizeof(matchingInfo));
  if (isController1(mac)) {
    memcpy(&ControllerInfo, incomingData, sizeof(ControllerInfo));
    Serial.printf("gameState : %d\n", ControllerInfo.gameState);
  }
  

}



void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  //setup BUTTON
  pinMode(PUSH_BUTTON, INPUT_PULLUP);

  //setup DISPLAY
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  //Do OnDataSent when send
  esp_now_register_send_cb(OnDataSent);
  
  //Do OnDataRecv when recieve
  esp_now_register_recv_cb(OnDataRecv);

  //register peer Controller1
  memcpy(peerInfoController1.peer_addr, Controller1Address, 6);
  peerInfoController1.channel = 0;
  peerInfoController1.channel = false;
  esp_now_add_peer(&peerInfoController1);
  if (esp_now_add_peer(&peerInfoController1) != ESP_OK) {
    Serial.println("Failed to add peer Controller 1");
  }

  //ControllerInfo.gameState = 0;
  

}

void loop() {
  
  if (ControllerInfo.gameState == 0) { // No matching
    displayNotMatch();
  }
  
  if (ControllerInfo.gameState == 1) { // Entry
    displayMatched();
  }
  
  if (ControllerInfo.gameState == 2) { // Countdown
    displayCountdown();
    ControllerInfo.gameState = 3; // Force state to Playing, just in case that state from Controller1 not recieved in time
    
  }
  
  if (ControllerInfo.gameState == 3) {
    Playing();
  }
  
}

