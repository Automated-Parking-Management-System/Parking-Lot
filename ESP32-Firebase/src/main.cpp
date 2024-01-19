#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Yadii"
#define WIFI_PASSWORD "gangshit"

/* 2. Define the RTDB URL */
#define DATABASE_URL "https://parking-management-syste-7ead5-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

#define API_KEY "AIzaSyD7OHTKAEGgTzZJbGEghgwK0zvlyVJeQVE"

#define LED_PIN 32

/* 3. Define the Firebase Data object */
FirebaseData fbdo;

/* 4, Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* Define the FirebaseConfig data for config data */
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
int ledState = false;
boolean signupOk = false;

void setup()
{
    
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    config.api_key = API_KEY;

    config.database_url = DATABASE_URL;


    if (Firebase.signUp(&config, &auth, "", "")) {
      Serial.println("Firebase sign up success!\n");
      signupOk = true;
    }
    else {
      Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }

    config.token_status_callback = tokenStatusCallback;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    pinMode(LED_PIN, OUTPUT);
}

void ledInit() {
  
  if (Firebase.RTDB.setBool(&fbdo, "test/ledState", ledState)) {
    Serial.print("Initialized led state to ON\n");
  }
}

void setLedState(boolean state) {
  digitalWrite(LED_PIN, state);
  delay(100);
}

void updateLedStateFirebase(boolean state) {
    if (Firebase.ready() && signupOk) {
        sendDataPrevMillis = millis();

        if (Firebase.RTDB.setBool(&fbdo, "test/ledState", state)) {
          Serial.printf("Successfully updated LED state to : ");
          Serial.printf("%i\n", state);
        }
        else {
          Serial.printf("Failed to update LED state");
          Serial.printf("REASON: %s\n", fbdo.errorReason());
        }
    }
}

bool updateLedState(bool state) {
  if (!digitalPinIsValid(LED_PIN)) {
    Serial.print("Pin is not valid ?");
    return false;
  }

  Serial.print("Update led state to: ");
  Serial.printf("%i\n\n", state);
  delay(100);
  digitalWrite(LED_PIN, state);
  return true;
}

void loop()
{
  delay(3000);
  // updateLedStateFirebase(true);
  updateLedState(true);
  delay(3000);
  
  // updateLedStateFirebase(false);
  updateLedState(false);
    
    
}