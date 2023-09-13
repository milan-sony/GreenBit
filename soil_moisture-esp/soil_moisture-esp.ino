//NodeMCU/ESP8266
#include <ESP8266WiFi.h>

//ESP32
// #include <WiFi.h>

//Firebase
#include <Firebase_ESP_Client.h>

//Provides the token generation process info
#include "addons/TokenHelper.h"
//RTDB payload printing info and other helper functions
#include "addons/RTDBHelper.h"

//Firebase API Key
#define API_KEY "Your Firebase API Key"

//RTDB URL
#define DATABASE_URL "Your Firebase Database URL"

//Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

//WiFi Credentials
const char* WIFI_SSID = "SSID Name";
const char* WIFI_PASSWORD = "Password";


const int sensor_pin = A0;  //Connect Soil moisture analog sensor pin to A0 of ESP

void setup() {
  Serial.begin(115200); //Define baud rate for serial communication 
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //Assign the api key
  config.api_key = API_KEY;

  //Assign the RTDB URL
  config.database_url = DATABASE_URL;

  //Firebase Sign Up
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Firebase Signup is ok");
    signupOK = true;
  }
  else{
    Serial.println("Firebase Signup is not ok");
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  //Assign the callback function for the long running token generation task
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  //Begin Firebase Connection
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  float moisture_percentage;

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    //Store data in RTDB
    moisture_percentage = ( 100.00 - ( (analogRead(sensor_pin)/1023.00) * 100.00 ) );

    //Write/send Float number on the database
    if (Firebase.RTDB.setFloat(&fbdo, "soil_moisture/moisture_percentage", moisture_percentage)) {
      Serial.println(moisture_percentage);
      Serial.println("Data Sent");

      Serial.println("Data Path: " + fbdo.dataPath());
      Serial.println("Data Type: " + fbdo.dataType());

    }else{
      Serial.println("Failed to send data");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}