# Parking Lot ESP32 code

This project is an **Automated Parking Management System** that utilizes **Firebase** for real-time database (RTDB) operations, weight sensors to detect parking spots occupancy, and servos to control gate operations for entry and exit of vehicles. The system authenticates users through Firebase and manages the state of the parking lot gates and spot occupancy.

This repository contains the ParkingLot.ino file which is the C++ code developed in the Arduino IDE used to run the ESP32 code which operates the gates, lights and updates the realtime database.

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Libraries Used](#libraries-used)
3. [WiFi and Firebase Setup](#wifi-and-firebase-setup)
4. [Weight Sensors](#weight-sensors)
5. [Servo Motors](#servo-motors)
6. [Main Code Functions](#main-code-functions)
    - [Initialization Functions](#initialization-functions)
    - [Firebase Functions](#firebase-functions)
    - [Weight Sensor Functions](#weight-sensor-functions)
    - [Servo Motor Functions](#servo-motor-functions)
    - [Loop Function](#loop-function)

## Prerequisites
- **Arduino** development environment.
- **ESP32** microcontroller.
- **Firebase** account with Real-Time Database (RTDB) configured.
- **WiFi** connection details.
- Components:
  - Weight Sensors (Load Cells)
  - Servo motors (for gate control)
  - LED indicators

## Libraries Used
The code relies on the following Arduino libraries:
- **WiFi.h**: For establishing WiFi connection.
- **Firebase_ESP_Client.h**: Firebase real-time database operations.
- **HX711.h**: For reading data from weight sensors (load cells).
- **ESP32Servo.h**: For controlling servos.
- **TokenHelper.h** and **RTDBHelper.h**: For managing Firebase tokens and handling RTDB responses.

## WiFi and Firebase Setup
Before running the code, insert your **WiFi credentials** and **Firebase API information** in the relevant sections:

```cpp
#define WIFI_SSID "Your-WiFi-SSID"
#define WIFI_PASSWORD "Your-WiFi-Password"
#define API_KEY "Your-Firebase-API-Key"
#define DATABASE_URL "https://your-database-url.firebaseio.com/"
#define USER_EMAIL "your-email@example.com"
#define USER_PASSWORD "your-password"
```

These values will allow the ESP32 microcontroller to connect to the Firebase RTDB and authenticate users for parking spot entry/exit.

## Weight Sensors
The code utilizes HX711 modules to read data from load cells (weight sensors). Each parking spot has an associated weight sensor to detect when a car is parked.

The calibration values for the load cells are as follows:
```cpp
const int WS1_FACTOR = 2377;
const int WS2_FACTOR = 2306;
const int WS3_FACTOR = 2329;
const int WS4_FACTOR = 2309;
```
These factors are used to ensure accurate weight readings.

## Servo Motors
Servos control the gates for entry and exit. The gate moves to different angles based on whether the vehicle is allowed to enter or exit.

The pin configuration for the servos:
```cpp
const int entrancePin = 13;
const int exitPin = 27;
```

## Main Code Functions
### Initialization Functions
`initWiFi()`
This function connects the ESP32 to the specified WiFi network. It continuously checks the connection status and prints the local IP once connected.
```cpp
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}
```
`initScales()`
Initializes the weight sensors (load cells) using their associated data pins, and sets up calibration values for accurate weight readings.
```cpp
void initScales() {
  scale1.begin(LOADCELL_DOUT_PIN_1, LOADCELL_SCK_PIN_1);
  scale2.begin(LOADCELL_DOUT_PIN_2, LOADCELL_SCK_PIN_2);
  scale3.begin(LOADCELL_DOUT_PIN_3, LOADCELL_SCK_PIN_3);
  scale4.begin(LOADCELL_DOUT_PIN_4, LOADCELL_SCK_PIN_4);
  scale1.set_scale(WS1_FACTOR);
  scale2.set_scale(WS2_FACTOR);
  scale3.set_scale(WS3_FACTOR);
  scale4.set_scale(WS4_FACTOR);
}
```
`initServo()`
Sets up the servos for gate control by attaching them to the specified pins and setting their operating frequency.
```cpp
void initServo() {
  entranceGate.setPeriodHertz(50);
  entranceGate.attach(entrancePin, 500, 2400);
  exitGate.setPeriodHertz(50);
  exitGate.attach(exitPin, 500, 2400);
}
```
### Firebase Functions
`sendBool()`
Writes a boolean value to a specified Firebase RTDB path. It is used to update the gate status and parking spot occupancy.
```cpp
bool sendBool(String path, bool value) {
  if (Firebase.RTDB.setBool(&fbdo, path.c_str(), value)) {
    return true;
  }
  return false;
}
```
`updateAuthState()`
Checks the Firebase RTDB for the latest entrance and exit authentication statuses. If valid, it allows the gate to open.
```cpp
void updateAuthState() {
  if (Firebase.RTDB.getBool(&fbdo, entrancePath)) {
    entranceAuth = fbdo.boolData();
  }
  if (Firebase.RTDB.getBool(&fbdo, exitPath)) {
    exitAuth = fbdo.boolData();
  }
}
```
### Weight Sensor Functions
`updateWeight()`
Reads the data from the weight sensors to detect whether a car is parked in a specific spot. It also updates the LED indicators to show the occupancy status.
```cpp
void updateWeight() {
  if (scale1.get_units() > CAR_WEIGHT) {
    ws1 = true;
    digitalWrite(LED_1, HIGH);
  } else {
    ws1 = false;
    digitalWrite(LED_1, LOW);
  }
}
```
### Servo Motor Functions
`openEntrance()` and `closeEntrance()`
Controls the movement of the entrance gate. The gate opens to a maximum angle of 110 degrees, then closes after a delay.
```cpp
void openEntrance() {
  for (int pos = 0; pos <= maxAngle; pos++) {
    entranceGate.write(pos);
    delay(15);
  }
}
```
Similar functions exist for closing the entrance and controlling the exit gate.

### Loop Function
The `loop()` function constantly monitors the state of the system, including:

Checking for valid Firebase authentication to open/close gates.
Reading the weight sensors to detect car presence.
Sending data to the Firebase RTDB for real-time monitoring.
```cpp
void loop() {
  updateAuthState();
  updateWeight();

  if (entranceAuth) {
    openEntrance();
    delay(1800);
    closeEntrance();
  }

  if (exitAuth) {
    openExit();
    delay(1800);
    closeExit();
  }

  if (Firebase.ready()) {
    sendBool(spotsPath + "/ws1", ws1);
  }
}
```
## Conclusion
This code provides a complete solution for automated parking management. It integrates Firebase for real-time data handling, weight sensors for detecting parking occupancy, and servos for managing entry/exit gates. With these components, the system efficiently manages parking spaces and authentication in a smart parking environment.

This `README.md` explains each logical block of the code, from initialization to main functions, giving a clear overview of how the system operates.
