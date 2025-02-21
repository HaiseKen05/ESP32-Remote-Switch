// Master
#include <esp_now.h>
#include <WiFi.h>

#define BUTTON_PIN 4  // Physical button GPIO

uint8_t receiverMAC[] = {0xCC, 0xDB, 0xA7, 0x99, 0x70, 0xC8};  // Update with Receiver MAC

typedef struct struct_message {
    bool lockState;  // true = LOCK, false = UNLOCK
} struct_message;

struct_message myData;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Delivery Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    pinMode(BUTTON_PIN, INPUT_PULLUP);  // Enable internal pull-up resistor

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW Init Failed");
        return;
    }

    esp_now_register_send_cb(OnDataSent);
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, receiverMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }
}

void loop() {
    if (digitalRead(BUTTON_PIN) == LOW) {  // Button pressed
        myData.lockState = !myData.lockState;  // Toggle lock state
        esp_err_t result = esp_now_send(receiverMAC, (uint8_t *)&myData, sizeof(myData));

        Serial.print("Sent: ");
        Serial.println(myData.lockState ? "LOCK" : "UNLOCK");

        delay(500);  // Debounce delay
        while (digitalRead(BUTTON_PIN) == LOW);  // Wait for button release
    }
}
