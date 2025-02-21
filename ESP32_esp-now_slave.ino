// Slave
#include <esp_now.h>
#include <WiFi.h>

#define LOCK_PIN 5        // GPIO for Lock (Relay/Solenoid)
#define UNLOCK_DELAY 700 // Auto-unlock time in milliseconds (5 sec)

typedef struct struct_message {
    bool lockState;
} struct_message;

struct_message myData;
unsigned long lockActivatedTime = 0;  // Stores the timestamp of lock activation
bool isLocked = false;                // Track lock state

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
    memcpy(&myData, incomingData, sizeof(myData));

    if (myData.lockState) {
        Serial.println("Locked");
        digitalWrite(LOCK_PIN, HIGH);  // Activate Lock
        isLocked = true;
        lockActivatedTime = millis();  // Start the timer
    } else {
        Serial.println("Unlocked");
        digitalWrite(LOCK_PIN, LOW);   // Deactivate Lock
        isLocked = false;
    }
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    pinMode(LOCK_PIN, OUTPUT);
    digitalWrite(LOCK_PIN, LOW);  // Default state (Unlocked)

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW Init Failed");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
    // Check if the lock is active and the time has elapsed
    if (isLocked && (millis() - lockActivatedTime >= UNLOCK_DELAY)) {
        Serial.println("Auto-unlocking...");
        digitalWrite(LOCK_PIN, LOW);  // Turn off the lock
        isLocked = false;
    }
}
