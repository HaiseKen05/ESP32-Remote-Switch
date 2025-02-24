// ESP-NOW Master (Sender) Code
#include <esp_now.h>
#include <WiFi.h>

#define BUTTON_PIN 4   // GPIO for sending 0
#define RELAY_BUTTON_PIN 22 // GPIO for sending relay activation signal
#define LED_PIN 2      // GPIO for status indication (optional)
uint8_t slaveAddress[] = {0xEC, 0x64, 0xC9, 0x7C, 0xD6, 0x2C}; // Replace with the actual MAC of the Slave ESP32

// Data structure to send
typedef struct {
    uint8_t value;
} DataPacket;

DataPacket sendData;
unsigned long lastButtonPress = 0;
unsigned long lastRelayPress = 0;
const int debounceDelay = 300;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    DataPacket receivedData;
    memcpy(&receivedData, incomingData, sizeof(receivedData));
    if (receivedData.value == 1) {
        digitalWrite(LED_PIN, HIGH); // Turn on LED to indicate acknowledgment
        Serial.println("Acknowledgment received from Slave!");
        delay(500);
        digitalWrite(LED_PIN, LOW);
    }
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(RELAY_BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW initialization failed!");
        return;
    }
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, slaveAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }
}

void loop() {
    if (digitalRead(BUTTON_PIN) == LOW && millis() - lastButtonPress > debounceDelay) {
        lastButtonPress = millis();
        sendData.value = 0;
        esp_err_t result = esp_now_send(slaveAddress, (uint8_t *)&sendData, sizeof(sendData));
        if (result == ESP_OK) {
            Serial.println("Sent: 0 to Slave");
        } else {
            Serial.println("Send failed");
        }
    }
    if (digitalRead(RELAY_BUTTON_PIN) == LOW && millis() - lastRelayPress > debounceDelay) {
        lastRelayPress = millis();
        sendData.value = 2; // Signal for relay activation
        esp_err_t result = esp_now_send(slaveAddress, (uint8_t *)&sendData, sizeof(sendData));
        if (result == ESP_OK) {
            Serial.println("Sent: Relay Activation Signal to Slave");
        } else {
            Serial.println("Send failed");
        }
    }
}
