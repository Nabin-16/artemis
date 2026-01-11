#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <map>
#include "html_content.h" // User app HTML
#include "config.h"       // WiFi and Server configuration

AsyncWebServer server(WEBSOCKET_PORT);
AsyncWebSocket ws(WEBSOCKET_PATH);

// User session management
struct UserSession
{
    String username;
    String deviceId;
    uint32_t clientId;
    unsigned long lastSeen;
    bool dataSharingEnabled;
    bool disconnectPending;
    unsigned long disconnectTime;
};

std::map<uint32_t, UserSession> activeSessions;

// Queue for data to send to Flask (to avoid blocking WebSocket handler)
#include <queue>
std::queue<String> flaskQueue;

// Queue data for Flask server (called from WebSocket handler)
void queueForFlask(const JsonDocument &doc)
{
    String jsonString;
    serializeJson(doc, jsonString);
    if (flaskQueue.size() < 10)
    { // Limit queue size
        flaskQueue.push(jsonString);
    }
}

// Actually send data to Flask server (called from loop)
void sendToFlaskServer(const String &jsonString)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("❌ Not connected to home WiFi, cannot forward to Flask");
        return;
    }

    // Serial.println("📡 Forwarding data to Flask server..."); // Reduced serial spam
    HTTPClient http;
    String serverUrl = String("http://") + FLASK_SERVER_IP + ":" + String(FLASK_SERVER_PORT) + "/api/esp32/data";

    // Serial.printf("   URL: %s\n", serverUrl.c_str());

    http.begin(serverUrl);
    http.setTimeout(1000); // Reduce timeout to 1s to prevent blocking loop
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Connection", "keep-alive"); // Attempt to keep connection alive

    int httpResponseCode = http.POST(jsonString);

    if (httpResponseCode > 0)
    {
        // Serial.printf("✅ Data forwarded to Flask: %d\n", httpResponseCode);
    }
    else
    {
        Serial.printf("❌ Error forwarding to Flask: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.printf("WebSocket client #%u connected\n", client->id());
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        if (activeSessions.count(client->id()))
        {
            UserSession &session = activeSessions[client->id()];
            session.disconnectPending = true;
            session.disconnectTime = millis();

            // Notify admin
            JsonDocument alertDoc;
            alertDoc["type"] = "USER_DISCONNECT";
            alertDoc["username"] = session.username;
            alertDoc["deviceId"] = session.deviceId;

            String alertMsg;
            serializeJson(alertDoc, alertMsg);
            ws.textAll(alertMsg);

            // Notify Flask Server about disconnection
            queueForFlask(alertDoc);
        }
    }
    else if (type == WS_EVT_DATA)
    {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, (const char *)data, len);

        if (error)
            return;

        String msgType = doc["type"] | "";

        if (msgType == "REGISTER")
        {
            UserSession session;
            session.username = doc["username"].as<String>();
            session.deviceId = doc["deviceId"].as<String>();
            session.clientId = client->id();
            session.lastSeen = millis();
            session.dataSharingEnabled = false;
            session.disconnectPending = false;

            activeSessions[client->id()] = session;

            // Send confirmation to the registering client
            JsonDocument confirmDoc;
            confirmDoc["type"] = "REGISTERED";
            String confirmMsg;
            serializeJson(confirmDoc, confirmMsg);
            client->text(confirmMsg);

            // Broadcast USER_CONNECTED to all other clients (dashboards)
            JsonDocument notifyDoc;
            notifyDoc["type"] = "USER_CONNECTED";
            notifyDoc["username"] = session.username;
            notifyDoc["deviceId"] = session.deviceId;
            notifyDoc["clientId"] = session.clientId;
            String notifyMsg;
            serializeJson(notifyDoc, notifyMsg);
            ws.textAll(notifyMsg);

            Serial.printf("User registered: %s (%s)\n", session.username.c_str(), session.deviceId.c_str());

            // Queue registration for Flask server
            queueForFlask(notifyDoc);
        }
        else if (msgType == "ENABLE_SHARING")
        {
            if (activeSessions.count(client->id()))
            {
                bool enabled = doc["enabled"].as<bool>();
                activeSessions[client->id()].dataSharingEnabled = enabled;

                Serial.printf("Data sharing %s for %s\n",
                              enabled ? "enabled" : "disabled",
                              activeSessions[client->id()].username.c_str());
            }
        }
        else if (msgType == "GPS" || msgType == "IMU")
        {
            if (activeSessions.count(client->id()))
            {
                UserSession &session = activeSessions[client->id()];
                session.lastSeen = millis();

                if (session.dataSharingEnabled)
                {
                    // Broadcast to local WebSocket clients
                    String output;
                    serializeJson(doc, output);
                    ws.textAll(output);

                    // Queue for Flask server
                    queueForFlask(doc);
                }
            }
        }
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n=== ESP32 Starting ===");

    // Scan for visible networks
    Serial.println("Scanning for WiFi networks...");
    int n = WiFi.scanNetworks();
    Serial.println("Scan done");
    if (n == 0)
    {
        Serial.println("no networks found");
    }
    else
    {
        Serial.print(n);
        Serial.println(" networks found:");
        for (int i = 0; i < n; ++i)
        {
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
            delay(10);
        }
    }

    // Setup WiFi in AP+STA mode (both Access Point and connect to home WiFi)
    Serial.println("Setting up WiFi in AP+STA mode...");
    WiFi.mode(WIFI_AP_STA);

    // Create Access Point for phone connection
    Serial.printf("Creating Access Point: %s\n", WIFI_SSID);
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("✅ Access Point Created!");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Connect to home WiFi for Flask server access
    Serial.printf("Connecting to home WiFi: %s\n", HOME_WIFI_SSID);
    WiFi.begin(HOME_WIFI_SSID, HOME_WIFI_PASSWORD);

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT)
    {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\n✅ Connected to home WiFi!");
        Serial.print("Station IP: ");
        Serial.println(WiFi.localIP());
        Serial.printf("Flask Server: http://%s:%d\n", FLASK_SERVER_IP, FLASK_SERVER_PORT);
    }
    else
    {
        Serial.println("\n⚠️ Home WiFi connection failed!");
        Serial.println("AP mode still works, but Flask forwarding disabled");
    }

    Serial.println("\n📱 To connect from your phone:");
    Serial.printf("1. Connect to WiFi: %s\n", WIFI_SSID);
    Serial.printf("2. Password: %s\n", WIFI_PASSWORD);
    Serial.printf("3. Open browser: http://%s/\n", WiFi.softAPIP().toString().c_str());

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/html", index_html); });

    server.begin();
    Serial.println("✅ WebSocket Server Started");
    Serial.printf("Access user app at: http://%s/\n", WiFi.softAPIP().toString().c_str());
}

void loop()
{
    ws.cleanupClients();

    // Process Flask queue (send one item per loop to avoid blocking)
    if (!flaskQueue.empty() && WiFi.status() == WL_CONNECTED)
    {
        String data = flaskQueue.front();
        flaskQueue.pop();
        sendToFlaskServer(data);
    }

    unsigned long currentTime = millis();
    auto it = activeSessions.begin();
    while (it != activeSessions.end())
    {
        if (it->second.disconnectPending && (currentTime - it->second.disconnectTime > DISCONNECT_TIMEOUT))
        {
            Serial.printf("Removing session for %s\n", it->second.username.c_str());
            it = activeSessions.erase(it); // Correct way to erase while iterating
        }
        else
        {
            ++it;
        }
    }
}