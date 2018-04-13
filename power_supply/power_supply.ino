#include <Arduino.h>
#include <WebSocketsClient.h>

#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

#include <ladder.h>

#define SER D1
#define CLOCK D3
#define LATCH D2

#define LED_OFF 0
#define LED_GREEN 1
#define LED_BLUE 2
#define LED_RED 4


Ladder executor;

byte ledState[] = { LED_OFF, LED_OFF, LED_OFF, LED_OFF, LED_OFF, LED_OFF, LED_OFF, LED_OFF };

WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);

#include "actions.h";

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {


  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;

    case WStype_CONNECTED:
      Serial.printf("[WSc] Connected to url: %s\n",  payload);
      executor.addAction(new SendPowerOnAction());
      break;

    case WStype_TEXT:
      {
        Serial.printf("[WSc] get text: %s\n", payload);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(payload);
        if (root["messageType"].as<String>() == "625f0f906dca05bc6ee7") {
          for (int i = 0; i < 8; i++) {
            String var = "led";
            var = var + i;
            String colour = root["messages"][0][var];
            if (colour == "red") {
              ledState[i] = LED_RED;
            }
            if (colour == "green") {
              ledState[i] = LED_GREEN;
            }
            if (colour == "blue") {
              ledState[i] = LED_BLUE;
            }
            if (colour == "off") {
              ledState[i] = LED_OFF;
            }
          }
        }
      }

      break;
 
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      hexdump(payload, length);

      // send data to server
      // webSocket.sendBIN(payload, length);
      break;
  }

}


void setup() {
  pinMode(SER, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(LATCH, OUTPUT);
  digitalWrite(SER, LOW);
  digitalWrite(CLOCK, LOW);
  digitalWrite(LATCH, LOW);

  /*

    addAction(ACTION_SELF_TEST, micros(), 0);
  */

  executor.addAction(new ApplyLedStateAction(micros(), 1000));
  executor.addAction(new SelfTestAction());
  executor.addAction(new ScanNetworksAction(2000000));

  Serial.begin(74880);
  Serial.println("Setup done");
}

void loop() {
  executor.execute();
}
