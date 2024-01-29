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
#define WIFI_PASSWORD "gangshit"

/* 2. Define the RTDB URL */
#define DATABASE_URL "https://parking-management-syste-7ead5-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define API_KEY "AIzaSyD7OHTKAEGgTzZJbGEghgwK0zvlyVJeQVE"
#define LED_PIN 32
#define SERVO_1_PIN 16
#define BUTTON_1 21

/* 3. Define the Firebase Data object */
FirebaseData fbdo;

/* 4, Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* Define the FirebaseConfig data for config data */
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

Servo myservo;

int count = 0;
int ledState = false;
boolean signupOk = false;
const int parking_spots = 3;
bool vehicle_detected = false;

std::vector<std::string> parking_spot_variables = {"ledPin", "ledState", "parkingSpotState"}; 
void initFirebaseDatabase();


// Sample variables 
// TO BE REMOVED
int last_state = HIGH;
int current_state;


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
    pinMode(BUTTON_1, INPUT_PULLUP);
    ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	myservo.setPeriodHertz(50);    // standard 50 hz servo
	myservo.attach(SERVO_1_PIN, 500, 2400); // attaches the servo on pin 18 to the servo object
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
  return EXIT_FAILURE;
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

bool controlGate(bool to_open) 
{
    static bool curr_gate_state;
    if (curr_gate_state) // If true, gate is open
    {
        std::cout << "Gate is open";
        if (!to_open)  // If you want to close
        {
            std::cout << " : want to close" << std::endl;
            myservo.write(90);
            for (int pos = 90; pos >= 0; pos--) 
            { 
                myservo.write(pos);    
                delay(15);             
	        }
            curr_gate_state = false;
            return true;
        }
    }
    else if(!curr_gate_state)  // If false, gate is closed
    {
        std::cout << "Gate is closed";
        if (to_open)
        {
            std::cout << " : want to open" << std::endl;
            myservo.write(0);
            for (int pos = 0; pos <= 90; pos++) 
            { 
                myservo.write(pos);    
                delay(15);             
	        }
            curr_gate_state = true;
            return true;
        }
    }
    return false;
}

void loop()
{
    
    vehicle_detected = digitalRead(BUTTON_1);
    // std::cout << vehicle_detected << std::endl;
    current_state = vehicle_detected;  // Current button state
    bool confirmation = false;  // Dummy val keeping track of QR code success

    if (last_state == LOW && current_state == HIGH)
    {
        std::cout << "Button State is ON" << std::endl;
        while(!confirmation)
        {
            controlGate(false);  // Keep gate closed until confirmation
            delay(10000);
            confirmation = true;
        }
        controlGate(true);
        delay(3000);
        controlGate(false);
    }

    last_state = current_state;
    delay(200);


    
    // delay(3000);
    // digitalWrite(SERVO_1, HIGH);
//   updateLedStateFirebase(true);
//   updateLedState(true);
//   getLedStateFirebase();


    // delay(3000);
    // digitalWrite(SERVO_1, LOW);

  
//   updateLedStateFirebase(false);
//   updateLedState(false);
//   getLedStateFirebase();    

    
    
}