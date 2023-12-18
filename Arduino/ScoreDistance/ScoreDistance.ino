#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
// #include <SPI.h>
// #include <MFRC522.h>

// WiFi credentials
#define WIFI_SSID "clome"
#define WIFI_PASSWORD "123123123"

// Firebase credentials
#define API_KEY "AIzaSyCUDkgzugaNNcNxt77jG6h3e5pO9B8jnDg"
#define FIREBASE_HOST "esp-firebase-3a6d8-default-rtdb.firebaseio.com"
#define USER_EMAIL "nva2kk8@gmail.com"
#define USER_PASSWORD "Nva23802."

// Delay constants
#define SETUP_DELAY 2000
#define LOOP_DELAY 100

WiFiClient client;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// #define RST_PIN    22    
// #define SS_PIN     21    

// MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create an MFRC522 object

class HCSR04 {
private:
  int trigPin;
  int echoPin;

public:
  HCSR04(int trig, int echo) : trigPin(trig), echoPin(echo) {
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
  }

  float measureDistance() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH);
    return duration * 0.034 / 2;
  }
};

// Use the HCSR04 class
HCSR04 sensor1(4, 5);
HCSR04 sensor2(19, 18);
HCSR04 sensor3(12, 13);

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

void setup() {
  Serial.begin(115200);
  delay(SETUP_DELAY);

  connectToWiFi();

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

  // SPI.begin();      
  // mfrc522.PCD_Init(); 
  // Serial.println("Waiting for an RFID card...");
}

void updateFirebaseNode(const char *nodeName, HCSR04 &sensor) {
  float distance = sensor.measureDistance();
  FirebaseJson json;
  json.set("Value", distance);

  if (Firebase.updateNode(fbdo, nodeName, json)) {
    Serial.println("Node updated successfully");
    Serial.println("Path: " + fbdo.dataPath());
    Serial.println("Type: " + fbdo.dataType());
    Serial.println("JSON String: " + fbdo.jsonString());
  } else {
    Serial.println("Failed to update node. Error: " + fbdo.errorReason());
  }
  delay(LOOP_DELAY);
}

void loop() {
    updateFirebaseNode("/Distance_1", sensor1);
    updateFirebaseNode("/Distance_2", sensor2);
    updateFirebaseNode("/Distance_3", sensor3);
  
}
