#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <iostream>
#include <ESP32Servo.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Yadii"
#define WIFI_PASSWORD "secretPass123"

/* 2. Define the RTDB URL */
#define DATABASE_URL "https://parking-management-syste-86809-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define API_KEY "AIzaSyAz-WiwW6BNVyEkeq9BjMjSPPus_W90Uu0"
#define LED_PIN 32
#define SERVO_1_PIN 13
#define BUTTON_1 21

/* 3. Define the Firebase Data object */
FirebaseData fbdo;

/* 4, Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* Define the FirebaseConfig data for config data */
FirebaseConfig config;

static unsigned long sendDataPrevMillis = 0;  // This was from copy paste but it served to send requests to rtdb on a timer

static Servo myservo;  // Servo object

static int count = 0;
static int highest_gate_angle = 110;
static boolean signupOk = false;

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

    //------------------------Firebase Setup------------------------------------//

    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    
    if (Firebase.signUp(&config, &auth, "", "")) {
      Serial.println("Firebase sign up success!\n");
      signupOk = true;
    }
    else {
      Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }

    // config.token_status_callback = tokenStatusCallback;     // Bug line maybe ?

    Firebase.begin(&config, &auth);
    Firebase.reconnectNetwork(true);

    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_1, INPUT_PULLUP);
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);


    //------------------------Servo Setup---------------------------------------//

    myservo.setPeriodHertz(50);    // standard 50 hz servo
    myservo.attach(SERVO_1_PIN, 500, 2400); // attaches the servo on pin 18 to the servo object
}


void openGate() {
  myservo.write(0);
  for (int pos = 0; pos <= highest_gate_angle; pos++) {
    myservo.write(pos);
    delay(15);
  }
}

void closeGate() {
  myservo.write(highest_gate_angle);
  for (int pos = highest_gate_angle; pos >= 0; pos--) {
    myservo.write(pos);
    delay(15);
  }
}
 
void authCheck() {
  // Checks if the "Hello" QR code is ready to be authenticated

  Serial.println("inside auth Check");
  if (Firebase.RTDB.getBool(&fbdo, "QR-Code/Hello")) {
    bool authState = fbdo.boolData();

    Serial.print("Auth state: ");
    Serial.println(authState);
    if (authState == true) { // If auth is true then open gate
        openGate();
        delay(2000);
    }
    if (authState && Firebase.RTDB.setBool(&fbdo, "QR-Code/Hello", false)) { // Close gate after auth
      closeGate();
      Serial.println("Successfully updated auth state (to False)");
      return;
    }
  }

}

void loop()
{  
  authCheck();  // Continuously checks for auth from RTDB
}