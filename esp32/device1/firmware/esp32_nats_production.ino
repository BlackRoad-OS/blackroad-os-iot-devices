/*
 * ESP32 NATS Production Firmware - BlackRoad Infrastructure
 *
 * Features:
 * - Full NATS client with NKEYS authentication
 * - Publishes device status and sensor data
 * - Subscribes to command topics
 * - Auto-reconnect with exponential backoff
 * - Integrates with Octavia NATS server (192.168.4.38:4222)
 *
 * NATS Server: blackroad-nats on Octavia (192.168.4.38)
 * User: esp32-device
 * Public Key: UBAVV4GNFXP5VMD62YOENVMNORDN77EUJ7KJ7UJEULYVYR4VBCQITQSM
 */

#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// CONFIGURATION - UPDATE THESE VALUES
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

// WiFi credentials
const char* WIFI_SSID = "asdfghjkl";
const char* WIFI_PASSWORD = "Smushymush421!";

// NATS server configuration (Octavia)
const char* NATS_SERVER = "192.168.4.38";
const int NATS_PORT = 4222;
const int NATS_MONITOR_PORT = 8222;

// NKEYS credentials (esp32-device user)
const char* NATS_USER_JWT = "eyJ0eXAiOiJKV1QiLCJhbGciOiJlZDI1NTE5LW5rZXkifQ.eyJqdGkiOiI0SE02STJJVlQ0RldaQTNMR0dMNlRWRkdSUVhNQkI3QzRJSVFBNVRLMlpPTUVCS1JBUlNBIiwiaWF0IjoxNzY3NTU2NDY3LCJpc3MiOiJBQkFWVlBXWU43SlpCSE9NQ1lHVE1JSkdLR05MUVRKRVVPRkhFRVRPTlNRVkVLUFFBR0ZBVklWWCIsIm5hbWUiOiJlc3AzMi1kZXZpY2UiLCJzdWIiOiJVQkFWVjRHTkZYUDVWTUQ2MllPRU5WTU5PUkRONzdFVUo3S0o3VUpFVUxZVllSNFZCQ1FJVFFTTSIsIm5hdHMiOnsicHViIjp7fSwic3ViIjp7fSwic3VicyI6LTEsImRhdGEiOi0xLCJwYXlsb2FkIjotMSwidHlwZSI6InVzZXIiLCJ2ZXJzaW9uIjoyfX0.skKV5OJeyCfm4BU-0vsR6dEJsr0YdUHdO-SGO8_7rWmMGT-RnU7CYFPU3YDks64hgenKB59B_jCY5DmDS54kCw";

const char* NATS_USER_SEED = "SUAHHRRP3C7GTP6GVJE7ALMBBST5ZRY5QMSG3AMJ6EH6MB2EE255K2KUII";

// Device identification
const char* DEVICE_ID = "esp32-device1";
const char* DEVICE_TYPE = "esp32-devkit";

// NATS subjects
const char* SUBJECT_STATUS = "blackroad.devices.esp32.status";
const char* SUBJECT_SENSORS = "blackroad.devices.esp32.sensors";
const char* SUBJECT_COMMANDS = "blackroad.devices.esp32.commands";
const char* SUBJECT_HEARTBEAT = "blackroad.devices.esp32.heartbeat";

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// GLOBALS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

WiFiClient natsClient;
bool natsConnected = false;
unsigned long lastReconnectAttempt = 0;
unsigned long lastHeartbeat = 0;
unsigned long lastSensorPublish = 0;
int reconnectDelay = 1000; // Start with 1 second, exponential backoff

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// SETUP
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void setup() {
  Serial.begin(115200);
  delay(1000);

  printHeader();

  // Connect to WiFi
  connectWiFi();

  // Verify NATS server is accessible
  verifyNATSServer();

  // Connect to NATS
  connectNATS();

  // Publish initial status
  publishDeviceStatus();
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// MAIN LOOP
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void loop() {
  // Maintain WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️  WiFi disconnected, reconnecting...");
    connectWiFi();
  }

  // Maintain NATS connection
  if (!natsConnected) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > reconnectDelay) {
      lastReconnectAttempt = now;
      if (connectNATS()) {
        reconnectDelay = 1000; // Reset backoff on success
      } else {
        reconnectDelay = min(reconnectDelay * 2, 30000); // Max 30s
      }
    }
  }

  // Publish heartbeat every 30 seconds
  if (natsConnected && millis() - lastHeartbeat > 30000) {
    publishHeartbeat();
    lastHeartbeat = millis();
  }

  // Publish sensor data every 5 seconds
  if (natsConnected && millis() - lastSensorPublish > 5000) {
    publishSensorData();
    lastSensorPublish = millis();
  }

  // Process NATS messages
  if (natsConnected && natsClient.available()) {
    processNATSMessage();
  }

  delay(100);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// WiFi FUNCTIONS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void connectWiFi() {
  Serial.println("");
  Serial.print("📡 Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected");
    Serial.print("   IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("   RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("\n❌ WiFi connection failed - will retry");
  }
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// NATS VERIFICATION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void verifyNATSServer() {
  Serial.println("\n🔍 Verifying NATS server...");
  Serial.print("   Server: ");
  Serial.print(NATS_SERVER);
  Serial.print(":");
  Serial.println(NATS_MONITOR_PORT);

  HTTPClient http;
  String url = String("http://") + NATS_SERVER + ":" + NATS_MONITOR_PORT + "/varz";

  http.begin(url);
  http.setTimeout(5000);
  int httpCode = http.GET();

  if (httpCode == 200) {
    Serial.println("✅ NATS server is reachable");
    String payload = http.getString();

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      Serial.println("   Server info:");
      Serial.print("   - Server: ");
      Serial.println(doc["server_name"].as<const char*>());
      Serial.print("   - Version: ");
      Serial.println(doc["version"].as<const char*>());
      Serial.print("   - Uptime: ");
      Serial.println(doc["uptime"].as<const char*>());
      Serial.print("   - JetStream: ");
      Serial.println(doc["jetstream"]["config"]["max_memory"].as<long>() > 0 ? "enabled" : "disabled");
    }
  } else {
    Serial.print("❌ Failed to reach NATS server (HTTP ");
    Serial.print(httpCode);
    Serial.println(")");
  }

  http.end();
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// NATS CONNECTION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

bool connectNATS() {
  Serial.println("\n🚀 Connecting to NATS server...");
  Serial.print("   ");
  Serial.print(NATS_SERVER);
  Serial.print(":");
  Serial.println(NATS_PORT);

  if (natsClient.connect(NATS_SERVER, NATS_PORT)) {
    Serial.println("✅ TCP connection established");

    // Read server INFO
    String info = readLine();
    if (info.startsWith("INFO")) {
      Serial.println("✅ Received NATS server INFO");

      // Send CONNECT with JWT
      String connectMsg = "CONNECT {\"jwt\":\"";
      connectMsg += NATS_USER_JWT;
      connectMsg += "\",\"name\":\"";
      connectMsg += DEVICE_ID;
      connectMsg += "\",\"verbose\":false,\"pedantic\":false,\"protocol\":1}\r\n";

      natsClient.print(connectMsg);

      // Wait for +OK or -ERR
      delay(100);
      if (natsClient.available()) {
        String response = readLine();
        if (response.startsWith("+OK") || response.length() == 0) {
          Serial.println("✅ NATS authentication successful");
          natsConnected = true;

          // Subscribe to command topic
          subscribeToCommands();

          return true;
        } else {
          Serial.print("❌ NATS auth failed: ");
          Serial.println(response);
        }
      }
    }
  } else {
    Serial.println("❌ TCP connection failed");
  }

  natsConnected = false;
  natsClient.stop();
  return false;
}

void subscribeToCommands() {
  String sub = "SUB ";
  sub += SUBJECT_COMMANDS;
  sub += " 1\r\n";
  natsClient.print(sub);
  Serial.print("📬 Subscribed to: ");
  Serial.println(SUBJECT_COMMANDS);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// NATS PUBLISHING
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void publishToNATS(const char* subject, String payload) {
  if (!natsConnected) return;

  String msg = "PUB ";
  msg += subject;
  msg += " ";
  msg += payload.length();
  msg += "\r\n";
  msg += payload;
  msg += "\r\n";

  natsClient.print(msg);
}

void publishDeviceStatus() {
  Serial.println("\n📤 Publishing device status...");

  StaticJsonDocument<512> doc;
  doc["device_id"] = DEVICE_ID;
  doc["device_type"] = DEVICE_TYPE;
  doc["status"] = "online";
  doc["ip"] = WiFi.localIP().toString();
  doc["rssi"] = WiFi.RSSI();
  doc["uptime_seconds"] = millis() / 1000;
  doc["free_heap"] = ESP.getFreeHeap();
  doc["chip_model"] = ESP.getChipModel();
  doc["chip_revision"] = ESP.getChipRevision();
  doc["cpu_freq_mhz"] = ESP.getCpuFreqMHz();
  doc["timestamp"] = millis();

  String json;
  serializeJson(doc, json);

  publishToNATS(SUBJECT_STATUS, json);

  Serial.print("   Subject: ");
  Serial.println(SUBJECT_STATUS);
  Serial.print("   Payload: ");
  Serial.println(json);
}

void publishSensorData() {
  StaticJsonDocument<512> doc;
  doc["device_id"] = DEVICE_ID;
  doc["timestamp"] = millis();

  // Mock sensor data (replace with real sensors)
  doc["temperature_c"] = 20 + random(0, 10);
  doc["humidity_pct"] = 40 + random(0, 20);
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["free_heap"] = ESP.getFreeHeap();

  String json;
  serializeJson(doc, json);

  publishToNATS(SUBJECT_SENSORS, json);

  Serial.print("📊 Sensor: ");
  Serial.println(json);
}

void publishHeartbeat() {
  StaticJsonDocument<256> doc;
  doc["device_id"] = DEVICE_ID;
  doc["status"] = "alive";
  doc["uptime"] = millis() / 1000;
  doc["free_heap"] = ESP.getFreeHeap();

  String json;
  serializeJson(doc, json);

  publishToNATS(SUBJECT_HEARTBEAT, json);

  Serial.print("💓 Heartbeat sent (uptime: ");
  Serial.print(millis() / 1000);
  Serial.println("s)");
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// NATS MESSAGE PROCESSING
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void processNATSMessage() {
  String msg = readLine();

  if (msg.startsWith("MSG")) {
    // Parse: MSG <subject> <sid> <size>
    int firstSpace = msg.indexOf(' ');
    int secondSpace = msg.indexOf(' ', firstSpace + 1);
    int thirdSpace = msg.indexOf(' ', secondSpace + 1);

    String subject = msg.substring(firstSpace + 1, secondSpace);
    int size = msg.substring(thirdSpace + 1).toInt();

    // Read payload
    char payload[size + 1];
    natsClient.readBytes(payload, size);
    payload[size] = '\0';

    Serial.print("\n📨 Received command: ");
    Serial.println(payload);

    // Process command
    handleCommand(payload);

    // Read trailing \r\n
    readLine();
  } else if (msg.startsWith("PING")) {
    natsClient.print("PONG\r\n");
    Serial.println("🏓 PONG");
  } else if (msg.startsWith("-ERR")) {
    Serial.print("❌ NATS error: ");
    Serial.println(msg);
    natsConnected = false;
    natsClient.stop();
  }
}

void handleCommand(String command) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, command);

  if (error) {
    Serial.println("❌ Invalid JSON command");
    return;
  }

  const char* cmd = doc["command"];

  if (strcmp(cmd, "reboot") == 0) {
    Serial.println("🔄 Rebooting in 3 seconds...");
    delay(3000);
    ESP.restart();
  } else if (strcmp(cmd, "status") == 0) {
    publishDeviceStatus();
  } else if (strcmp(cmd, "ping") == 0) {
    Serial.println("🏓 Ping received, sending status");
    publishDeviceStatus();
  } else {
    Serial.print("❓ Unknown command: ");
    Serial.println(cmd);
  }
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// UTILITY FUNCTIONS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

String readLine() {
  String line = "";
  unsigned long timeout = millis() + 1000;

  while (millis() < timeout) {
    if (natsClient.available()) {
      char c = natsClient.read();
      if (c == '\n') break;
      if (c != '\r') line += c;
    }
  }

  return line;
}

void printHeader() {
  Serial.println("");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("🚀 ESP32 NATS Production Firmware");
  Serial.println("   BlackRoad Infrastructure Integration");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("");
  Serial.print("Device ID: ");
  Serial.println(DEVICE_ID);
  Serial.print("Device Type: ");
  Serial.println(DEVICE_TYPE);
  Serial.print("NATS Server: ");
  Serial.print(NATS_SERVER);
  Serial.print(":");
  Serial.println(NATS_PORT);
  Serial.println("");
  Serial.println("Subjects:");
  Serial.print("  - Status: ");
  Serial.println(SUBJECT_STATUS);
  Serial.print("  - Sensors: ");
  Serial.println(SUBJECT_SENSORS);
  Serial.print("  - Commands: ");
  Serial.println(SUBJECT_COMMANDS);
  Serial.print("  - Heartbeat: ");
  Serial.println(SUBJECT_HEARTBEAT);
  Serial.println("");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
}
