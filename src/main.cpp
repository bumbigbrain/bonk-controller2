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


uint8_t MoleController1Address[] = {0xA4, 0xCF, 0x12, 0x8F, 0xCA, 0x28};
esp_now_peer_info_t peerInfoMoleController1;


uint8_t MoleController2Address[] = {0x24, 0x6F, 0x28, 0xD2, 0x0F, 0x9C};
esp_now_peer_info_t peerInfoMoleController2;




typedef struct ControllerMessage {
  int gameState;
  int ScorePlayer1;
  int ScorePlayer2;
  

} ControllerMessage;

typedef struct MoleMessage {
  int mole;
} MoleMessage;


MoleMessage MoleInfo;
ControllerMessage ControllerInfo;
int Controller2Score = 0;

bool canSendMole1 = true;
bool canSendMole2 = true;



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





void displayPlaying(int time_left) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("PLAYING");
  display.setCursor(11,20);
  display.printf("TIME LEFT : %d\n", time_left);
  display.display();
}



void displayScore() {

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.printf("Score Player 1 : %d\n", ControllerInfo.ScorePlayer1);
  display.setCursor(0, 20);
  display.printf("Score Player 2 : %d\n", Controller2Score);
  if (ControllerInfo.ScorePlayer1 > Controller2Score) {      
    display.setCursor(0, 30); 
    display.printf("Player 1 WIN!!!");
  } else {
    display.setCursor(0, 40); 
    display.printf("Player 2 WIN!!!");
  }
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


bool isMole1(const uint8_t *sentmac) {
  uint8_t address[6] =  {0xA4, 0xCF, 0x12, 0x8F, 0xCA, 0x28};
  for (int i=0; i<6; i++) {
    if (address[i] != sentmac[i]) {
      return false;
    }
  }
  return true;
}




bool isMole2(const uint8_t *sentmac) {

  //  change to LOLIN
  uint8_t address[6] = {0x24, 0x6F, 0x28, 0xD2, 0x0F, 0x9C}; 
  for (int i=0; i<6; i++) {
    if (address[i] != sentmac[i]) {
      return false;
    }
  }
  return true;
}



void ResetMole() {
  MoleInfo.mole = -1;
  
  esp_err_t result = esp_now_send(MoleController1Address, (uint8_t *) &MoleInfo, sizeof(MoleInfo)); 
  if (result == ESP_OK) {
    Serial.println("Send state to MoleController1 : SUCCESS");
  } else {
    Serial.println("Sent state to MoleController1 : FAILED");
  }

  result = esp_now_send(MoleController2Address, (uint8_t *) &MoleInfo, sizeof(MoleInfo)); 
  if (result == ESP_OK) {
    Serial.println("Send state to MoleController2 : SUCCESS");
  } else {
    Serial.println("Sent state to MoleController2 : FAILED");
  }

}




void UpMole1() {
  
  int mode = rand();    
  mode = mode % 4; // 0 1 2 3 4

  if (mode == 0) {
    MoleInfo.mole = 0;   
  } 

  if (mode == 1) {
    MoleInfo.mole = 1; 
  } 

  if (mode == 2) {
    MoleInfo.mole = 2;
  }

  if (mode == 3) {
    MoleInfo.mole = 3;
  }
   
  esp_err_t result = esp_now_send(MoleController1Address, (uint8_t *) &MoleInfo, sizeof(MoleInfo)); 
  if (result == ESP_OK) {
    Serial.println("Send state to MoleController1 : SUCCESS");
  } else {
    Serial.println("Sent state to MoleController1 : FAILED");
  }


}


void UpMole2() {
  // MoleInfo.mole = 1;
  // esp_err_t result = esp_now_send(MoleController1Address, (uint8_t *) &MoleInfo, sizeof(MoleInfo));
  // if (result == ESP_OK) {
  //   Serial.println("Send state to MoleController1 : SUCCESS");
  // } else {
  //   Serial.println("Send state to MoleController1 : FAILED");
  // }
  
  int mode = rand();    
  mode = mode % 4; // 0 1 2 3 4

  if (mode == 0) {
    MoleInfo.mole = 0;   
  } 

  if (mode == 1) {
    MoleInfo.mole = 1; 
  } 

  if (mode == 2) {
    MoleInfo.mole = 2;
  }

  if (mode == 3) {
    MoleInfo.mole = 3;
  }
   
  esp_err_t result = esp_now_send(MoleController2Address, (uint8_t *) &MoleInfo, sizeof(MoleInfo)); 
  if (result == ESP_OK) {
    Serial.println("Send state to MoleController2 : SUCCESS");
  } else {
    Serial.println("Sent state to MoleController2 : FAILED");
  }



}



void UpMoleMaster() {
  int rand_mole = rand();
  rand_mole = rand_mole % 2;
  if (rand_mole == 0 && canSendMole1) {
    UpMole1();
    canSendMole1 = false;
  } else if (rand_mole == 1 && canSendMole2) {
    UpMole2(); 
    canSendMole2 = false;
  }
  delay(50);
}


int rand(void);

void Playing() {
  
  // Playing for 30 sec
  int msec = 0, trigger = 30;
  clock_t before = clock();

  do {
    displayPlaying(trigger - msec);
    UpMoleMaster();


    clock_t difference = clock() - before;
    msec = difference * 1000 / CLOCKS_PER_SEC;

    
  } while (msec < trigger);

}


void sendScoreToController1() {
  
  esp_err_t result = esp_now_send(Controller1Address, (uint8_t *) &ControllerInfo, sizeof(ControllerInfo));
  if (result == ESP_OK) {
    Serial.println("Send state to Controller1 : SUCCESS");
  } else {
    Serial.println("Send state to Controller1 : FAILED");
  }

  
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
  } else if (isMole1(mac) && ControllerInfo.gameState == 3) {
    canSendMole1 = true;
    Controller2Score++;
  } else if (isMole2(mac) && ControllerInfo.gameState == 3) {
    canSendMole2 = true;
    Controller2Score++;
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


  //register peer MoleController1
  memcpy(peerInfoMoleController1.peer_addr, MoleController1Address, 6);
  peerInfoMoleController1.channel = 0;
  peerInfoMoleController1.encrypt = false;
  esp_now_add_peer(&peerInfoMoleController1);
  if (esp_now_add_peer(&peerInfoMoleController1) != ESP_OK) { 
    Serial.println("Failed to add peer MoleController1");
  }
  
  //register peer MoleController2
  memcpy(peerInfoMoleController2.peer_addr, MoleController2Address, 6);
  peerInfoMoleController2.channel = 0;
  peerInfoMoleController2.encrypt = false;
  esp_now_add_peer(&peerInfoMoleController2);
  if (esp_now_add_peer(&peerInfoMoleController2) != ESP_OK) {
    Serial.println("Failed to add peer MoleController1");
  }
  

}

void loop() {
  
  if (ControllerInfo.gameState == 0) { // No matching
    displayNotMatch();
  }
  
  if (ControllerInfo.gameState == 1) { // Entry
    Controller2Score = 0;
    displayMatched();
  }
  
  if (ControllerInfo.gameState == 2) { // Countdown
    displayCountdown();
    ControllerInfo.gameState = 3; // Force state to Playing, just in case that state from Controller1 not recieved in time
    
  }
  
  if (ControllerInfo.gameState == 3) { // Playing 
    Playing();
    ControllerInfo.gameState = 4;
    
  }
  
  if (ControllerInfo.gameState == 4) { // Exchange Score
    // send  score to tubtab Controller 1
    ControllerInfo.ScorePlayer2 = Controller2Score;
    sendScoreToController1();
    // Don't forget to clear score
    //displayScore();
    
  } 

  if (ControllerInfo.gameState == 5) {
    displayScore();
  }

  if (ControllerInfo.gameState == 6) {
    ResetMole();
    ControllerInfo.gameState = 1;
  }
  
}

