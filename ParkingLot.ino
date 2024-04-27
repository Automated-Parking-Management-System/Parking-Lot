#include <Arduino.h>

/******************** Firebase RTDB Imports ********************/
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

/******************** Weight Sensor Imports ********************/
#include "HX711.h"
#include "soc/rtc.h"

/******************** Servo Imports ********************/
#include <ESP32Servo.h>

/******************** Firebase RTDB Definitions ********************/
// Insert your network credentials
#define WIFI_SSID "Taskin-Hotspot"
#define WIFI_PASSWORD "dbmw6047"

// Insert Firebase project API Key
#define API_KEY "AIzaSyC129F4OIPEJUjGMbrEu9rUYMeKoo_zBY8"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://aa05-apms-default-rtdb.firebaseio.com/" 

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "navidrahman5@gmail.com"
#define USER_PASSWORD "vrselbhdbbfclsro"

String UID = "JdeQsCeNA1T3ZQaUtQhcHGf1s343";

/******************** Firebase variables ********************/
// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// String uid;
bool signupOK = false;

unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 15000;

/******************** Servo variables ********************/
static Servo entranceGate;  // Servo object
static Servo exitGate;  // Servo object

const int entrancePin = 13;
const int exitPin = 27;

bool entranceAuth = false;
bool exitAuth = false;

String entrancePath;
String exitPath;

const int maxAngle = 110;

/******************** Weight Sensor variables ********************/
// WS1 Wiring
const int LOADCELL_DOUT_PIN_1 = 16;
const int LOADCELL_SCK_PIN_1 = 4;
// WS2 Wiring
const int LOADCELL_DOUT_PIN_2 = 17;
const int LOADCELL_SCK_PIN_2 = 5;
// WS3 Wiring
const int LOADCELL_DOUT_PIN_3 = 15;
const int LOADCELL_SCK_PIN_3 = 2;
// WS4 Wiring
const int LOADCELL_DOUT_PIN_4 = 14;
const int LOADCELL_SCK_PIN_4 = 12;

// LED Boards
const int LED_1 = 25;
const int LED_2 = 26;
const int LED_3 = 0;
const int LED_4 = 21;

// Calibration factors
const int WS1_FACTOR = 2377;
const int WS2_FACTOR = 2306;
const int WS3_FACTOR = 2329;
const int WS4_FACTOR = 2309;

const int CAR_WEIGHT = 79;

// HX711 scale init
HX711 scale1;
HX711 scale2;
HX711 scale3;
HX711 scale4;

// Weight sensor values
bool ws1;
bool ws2;
bool ws3;
bool ws4;

String spotsPath;

// Initialize Servo
void initServo() {
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  entranceGate.setPeriodHertz(50);    // standard 50 hz servo
  entranceGate.attach(entrancePin, 500, 2400); // attaches the servo on pin 18 to the servo object
  exitGate.setPeriodHertz(50);    // standard 50 hz servo
  exitGate.attach(exitPin, 500, 2400); // attaches the servo on pin 18 to the servo object
}

// Initialize Weight Sensors
void initScales(){
  scale1.begin(LOADCELL_DOUT_PIN_1, LOADCELL_SCK_PIN_1);
  scale2.begin(LOADCELL_DOUT_PIN_2, LOADCELL_SCK_PIN_2);
  scale3.begin(LOADCELL_DOUT_PIN_3, LOADCELL_SCK_PIN_3);
  scale4.begin(LOADCELL_DOUT_PIN_4, LOADCELL_SCK_PIN_4);
            
  scale1.set_scale(WS1_FACTOR);
  scale1.tare();

  scale2.set_scale(WS2_FACTOR);
  scale2.tare();

  scale3.set_scale(WS3_FACTOR);
  scale3.tare();

  scale4.set_scale(WS4_FACTOR);
  scale4.tare();
}

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Write bool values to the database
bool sendBool(String path, bool value){
  if (Firebase.RTDB.setBool(&fbdo, path.c_str(), value)){
    Serial.print("Set " + path + " to ");
    Serial.println(value);
    return true;
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
    return false;
  }
}

void setup() {
  Serial.begin(115200);

  /******************************* Setting up Servos *****************************/
  initServo();

  /******************************* Setting up Weight Sensors *****************************/

  // LED boards initiatlization
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);

  rtc_cpu_freq_config_t rtc_config;
  rtc_clk_cpu_freq_get_config(&rtc_config);
  rtc_clk_cpu_freq_to_config(RTC_CPU_FREQ_80M, &rtc_config);
  rtc_clk_cpu_freq_set_config_fast(&rtc_config);
  Serial.println("Parking Lot Demo");

  Serial.println("Initializing the scale");
  initScales();

  /**************************** Setting up Firebase ***************************/
  initWiFi();

  // Assign the api key (required)
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  // // Assign the user sign in credentials
  // auth.user.email = USER_EMAIL;
  // auth.user.password = USER_PASSWORD;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);
  Firebase.reconnectNetwork(true);

  // Getting the user UID might take a few seconds
  // Serial.println("Getting User UID");
  // while ((auth.token.uid) == "") {
  //   Serial.print('.');
  //   delay(1000);
  // }
  // // Print user UID
  // uid = auth.token.uid.c_str();
  // Serial.print("User UID: ");
  // Serial.println(uid);

  // Update paths
  spotsPath = "/" + UID + "/spots";
  entrancePath = "/" + UID + "/entranceAuth";
  exitPath = "/" + UID + "/exitAuth";
  sendBool(entrancePath, false);
  sendBool(exitPath, false);
}

void openEntrance() {
  entranceGate.write(0);
  for (int pos = 0; pos <= maxAngle; pos++) {
    entranceGate.write(pos);
    delay(15);
  }
}

void closeEntrance() {
  entranceGate.write(maxAngle);
  for (int pos = maxAngle; pos >= 0; pos--) {
    entranceGate.write(pos);
    delay(15);
  }
}

void openExit() {
  exitGate.write(0);
  for (int pos = 0; pos <= maxAngle; pos++) {
    exitGate.write(pos);
    delay(15);
  }
}

void closeExit() {
  exitGate.write(maxAngle);
  for (int pos = maxAngle; pos >= 0; pos--) {
    exitGate.write(pos);
    delay(15);
  }
}

void updateAuthState() {

  if (Firebase.ready() && signupOK) {
    if (Firebase.RTDB.getBool(&fbdo, entrancePath)) {
      entranceAuth = fbdo.boolData();
    }
  }
  if (Firebase.ready() && signupOK) {
    if (Firebase.RTDB.getBool(&fbdo, exitPath)) {
      exitAuth = fbdo.boolData();
    }
  }
}

void updateWeight() {
  if (scale1.get_units() > CAR_WEIGHT) {
    ws1 = true;
    digitalWrite(LED_1, HIGH);
  } else {
    ws1 = false;
    digitalWrite(LED_1, LOW);
  }
  
  if (scale2.get_units() > CAR_WEIGHT) {
    ws2 = true;
    digitalWrite(LED_2, HIGH);
  } else {
    ws2 = false;
    digitalWrite(LED_2, LOW);
  }

  if (scale3.get_units() > CAR_WEIGHT) {
    ws3 = true;
    digitalWrite(LED_3, HIGH);
  } else {
    ws3 = false;
    digitalWrite(LED_3, LOW);
  }

  if (scale4.get_units() > CAR_WEIGHT) {
    ws4 = true;
    digitalWrite(LED_4, HIGH);
  } else {
    ws4 = false;
    digitalWrite(LED_4, LOW);
  }
}

void rebootScales() {
  scale1.power_down();             // put the ADC in sleep mode
  scale2.power_down();             // put the ADC in sleep mode
  scale3.power_down();             // put the ADC in sleep mode
  scale4.power_down();             // put the ADC in sleep mode
  delay(1000);
  scale1.power_up();
  scale2.power_up();
  scale3.power_up();
  scale4.power_up();
}

void loop() {

  updateAuthState();
  updateWeight();

  if (entranceAuth) { // If auth is true then open gate
    openEntrance();
    delay(1800);
  }
  if (exitAuth) {
    openExit();
    delay(1800);
  }
  
  if (entranceAuth && sendBool(entrancePath, false)) { // Close gate after auth
    closeEntrance();
  }
  if (exitAuth && sendBool(exitPath, false)) { // Close gate after auth
    closeExit();
  }

  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Serial.println("Refresh token");
  }

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    
    sendBool(spotsPath + "/ws1", ws1);
    sendBool(spotsPath + "/ws2", ws2);
    sendBool(spotsPath + "/ws3", ws3);
    sendBool(spotsPath + "/ws4", ws4);
  }

  rebootScales();
}