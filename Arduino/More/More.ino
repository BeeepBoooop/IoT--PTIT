#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <time.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>

#define RST_PIN    22    
#define SS_PIN     21    

// WiFi credentials
#define WIFI_SSID "clome"
#define WIFI_PASSWORD "123123123"

#define API_KEY "AIzaSyCUDkgzugaNNcNxt77jG6h3e5pO9B8jnDg"
#define FIREBASE_HOST "esp-firebase-3a6d8-default-rtdb.firebaseio.com"
#define USER_EMAIL "nva2kk8@gmail.com"
#define USER_PASSWORD "Nva23802."

LiquidCrystal_I2C lcd(0x27, 16, 2);

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create an MFRC522 object

WiFiClient client;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String uidString = "";
String uid = "";
String timeStart = "";

Servo servo1;
Servo servo2;

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void connectFirebase() {
  config.host = FIREBASE_HOST;
  config.api_key = API_KEY;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.setMaxRetry(fbdo, 3);
  Firebase.setMaxErrorQueue(fbdo, 30);
  Firebase.enableClassicRequest(fbdo, true);

  fbdo.setBSSLBufferSize(4096, 1024);
  fbdo.setResponseSize(2048);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(25, 33);
  
  connectToWiFi();
  connectFirebase();
  configTime(7 * 3600, 0, "vn.pool.ntp.org", "time.nist.gov");

  // // Initialize the LCD
  lcd.begin(16, 2);  
  lcd.setBacklight(255);

  lcd.setCursor(0, 0);
  lcd.print("SCAN CARD...");

  servo1.attach(32);
  servo2.attach(15);

  SPI.begin();      
  mfrc522.PCD_Init(); 
  Serial.println("Waiting for an RFID card...");
}

void getDataSensor() {
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidString += String(mfrc522.uid.uidByte[i], HEX);
  }

  Serial.print("RFID Card UID: ");
  Serial.println(uidString);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void getUidFromFirebase() {
  FirebaseJson json;

  if (Firebase.getJSON(fbdo, "/rfid_data")) {
    if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_json) {
      json = fbdo.to<FirebaseJson>();

      FirebaseJsonData jsonData;
      
      // Use the get method to retrieve values from the JSON object
      if (json.get(jsonData, "Uid")) {
        uid = jsonData.to<String>();
      } else {
        Serial.println("Failed to get 'Uid' key from JSON");
      }

      if (json.get(jsonData, "TimeStart")) {
        timeStart = jsonData.to<String>();
      } else {
        Serial.println("Failed to get 'TimeStart' key from JSON");
      }
      
      Serial.println("UID: " + uid);
      Serial.println("TimeStart: " + timeStart);
    } else {
      Serial.println("Invalid data type retrieved from Firebase");
    }
  } else {
    Serial.println("Failed to retrieve node. Error: " + fbdo.errorReason());
  }
}

void sendUidToFirebase(const char *nodeName, const String &uidString, const String &time) {
  FirebaseJson json;
  json.set("Uid", uidString);

  json.set("TimeStart", time);

  if (Firebase.updateNode(fbdo, nodeName, json)) {
    Serial.println("Node updated successfully");
    Serial.println("Path: " + fbdo.dataPath());
    Serial.println("Type: " + fbdo.dataType());
    Serial.println("JSON String: " + fbdo.jsonString());
  } else {
    Serial.println("Failed to update node. Error: " + fbdo.errorReason());
  }
}

String CurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Failed to obtain time";
  }

  return String(timeinfo.tm_year + 1900) + "-" +
         String(timeinfo.tm_mon + 1) + "-" +
         String(timeinfo.tm_mday) + " " +
         String(timeinfo.tm_hour) + ":" +
         String(timeinfo.tm_min) + ":" +
         String(timeinfo.tm_sec);
}

String countDifferenceDay(String t) {
    struct tm currentTimeinfo;
    if (!getLocalTime(&currentTimeinfo)) {
        return "Failed to obtain current time";
    }

    struct tm givenTimeinfo = {0};
    sscanf(t.c_str(), "%d-%d-%d %d:%d:%d",
           &givenTimeinfo.tm_year, &givenTimeinfo.tm_mon, &givenTimeinfo.tm_mday,
           &givenTimeinfo.tm_hour, &givenTimeinfo.tm_min, &givenTimeinfo.tm_sec);

    givenTimeinfo.tm_year -= 1900; 
    givenTimeinfo.tm_mon -= 1;     
    time_t currentTime = mktime(&currentTimeinfo);
    time_t givenTime = mktime(&givenTimeinfo);

    double difference = difftime(givenTime, currentTime);

    int daysDifference = abs(static_cast<int>(difference / (24 * 60 * 60)));

    if(daysDifference == 0) {
      return String(1);
    }

    return String("Difference in days: ") + String(daysDifference);
}

void reset(){
  uid = "";
  uidString = "";
  timeStart = "";
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SCAN CARD...");
}

void autoServo(Servo& servo) {
  servo.write(180);
  delay(2000);
  servo.write(90);  
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    getUidFromFirebase();
    if (uid.equals("null")) {
      getDataSensor();
      sendUidToFirebase("/rfid_data", uidString, CurrentTime());
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Login successfully!");
      lcd.setCursor(0, 1);
      lcd.print("Uid: " + uidString);
      delay(500);
      autoServo(servo1); 
      delay(5000);
      reset();
      Serial.println("Đăng nhập thành công!");
    }
    else {
      getDataSensor();
      if(uid.equals(uidString)) {
        Serial.println("Đăng xuất thành công!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Logout successfully!");
        delay(2000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Difference day:");
        lcd.setCursor(0, 1);
        lcd.print(countDifferenceDay(timeStart));
        Serial.println(countDifferenceDay(timeStart));
        sendUidToFirebase("/rfid_data", "null", "null");
        delay(1000);
        autoServo(servo2); 
        delay(5000);
        reset();
      }
      else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Scan Error");
        Serial.println("Lỗi thẻ! Hãy nhập lại thẻ!");
        delay(5000);
        reset();
      }
    }
  }
}
