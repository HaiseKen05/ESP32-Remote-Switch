// ESP-NOW Slave (Receiver) Code
#include <esp_now.h>
#include <WiFi.h>

#define RELAY_PIN 5  // GPIO for relay control

uint8_t masterAddress[] = {0xCC, 0xDB, 0xA7, 0x99, 0x70, 0xC8}; // Replace with actual MAC of the Master ESP32

// Data structure
typedef struct {
    uint8_t value;
} DataPacket;

DataPacket sendData;

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    DataPacket receivedData;
    memcpy(&receivedData, incomingData, sizeof(receivedData));
    Serial.print("Received Data: ");
    Serial.println(receivedData.value);
    
    if (receivedData.value == 0) {
        sendData.value = 1;
        esp_now_send(mac, (uint8_t *)&sendData, sizeof(sendData));
        Serial.println("Sent Acknowledgment: 1");
    } else if (receivedData.value == 2) {
        Serial.println("Activating Relay for brief moment");
        digitalWrite(RELAY_PIN, HIGH);
        delay(500);
        digitalWrite(RELAY_PIN, LOW);
    }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW initialization failed!");
        return;
    }
    esp_now_register_recv_cb(OnDataRecv);
    esp_now_register_send_cb(OnDataSent);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, masterAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }
}

void loop() {
    // Nothing here, Slave only listens and responds
}
