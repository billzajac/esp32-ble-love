#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEHIDDevice.h>
#include <HIDKeyboardTypes.h>
#include <HIDTypes.h>
#include "secrets.h"

AsyncWebServer server(80);  // Web server object

// Variables to track Bluetooth connection count
int connectCounter = 0;      // How many times a device has connected
BLEServer* pServer;          // Pointer to the BLEServer object

// Bluetooth HID variables
BLEHIDDevice* hid;
BLECharacteristic* input;

// Define the Bluetooth device name
// const char* bluetoothName = "❤️ I LOVE YOU ❤️";
const char *bluetoothName = "🎃 Happy Halloween 💀👻";

// Function declaration (moved before the class definition)
// void typeMessage();  // Function to type the message "I love you"

// Callback class to handle BLE connection events
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    connectCounter++;  // Increment the connection counter
    Serial.printf("Device connected! Count: %d\n", connectCounter);
    
    // typeMessage();  // Type the message when a device connects

    // Wait for 1 second and then disconnect
    delay(500);  
    pServer->disconnect(0);  // Disconnect the client (clientID 0 is usually used for single clients)
    Serial.println("Disconnected after 0.5 second.");
  }

  void onDisconnect(BLEServer* pServer) override {
    Serial.println("Device disconnected.");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());

    // Start advertising again after disconnection
    BLEDevice::startAdvertising();
  }
};

// Function to simulate typing "LOVE "
// void typeMessage() {
//   Serial.println("Typing 'I love you'...");

//   const char* message = "LOVE ";
//   for (int i = 0; i < strlen(message); i++) {
//     input->setValue((uint8_t*)&message[i], 1);
//     input->notify();
//     delay(100);

//     // Release key
//     input->setValue((uint8_t*)"\x00", 1);
//     input->notify();
//     delay(100);
//   }
// }

void setup() {
  Serial.begin(115200);

  // Setup Bluetooth HID
  BLEDevice::init(bluetoothName);
  pServer = BLEDevice::createServer();  // Save the BLEServer pointer
  pServer->setCallbacks(new MyServerCallbacks());  // Set callbacks for connection events

  hid = new BLEHIDDevice(pServer);
  input = hid->inputReport(1);  // Input report (keyboard)

  hid->manufacturer()->setValue("ESP32 Keyboard");
  hid->pnp(0x01, 0x02e5, 0xabcd, 0x0110);  // Generic PnP Bluetooth device

  // Start HID service
  hid->startServices();

  // Advertise as a HID device (keyboard)
  BLEAdvertising* pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_KEYBOARD);
  pAdvertising->addServiceUUID(hid->hidService()->getUUID());
  pAdvertising->start();
  Serial.println("Bluetooth keyboard ready...");

  // Start Wi-Fi and Web Server
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Serve a webpage that shows how many times Bluetooth has connected
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<html><body>";
    html += "<h1>Bluetooth Connection Counter</h1>";
    html += "<p>Bluetooth device has connected <strong>" + String(connectCounter) + "</strong> times.</p>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  server.begin();
}

void loop() {
  // No need for anything in the loop, as we're using callbacks for connection events
}