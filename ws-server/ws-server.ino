#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>

using namespace websockets;

// --- Motor Pin Configuration ---
const int motorA_pin1 = 12; // H-Bridge IN1 Pin
const int motorA_pin2 = 14; // H-Bridge IN2 Pin
const int motorB_pin1 = 27; // H-Bridge IN3 Pin
const int motorB_pin2 = 26; // H-Bridge IN4 Pin
// The same enable pins will be used for analogWrite
const int motorA_enable_pin = 13; // ENA on H-Bridge
const int motorB_enable_pin = 25; // ENB on H-Bridge

const int MAX_PWM_SPEED = 255;  // Max speed for analogWrite

// --- Wi-Fi Network Configuration ---
const char* ssid = "ESP32-AP";
const char* password = "12345678";

// --- WebSocket Server ---
WebsocketsServer server;

// --- Function Prototypes ---
void onMessage(WebsocketsClient &client, WebsocketsMessage message);
void onEvent(WebsocketsClient &client, WebsocketsEvent event, String data);
void driveMotorsXY(float x, float y);
void setMotorSpeed(int pin_fwd, int pin_rev, int pin_pwm, int speed);


void setup() {
  Serial.begin(115200);
  Serial.println("Starting cart system with JSON/XY (analogWrite)...");

  // Set motor direction pins as output
  pinMode(motorA_pin1, OUTPUT);
  pinMode(motorA_pin2, OUTPUT);
  pinMode(motorB_pin1, OUTPUT);
  pinMode(motorB_pin2, OUTPUT);

  // Set motor enable/pwm pins as output.
  pinMode(motorA_enable_pin, OUTPUT);
  pinMode(motorB_enable_pin, OUTPUT);
  
  Serial.println("Motor pins configured.");

  // Stop the cart on startup
  driveMotorsXY(0.0, 0.0);

  // Start Wi-Fi in Access Point (AP) mode
  WiFi.softAP(ssid, password);
  Serial.println("Access Point created successfully!");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("IP for connection: ");
  Serial.println(WiFi.softAPIP());

  // Start WebSocket server
  server.listen(8080);
  Serial.println("WebSocket server waiting on port 8080.");
}

void loop() {
  WebsocketsClient client = server.accept();
  if (client.available()) {
    client.onMessage(onMessage);
    client.onEvent(onEvent);
    while (client.available()) {
      client.poll();
      delay(10);
    }
  }
}

void onMessage(WebsocketsClient &client, WebsocketsMessage message) {
  String msg_str = message.data();
  StaticJsonDocument<100> doc;
  DeserializationError error = deserializeJson(doc, msg_str);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  float x = doc["x"] | 0.0;
  float y = doc["y"] | 0.0;
  
  Serial.printf("Command received: x=%.2f, y=%.2f\n", x, y);
  driveMotorsXY(x, y);
}

void driveMotorsXY(float x, float y) {
  int drive_pwm = (int)(-y * MAX_PWM_SPEED); 
  setMotorSpeed(motorA_pin1, motorA_pin2, motorA_enable_pin, drive_pwm);
  
  int steer_pwm = (int)(x * MAX_PWM_SPEED);
  setMotorSpeed(motorB_pin1, motorB_pin2, motorB_enable_pin, steer_pwm);
}

/**
 * @brief Sets the speed and direction of a single motor using analogWrite.
 * @param pin_fwd The "forward" direction pin for this motor.
 * @param pin_rev The "reverse" direction pin for this motor.
 * @param pin_pwm The PWM/Enable pin for this motor.
 * @param speed The desired speed, from -255 (full reverse) to 255 (full forward).
 */
void setMotorSpeed(int pin_fwd, int pin_rev, int pin_pwm, int speed) {
  if (speed > 0) { // Forward
    digitalWrite(pin_fwd, HIGH);
    digitalWrite(pin_rev, LOW);
    analogWrite(pin_pwm, speed);
  } else if (speed < 0) { // Backward
    digitalWrite(pin_fwd, LOW);
    digitalWrite(pin_rev, HIGH);
    analogWrite(pin_pwm, -speed); // Use absolute value for analogWrite
  } else { // Stop
    digitalWrite(pin_fwd, LOW);
    digitalWrite(pin_rev, LOW);
    analogWrite(pin_pwm, 0);
  }
}

void onEvent(WebsocketsClient &client, WebsocketsEvent event, String data) {
  if (event == WebsocketsEvent::ConnectionOpened) {
    Serial.println("A client has connected!");
  } else if (event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("Client disconnected. Stopping motors.");
    driveMotorsXY(0.0, 0.0);
  }
}
