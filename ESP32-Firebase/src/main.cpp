#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <iostream>

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
const int parking_spots = 3;
std::vector<std::string> parking_spot_variables = {"ledPin", "ledState", "parkingSpotState"}; 
void initFirebaseDatabase();


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


void initFirebaseDatabase() {
  std::string base_parking_spot_db_path = "test/parking_spot_";
  

  for (int i; i < parking_spots; ++i) {
    std::string curr_parking_spot_db_path =  base_parking_spot_db_path.append(std::to_string(i));
    for (std::string variable: parking_spot_variables) {

      if (Firebase.ready() && signupOk) {
        
      }
    }    

    
  } 
}

bool getLedStateFirebase() {
  if (Firebase.ready() && signupOk) {
    if (Firebase.RTDB.getBool(&fbdo, "test/ledState")) {
      Serial.printf("Current led state is: %i\n", fbdo.boolData());
      return fbdo.boolData();
    }
    else {
      EXIT_FAILURE;
    }
  }
}

void initFirebaseDatabase() {
  std::string base_parking_spot_db_path = "test/parking_spot_";
  

  for (int i; i < parking_spots; ++i) {
    std::string curr_parking_spot_db_path =  base_parking_spot_db_path.append(std::to_string(i));
    

    // if (Firebase.ready() && signupOk) {

        // if (Firebase.RTDB.s(&fbdo, curr_parking_spot_db_path, state)) {
        //   Serial.printf("Successfully updated LED state to : ");
        //   Serial.printf("%i\n", );
        // }
        // else {
        //   Serial.printf("Failed to update LED state");
        //   Serial.printf("REASON: %s\n", fbdo.errorReason());
        // }
    // }
  } 
}

void setLedState(boolean state) {
  digitalWrite(LED_PIN, state);
  delay(100);
}

void updateLedStateFirebase(boolean state) {
    if (Firebase.ready() && signupOk) {

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

  updateLedStateFirebase(true);
  updateLedState(true);
  getLedStateFirebase();

  delay(3000);
  
  updateLedStateFirebase(false);
  updateLedState(false);
  getLedStateFirebase();    
    
}