/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 *
 * Copyright (c) 2023 mobizt
 *
 */

/** This example will show how to access the RTDB in Test Mode (no authentication).
 */
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "BELL545"
#define WIFI_PASSWORD "ammuyadu916"

/* 2. Define the RTDB URL */
#define DATABASE_URL "https://parking-management-syste-7ead5-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

#define API_KEY "AIzaSyD7OHTKAEGgTzZJbGEghgwK0zvlyVJeQVE"

/* 3. Define the Firebase Data object */
FirebaseData fbdo;

/* 4, Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* Define the FirebaseConfig data for config data */
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOk = false;

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
}

void loop()
{
    if (Firebase.ready() && signupOk && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();

      if (Firebase.RTDB.setInt(&fbdo, "test/int", count)) {
        Serial.println("PASSED");
        Serial.printf("PATH: %s\n", fbdo.dataPath());
        Serial.printf("TYPE: %s\n", fbdo.dataType());
      }
      else {
        Serial.println("FAILED");
        Serial.printf("REASON: %s\n", fbdo.errorReason());
      }


      if (Firebase.RTDB.setFloat(&fbdo, "test/float", 0.01 + random(0, 100))) {
        Serial.println("PASSED");
        Serial.printf("PATH: %s\n", fbdo.dataPath());
        Serial.printf("TYPE: %s\n", fbdo.dataType());
      }
      else {
        Serial.println("FAILED");
        Serial.printf("REASON: %s\n", fbdo.errorReason());
      }
      
    }

    
    
}