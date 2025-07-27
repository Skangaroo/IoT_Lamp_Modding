#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "esp32-hal-ledc.h"

// === BLE Settings ===
#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define DEFAULT_BRIGHTNESS 3500
// === PWM Settings ===
const int pwmPin = 21;     // Change as needed
const int pwmChannel = 0;
const int pwmFreq = 5000;  // 5 kHz
const int pwmResolution = 12; // 8-bit (0–255)

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) override {
    Serial.println("rcvd something");
    String value = pCharacteristic->getValue();
    if (value.length() > 0) {
      int inputValue = atoi(value.c_str());
      inputValue = constrain(inputValue, 0, 100);
      int dutyCycle = map(inputValue, 0, 100, 0, 4095);
      ledcWrite(pwmPin, dutyCycle);
      if(dutyCycle >= 4050)
      {
        Serial.println("King Julien: O my god Mort. It is too bright...");
      }
      Serial.printf("Received %d -> PWM: %d\n", inputValue, dutyCycle);
    }
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  // === PWM Setup ===
  // ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  // ledcAttachPin(pwmPin, pwmChannel);
  // ledcAttachChannel(pwmPin, pwmFreq, pwmResolution, pwmChannel);
  if(ledcAttach(pwmPin, pwmFreq, pwmResolution) != 0)
  {
    ledcWrite(pwmPin, DEFAULT_BRIGHTNESS);
  };

  // === BLE Setup ===
  BLEDevice::init("Smexy_Lamp[ESP32-S2]");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();

  Serial.println("Mort: King Julieeennn!! Smexy Lamp is ready");
}

void loop() {
  // Nothing here – BLE handles updates via callbacks
}


