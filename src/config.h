#ifndef CONFIG_H
#define CONFIG_H

// ====== WiFi Configuration ======
// ESP32 Access Point credentials (connect to this WiFi from your phone)
const char *WIFI_SSID = "ESP32";
const char *WIFI_PASSWORD = "12345678";

// Home WiFi credentials (for internet/Flask server connection)
// TODO: ENTER YOUR LAPTOP HOTSPOT NAME AND PASSWORD HERE
const char *HOME_WIFI_SSID = "riyospace";
const char *HOME_WIFI_PASSWORD = "02242005";

// ====== Flask Server Configuration ======
// Change this to your laptop's IP address
// Find it by running 'ipconfig' in Windows (look for IPv4 Address)
const char *FLASK_SERVER_IP = "10.242.232.87";
const int FLASK_SERVER_PORT = 5000;

// ====== WebSocket Configuration ======
const int WEBSOCKET_PORT = 80;
const char *WEBSOCKET_PATH = "/ws";

// ====== Timeout Configuration ======
const unsigned long DISCONNECT_TIMEOUT = 60000; // 60 seconds
const unsigned long WIFI_TIMEOUT = 20000;       // 20 seconds for WiFi connection

#endif
