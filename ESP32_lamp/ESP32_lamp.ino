#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "esp32-hal-ledc.h"

void startBreathingEffect(uint16_t* pSetBrightness);

// === BLE Settings ===
#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define DEFAULT_BRIGHTNESS 3500

uint8_t set_brightness = 0;
uint16_t dutyCycle = 0;
bool flag_set_breathe = 0;
//--------------------------------CMD IDs----------------------------
#define STATIC_OP   120
#define BREATHE_OP  140
//-------------------------------------------------------------------
// === PWM Settings ===
const int pwmPin = 21;     // Change as needed
const int pwmChannel = 0;
const int pwmFreq = 5000;  // 5 kHz
const int pwmResolution = 12; // 12-bit (max-4095)

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) override {
    Serial.println("rcvd something");

    String rxStr = pCharacteristic->getValue();
    if (rxStr.isEmpty()) return;

    // Copy std::string to C-style char array
    char buffer[32];
    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, rxStr.c_str(), sizeof(buffer) - 1);

    uint8_t command = 0;
    uint8_t set_brightness = 0;

    // Parse using sscanf
    int parsed = sscanf(buffer, "%hhu,%hhu", &command, &set_brightness);
    if (parsed != 2) {
      Serial.println("Mort: Invalid format, send like '1,75'");
      return;
    }

    set_brightness = constrain(set_brightness, 0, 100);

    switch (command) {
      case STATIC_OP:
        flag_set_breathe = 0;
        dutyCycle = map(set_brightness, 0, 100, 0, 4095);
        ledcWrite(pwmPin, dutyCycle);
        if (dutyCycle >= 4050) {
          Serial.println("King Julien: O my god Mort. It is too bright...");
        }
        Serial.printf("Received %d -> PWM: %d\n", set_brightness, dutyCycle);
        break;

      case BREATHE_OP:
        flag_set_breathe = 1;
        dutyCycle = map(set_brightness, 0, 100, 0, 4095);
        Serial.println("King Julien: We need to breathe Mort! Breeaathee!!");
        break;

      default:
        Serial.println("Mort: Oh King Juuulieennn! I like feet!!");
    }
  }
};


void setup() {
  Serial.begin(115200);
  delay(1000);
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
  if(flag_set_breathe == 1)
  {
    startBreathingEffect(&dutyCycle);
  }
}


void startBreathingEffect(uint16_t* pSetBrightness) {
  for (int i = 0; i <= *pSetBrightness; i += 8) {
    ledcWrite(pwmPin, i);
    delay(50);
  }
  delay(3598);
  for (int i = *pSetBrightness; i >= 0; i -= 8) {
    ledcWrite(pwmPin, i);
    delay(70);
  }
}

