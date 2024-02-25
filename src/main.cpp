#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

//define display
// SDA -> GPIO 21
// SCL -> GPIO 22
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t targetAddress1[] = {0x3C, 0x61, 0x05, 0x03, 0xC3, 0x78};
uint8_t targetAddress2[] = {0xE8, 0xDB, 0x84, 0x00, 0xDC, 0xF0};
uint8_t targetAddress3[] = {0xA4, 0xCF, 0x12, 0x8F, 0xCA, 0x28};



// GunController's message
typedef struct startGame {
  bool start;
} startGame;

// Target's message
typedef struct targetMessage {
  int targetState;
} targetMessage;

startGame command;
targetMessage target_message;


// define peer
esp_now_peer_info_t peerInfoTarget1;
esp_now_peer_info_t peerInfoTarget2;
esp_now_peer_info_t peerInfoTarget3;


// state0: entry, state1: playing, state2: timeout
int gameState = 0; 
// restart game
bool restart = false;
int score = 0;

void displayEntry() {
  display.clearDisplay();
  display.setTextSize(1); 
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("DOWNING ALL TARGET, PRESS START BUTTON TO START THE GAME");
  display.display();
}



void displayPlaying(int timer_value) {
  display.clearDisplay();
  display.setTextSize(1);  
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("PLAYING");
  display.setCursor(11, 20);
  display.printf("TIME LEFT : %d\n", timer_value);
  display.display();
}


void displayTIMEOUT() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("TIME OUT !!");
  display.setCursor(11, 20);
  display.printf("SCORE : %d", score);
  display.display();
  
  
}

void Reset() {
  // UP ALL TARGET
  // targetState = 1 -> UP 
  target_message.targetState = 1;

  // UP Target1
  esp_err_t resultUpTarget1 = esp_now_send(targetAddress1, (uint8_t *) &target_message, sizeof(target_message));
  if (resultUpTarget1 == ESP_OK) {
    Serial.println("Up Target1 with success");
  } else {
    Serial.println("Up Target1 failed");
  }

  // UP Target2
  esp_err_t resultUpTarget2 = esp_now_send(targetAddress2, (uint8_t *) &target_message, sizeof(target_message));
  if (resultUpTarget2 == ESP_OK) {
    Serial.println("Up Target2 with success");
  } else {
    Serial.println("Down Target2 failed");
  }

  // UP Target3
  esp_err_t resultUpTarget3 = esp_now_send(targetAddress3, (uint8_t *) &target_message, sizeof(target_message));
  if (resultUpTarget3 == ESP_OK) {
    Serial.println("Up Target3 with success"); 
  } else {
    Serial.println("Down Target3 failed");
  }

  // reset score
  score = 0;
}

void DownAllTarget() {
  // targetState = 0 -> DOWN  
  target_message.targetState = 0;

  
  // Down Target1
  esp_err_t resultDownTarget1 = esp_now_send(targetAddress1, (uint8_t *) &target_message, sizeof(target_message));
  if (resultDownTarget1 == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data");
  }

  
   // Down Target2
  esp_err_t resultDownTarget2 = esp_now_send(targetAddress2, (uint8_t *) &target_message, sizeof(target_message));
  if (resultDownTarget2 == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data");
  }

 // Down Target3
  esp_err_t resultDownTarget3 = esp_now_send(targetAddress3, (uint8_t *) &target_message, sizeof(target_message));
  if (resultDownTarget3 == ESP_OK) {
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


void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&command, incomingData, sizeof(command));
  
  if (isGunController(mac)) { // GunController's message
    if (gameState == 0) {
      gameState = 1;   
      Serial.println("STARTING... -> UP TARGET, RESTARTED TIMER, RESTARTED SCORE");  
    } else if (gameState == 1) {
      restart = true;
      Serial.println("RESTARTING... -> UP TARGET, RESTARTED TIMER, RESTARTED SCORE");    
    }
    Reset();
  } else { 
    if (gameState == 1) { // Target's message
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

  //Display setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }


  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Do OnDataSent when send
  esp_now_register_send_cb(OnDataSent);
  // Do OnDataRecv when recieved 
  esp_now_register_recv_cb(OnDataRecv);


  // Registering peer  
  // Register target 1
  memcpy(peerInfoTarget1.peer_addr, targetAddress1, 6);
  peerInfoTarget1.channel = 0;
  peerInfoTarget1.encrypt = false;
  esp_now_add_peer(&peerInfoTarget1);
  if (esp_now_add_peer(&peerInfoTarget1) != ESP_OK){
    Serial.println("Failed to add Target1");
  }

  // Register target 2
  memcpy(peerInfoTarget2.peer_addr, targetAddress2, 6);
  peerInfoTarget2.channel = 0;
  peerInfoTarget2.encrypt = false;
  esp_now_add_peer(&peerInfoTarget2);
  if (esp_now_add_peer(&peerInfoTarget2) != ESP_OK) {
    Serial.println("Failed to add Target2");
  }

  // Register target 3
  memcpy(peerInfoTarget3.peer_addr, targetAddress3, 6);
  peerInfoTarget3.channel = 0;
  peerInfoTarget3.encrypt = false; 
  esp_now_add_peer(&peerInfoTarget3);
  if (esp_now_add_peer(&peerInfoTarget3) != ESP_OK) {
    Serial.println("Failed to add Target3");
  }

  

  
}
 
void loop() {
  if (gameState == 0) {
    displayEntry(); 
    DownAllTarget();   

  } else if (gameState == 1) {
    
    // PLAYING FOR 5 SEC
    int msec = 0, trigger = 5;
    clock_t before = clock();
     
    do {
      
      if (restart == true) {
        gameState = 1;
        restart = false;
        break;
      }

      displayPlaying(trigger - msec);

      clock_t difference = clock() - before;
      msec = difference * 1000 / CLOCKS_PER_SEC;
      //Serial.printf("msec : %d\n", msec);
    
    } while (msec < trigger);

    if (msec >= trigger) {
      //Serial.println("TIMEOUT");
      gameState = 2;
    }


  } else if (gameState == 2) {
    displayTIMEOUT();
    gameState = 0;
    delay(3000);
    
  }
  delay(1000);
  

}
