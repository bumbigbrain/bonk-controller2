#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <time.h>

// แก้ไขค่า mac ตัวที่ต้องการส่งไปหา
// uint8_t broadcastAddress[] = {0x7C, 0x9E, 0xBD, 0x62, 0x58, 0x44};//ส่งไปหาเฉพาะ mac address
//uint8_t broadcastAddress[] = {0x3C, 0x61, 0x05, 0x03, 0xA2, 0x74};//ส่งไปหาเฉพาะ mac address

uint8_t targetAddress1[] = {0x3C, 0x61, 0x05, 0x03, 0xC3, 0x78};

typedef struct startGame {
  bool start;
} startGame;

typedef struct targetMessage {
  int targetState;
} targetMessage;

startGame command;
targetMessage target_message;
esp_now_peer_info_t peerInfoTarget1;


// state0: entry, state1: playing, state2: timeout
int gameState = 0; 
bool restart = false;
int score = 0;

void Reset() {
  // UP ALL TARGET
  target_message.targetState = 1;
  esp_err_t result = esp_now_send(targetAddress1, (uint8_t *) &target_message, sizeof(target_message));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data");
  }
  score = 0;
}

void DownAllTarget() {
  
  target_message.targetState = 0;
  esp_err_t result = esp_now_send(targetAddress1, (uint8_t *) &target_message, sizeof(target_message));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data");
  }
}

bool isGunController(const uint8_t *sentmac) {
  uint8_t test[6] = {0x3c, 0x61, 0x05, 0x03, 0xa2, 0x74};
  for (int i=0; i<6; i++){
    if (test[i] != sentmac[i]) {
      
      return false;
    }  
  }
  return true;
  
}

void showMacAddress(const uint8_t *sentmac) {
  Serial.printf("MAC : ");
  for (int i=0; i<6; i++) {
    Serial.printf("%x ", sentmac[i]);
  }
  Serial.println("");
}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println("SEND TO TARGET");
  showMacAddress(mac_addr);
  delay(300);
  
}


// เมื่อรับข้อมูลมา ให้ทำในฟังก์ชั่นนี้
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&command, incomingData, sizeof(command));
  
  if (isGunController(mac)) { 
    // sent from gun controller
    if (gameState == 0) {
      gameState = 1;   
      Serial.println("STARTING... -> UP TARGET, RESTARTED TIMER, RESTARTED SCORE");  
    } else if (gameState == 1) {
      restart = true;
      Serial.println("RESTARTING... -> UP TARGET, RESTARTED TIMER, RESTARTED SCORE");    
    }
    Reset();
  } else { // sent from target 
    if (gameState == 1) {
      score++;   
      showMacAddress(mac); 
      Serial.printf("Score : %d\n", score);
      delay(3000);
    } 

    
  }


}




 
void setup() {

  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  // เมื่อรับข้อมูลมา ให้ทำในฟังก์ชั่น OnDataRecv ที่เราสร้างไว้
  esp_now_register_recv_cb(OnDataRecv);

  memcpy(peerInfoTarget1.peer_addr, targetAddress1, 6);
  peerInfoTarget1.channel = 0;
  peerInfoTarget1.encrypt = false;
  esp_now_add_peer(&peerInfoTarget1);

  
}
 
void loop() {
  if (gameState == 0) {
    Serial.println("PRESS START BUTTON"); 
    Serial.println("DOWN ALL TARGET NOW");
    DownAllTarget();   

  } else if (gameState == 1) {
    Serial.println("PLAYING"); 
    
    // play for 5 sec
    int msec = 0, trigger = 5;
    clock_t before = clock();
     
    do {
      
      if (restart == true) {
        gameState = 1;
        restart = false;
        break;
      }

      /* Do something here
      Serial.println("RECIEVING SCORE FROM TARGET....");
      */

      clock_t difference = clock() - before;
      msec = difference * 1000 / CLOCKS_PER_SEC;
      Serial.printf("msec : %d\n", msec);
    
    } while (msec < trigger);

    if (msec >= trigger) {
      Serial.println("TIMEOUT");
      gameState = 2;
    }


  } else if (gameState == 2) {
    Serial.println("SHOWING SCORE");
    Serial.printf("YOUR SCORE IS : %d\n", score);
    gameState = 0;
    delay(4000);
    
  }
  delay(1000);
  

  

}
