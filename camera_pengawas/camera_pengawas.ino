/*
  DIMODIF TANGGAL : 03-05-2022
  IRFAN ARDIANSYAH
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>

//const char* ssid = "Irfan.A";
//const char* password = "irfan0204";

WiFiManager wifi;
// Initialize Telegram BOT
String BOTtoken = "5195463460:AAG9md0otaoP37pUi1130931TIMAFp-ZOlI";  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
String CHAT_ID = "5310962025";

bool sendPhoto = false;

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

///////////regestrasi pin device/////////
#define relay 2
#define pir_1 13
#define pir_2 15
#define indikator 12

byte pir1 = 0;
byte pir2 = 0;
bool stateLam = false;
bool statePir1 = false;
bool statePir2 = false;
bool stateInd = false;
bool stateSensor = false;
bool stateSecurity = false;
bool stateMode = false;
bool stateReset = false;
bool stateI = false;

unsigned long Sindi = 0;
unsigned long SLedV = 0;
unsigned long SRecon = 0;
unsigned long SBack = 0;
byte Dindi = 1000;
byte DRecon = 1000;
byte DBack = 1000;
byte batas = 30;
int counter = 0;
int timer = 0;
int countWifi = 0;
bool connectWIFI;

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

//CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


void configInitCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  // Drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_CIF);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
  s->set_hmirror(s, 1);
}

void handleNewMessages(int numNewMessages) {
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;
    if (text == "/start") {
      String welcome = "Welcome , " + from_name + "\n";
      welcome += "Tekan perintah dibawah ini untuk memakai fitur dalam perangkat \n";
      welcome += "/photo : mengambil foto \n";
      welcome += "/security_ON : menyalakan fitur keamanan \n";
      welcome += "/security_OFF : mematikan fitur keamanan \n";
      welcome += "/lampu_ON : menyalakan lampu \n";
      welcome += "/lampu_OFF : mematikan lampu \n";
      welcome += "/mode_pagi : menyalakan mode pagi \n";
      welcome += "/mode_malam : menyalakan mode malam \n";
      welcome += "/RESET \n";

      bot.sendMessage(CHAT_ID, welcome, "");
    }
    /////blok eksekusi perintah
    if (text == "/photo") {
      sendPhoto = true;
      Serial.println("New photo request");
    }

    if (text == "/lampu_ON") {
      stateLam = true;
      Serial.println("lampu ON");
      bot.sendMessage(CHAT_ID, "lampu dinyalakan", "");
    }

    if (text == "/lampu_OFF") {
      stateLam = false;
      Serial.println("lampu OFF");
      bot.sendMessage(CHAT_ID, "lampu dimatikan", "");
    }

    if (text == "/security_ON") {
      stateSecurity = true;
      Serial.println("SYSTEM SECURITY AKTIF");
      bot.sendMessage(CHAT_ID, "SYSTEM SECURITY AKTIF", "");
    }

    if (text == "/security_OFF") {
      stateSecurity = false;
      Serial.println("SYSTEM SECURITY NON AKTIF");
      bot.sendMessage(CHAT_ID, "SYSTEM SECURITY NON AKTIF", "");
    }

    if (text == "/mode_pagi") {
      stateMode = true;
      Serial.println("mode pagi");
      bot.sendMessage(CHAT_ID, "MODE PAGI AKTIF", "");
    }

    if (text == "/mode_malam") {
      stateMode = false;
      Serial.println("mode malam");
      bot.sendMessage(CHAT_ID, "MODE MALAM AKTIF", "");
    }

    if (text == "/RESET") {
      stateReset = true;
    }
    /////////////done///////////////
  }
}


String sendPhotoTelegram() {
  const char* myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";

  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }

  Serial.println("Connect to " + String(myDomain));


  if (clientTCP.connect(myDomain, 443)) {
    Serial.println("Connection successful");

    String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + CHAT_ID + "\r\n--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--RandomNerdTutorials--\r\n";

    uint16_t imageLen = fb->len;
    uint16_t extraLen = head.length() + tail.length();
    uint16_t totalLen = imageLen + extraLen;

    clientTCP.println("POST /bot" + BOTtoken + "/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials");
    clientTCP.println();
    clientTCP.print(head);

    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n = 0; n < fbLen; n = n + 1024) {
      if (n + 1024 < fbLen) {
        clientTCP.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen % 1024 > 0) {
        size_t remainder = fbLen % 1024;
        clientTCP.write(fbBuf, remainder);
      }
    }

    clientTCP.print(tail);

    esp_camera_fb_return(fb);

    int waitTime = 10000;   // timeout 10 seconds
    long startTimer = millis();
    boolean state = false;

    while ((startTimer + waitTime) > millis()) {
      Serial.print(".");
      delay(100);
      while (clientTCP.available()) {
        char c = clientTCP.read();
        if (state == true) getBody += String(c);
        if (c == '\n') {
          if (getAll.length() == 0) state = true;
          getAll = "";
        }
        else if (c != '\r')
          getAll += String(c);
        startTimer = millis();
      }
      if (getBody.length() > 0) break;
    }
    clientTCP.stop();
    Serial.println(getBody);
  }
  else {
    getBody = "Connected to api.telegram.org failed.";
    Serial.println("Connected to api.telegram.org failed.");
  }
  return getBody;
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  // Init Serial Monitor
  Serial.begin(115200);

  pinMode(relay, OUTPUT);
  pinMode(pir_1, INPUT);
  pinMode(pir_2, INPUT);
  pinMode(indikator, OUTPUT);
  // Config and init the camera
  configInitCamera();

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  Serial.println();
  
  //Serial.println(ssid);
  connectWIFI = wifi.autoConnect("SMART CAMERA","00000000");
  if(!connectWIFI){
    Serial.println("NOT CONNECT IP");
    delay(100);
    digitalWrite(indikator, HIGH);
    delay(100);
    digitalWrite(indikator, LOW);
  }
  //WiFi.begin(ssid, password);
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED) {
    countWifi++;
    if (countWifi == 120) {
      ESP.restart();
    }
    //Serial.println(countWifi);
    Serial.print(".");
    delay(500);
    digitalWrite(indikator, HIGH);
    delay(500);
    digitalWrite(indikator, LOW);
  }
  Serial.print("Connecting to ");
  Serial.print(WiFi.SSID().c_str());
  bot.sendMessage(CHAT_ID, "SYSTEM CAMERA AKTIF", "");
  Serial.println();
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  Indikator();

  if (sendPhoto) {
    Serial.println("Preparing photo");
    sendPhotoTelegram();
    sendPhoto = false;
  }

  if (stateSecurity == true) {

    sensorPripare();
    //Serial.println(timer);
    if (stateSensor == true) {

      if (stateMode == true) {
        stateLam = false;
      }

      else {
        stateLam = true;
      }

      sendPhotoTelegram();
      bot.sendMessage(CHAT_ID, "OBJEK TERDETEKSI", "");
      //bot.sendMessage(CHAT_ID, "MENG-AKTIFKAN LAMPU DARURAT", "");
      TimerBack(0);
    }

    else if (stateSensor == false) {
      TimerBack(1);
      while (timer >= batas) {
        stateLam = false;
        timer = 0;
        break;
      }
    }
  }




  outLamp();
  Reconnect();
  RESET();
  //Serial.println(counter);
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

void Indikator() {
  unsigned long tmr = millis();

  if (WiFi.status() != WL_CONNECTED) {
    if (tmr - Sindi > Dindi) {
      Sindi = tmr;
      stateI = !stateI;
    }
  }

  else if (WiFi.status() == WL_CONNECTED) {
    stateI = true;
  }
}

void Reconnect() {
  unsigned long tmr = millis();

  if (WiFi.status() != WL_CONNECTED) {
    if (tmr - SRecon > DRecon) {
      SRecon = tmr;
      counter++;
      if (counter == 120) {
        Serial.println("RESTART");
        ESP.restart();
      }
    }
  }
}

void outLamp() {
  if (stateLam == true) {
    digitalWrite(relay, HIGH);
    // Serial.println("lampu");
  }
  else if (stateLam == false) {
    digitalWrite(relay, LOW);
  }

  if (stateI == true) {
    digitalWrite(indikator, HIGH);
  }
  else if (stateI == false) {
    digitalWrite(indikator, LOW);
  }
}

void sensorPripare() {
  pir1 = digitalRead(pir_1);
  pir2 = digitalRead(pir_2);

  if (pir1 == HIGH && pir2 == HIGH) {
    stateSensor = true;
  }
  if (pir1 == HIGH && pir2 == LOW) {
    stateSensor = true;
  }
  if (pir1 == LOW && pir2 == HIGH) {
    stateSensor = true;
  }
  if (pir1 == LOW && pir2 == LOW) {
    stateSensor = false;
  }

  //  Serial.print("sensor 1 :");
  //  Serial.println(pir1);
  //  Serial.print("sensor 2 :");
  //  Serial.println(pir2);
}


int TimerBack(bool state) {

  if (state == 1) {
    unsigned long tmr = millis();
    if (tmr - SBack > DBack) {
      SBack = tmr;
      timer++;

    }
  }

  else if (state == 0) {
    timer = 0;
    SBack = 0;
  }
}

void RESET() {
  if (stateReset == true) {
    bot.sendMessage(CHAT_ID, "CONTROLLER CAMERA DIRESET", "");
    delay(3000);
    ESP.restart();
  }
}
