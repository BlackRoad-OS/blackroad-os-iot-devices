# ESP32 NATS Firmware Flashing Guide

**Complete guide to flash production NATS firmware to your ESP32 device**

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 📋 Prerequisites

### Hardware
- ESP32 development board (ESP32-DevKitC or similar)
- USB-C cable (for ESP32 connection)
- Computer with Arduino IDE or PlatformIO

### Software
- **Option A: Arduino IDE** (recommended for beginners)
  - Download: https://www.arduino.cc/en/software
  - ESP32 board support package
  - Required libraries: WiFi, HTTPClient, ArduinoJson

- **Option B: PlatformIO** (recommended for advanced users)
  - VS Code + PlatformIO extension
  - Automatic dependency management

### Network
- WiFi network credentials (SSID and password)
- ESP32 must be able to reach Octavia at 192.168.4.38

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 🔧 Step 1: Configure WiFi Credentials

Edit the firmware file: `firmware/esp32_nats_production.ino`

Find lines **17-18** and update with your WiFi credentials:

```cpp
// BEFORE:
const char* WIFI_SSID = "YOUR_WIFI_SSID";        // ⚠️ UPDATE THIS
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // ⚠️ UPDATE THIS

// AFTER (example):
const char* WIFI_SSID = "MyNetwork";
const char* WIFI_PASSWORD = "MySecurePassword123";
```

**⚠️ IMPORTANT:** Make sure your WiFi network can reach 192.168.4.38 (Octavia)

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 🔐 Step 2: Verify NKEYS Credentials

The firmware already contains embedded NKEYS credentials:

```cpp
// JWT Token (already embedded - line 32)
const char* NATS_USER_JWT = "eyJ0eXAiOiJKV1QiLCJhbGciOiJlZDI1NTE5LW5rZXkifQ...";

// NKEY Seed (already embedded - line 34)
const char* NATS_USER_SEED = "SUAHHRRP3C7GTP6GVJE7ALMBBST5ZRY5QMSG3AMJ6EH6MB2EE255K2KUII";
```

**✅ These are pre-configured for the `esp32-device` user and ready to use!**

**🔒 Security Note:** Keep this firmware file private - it contains authentication credentials.

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 🚀 Step 3: Flash Firmware

### Option A: Using Arduino IDE

1. **Install ESP32 Board Support:**
   - Open Arduino IDE
   - Go to `File > Preferences`
   - Add to "Additional Boards Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to `Tools > Board > Boards Manager`
   - Search for "esp32" and install "ESP32 by Espressif Systems"

2. **Install Required Libraries:**
   - Go to `Tools > Manage Libraries`
   - Install:
     - **ArduinoJson** (by Benoit Blanchon)
     - **WiFi** (should be included with ESP32 board package)
     - **HTTPClient** (should be included with ESP32 board package)

3. **Open Firmware:**
   - Open `firmware/esp32_nats_production.ino`

4. **Configure Board:**
   - `Tools > Board > ESP32 Arduino > ESP32 Dev Module`
   - `Tools > Upload Speed > 115200`
   - `Tools > Flash Frequency > 80MHz`
   - `Tools > Port > [Select your ESP32's COM port]`

5. **Flash:**
   - Click the **Upload** button (→)
   - Wait for compilation and upload to complete
   - You should see: "Hard resetting via RTS pin..."

### Option B: Using PlatformIO

1. **Install PlatformIO:**
   - Install VS Code
   - Install PlatformIO extension from VS Code marketplace

2. **Open Project:**
   ```bash
   cd /Users/alexa/ceo-hub-esp32/esp32/device1
   ```

3. **Create platformio.ini (if not exists):**
   ```ini
   [env:esp32dev]
   platform = espressif32
   board = esp32dev
   framework = arduino
   monitor_speed = 115200
   lib_deps =
       bblanchon/ArduinoJson@^6.21.3
   ```

4. **Flash:**
   ```bash
   pio run --target upload
   ```

   Or use the PlatformIO toolbar in VS Code:
   - Click "Upload" (→) button

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 📡 Step 4: Monitor Serial Output

### Arduino IDE:
- Go to `Tools > Serial Monitor`
- Set baud rate to **115200**
- You should see:

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
🚀 ESP32 NATS Production Firmware
   BlackRoad Infrastructure Integration
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Device ID: esp32-device1
Device Type: esp32-devkit
NATS Server: 192.168.4.38:4222

📡 Connecting to WiFi: MyNetwork
.....
✅ WiFi connected
   IP: 192.168.4.123
   RSSI: -45 dBm

🔍 Verifying NATS server...
   Server: 192.168.4.38:8222
✅ NATS server is reachable
   Server info:
   - Server: nats-server
   - Version: 2.10.7
   - Uptime: 5m32s
   - JetStream: enabled

🚀 Connecting to NATS server...
   192.168.4.38:4222
✅ TCP connection established
✅ Received NATS server INFO
✅ NATS authentication successful
📬 Subscribed to: blackroad.devices.esp32.commands

📤 Publishing device status...
   Subject: blackroad.devices.esp32.status
   Payload: {"device_id":"esp32-device1","status":"online",...}

📊 Sensor: {"device_id":"esp32-device1","temperature_c":23,...}
💓 Heartbeat sent (uptime: 32s)
```

### PlatformIO:
```bash
pio device monitor
```

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## ✅ Step 5: Verify Connection

### From Your Mac:

1. **Subscribe to ESP32 status:**
   ```bash
   nats sub "blackroad.devices.esp32.>" \
     --creds=~/.local/share/nats/nsc/keys/creds/blackroad-operator/blackroad-account/admin-user.creds
   ```

2. **You should see messages:**
   ```json
   [#1] Received on "blackroad.devices.esp32.status"
   {"device_id":"esp32-device1","status":"online","ip":"192.168.4.123",...}

   [#2] Received on "blackroad.devices.esp32.sensors"
   {"device_id":"esp32-device1","temperature_c":23,"humidity_pct":55,...}

   [#3] Received on "blackroad.devices.esp32.heartbeat"
   {"device_id":"esp32-device1","status":"alive","uptime":60}
   ```

3. **Send a command to ESP32:**
   ```bash
   nats pub blackroad.devices.esp32.commands '{"command":"ping"}' \
     --creds=~/.local/share/nats/nsc/keys/creds/blackroad-operator/blackroad-account/admin-user.creds
   ```

   ESP32 should respond with a status update.

4. **Check NATS server stats:**
   ```bash
   curl -s http://192.168.4.38:8222/connz | jq
   ```

   You should see the ESP32 connection listed.

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 🔍 Troubleshooting

### Issue: WiFi Connection Fails

**Symptoms:**
```
❌ WiFi connection failed - will retry
```

**Solutions:**
1. Verify WiFi credentials are correct
2. Check that WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
3. Ensure ESP32 is in range of WiFi router
4. Check if WiFi has MAC address filtering enabled

### Issue: NATS Server Not Reachable

**Symptoms:**
```
❌ Failed to reach NATS server (HTTP 0)
```

**Solutions:**
1. Verify Octavia is online:
   ```bash
   ping 192.168.4.38
   ```

2. Check NATS is running on Octavia:
   ```bash
   ssh octavia "docker ps | grep blackroad-nats"
   ```

3. Test NATS HTTP endpoint:
   ```bash
   curl http://192.168.4.38:8222/varz
   ```

4. Ensure ESP32 and Octavia are on same network

### Issue: NATS Authentication Failed

**Symptoms:**
```
❌ NATS auth failed: -ERR 'Authentication Timeout'
```

**Solutions:**
1. Verify NKEYS credentials haven't been regenerated
2. Check that NATS server is configured to accept JWT authentication
3. Ensure no typos in JWT or SEED constants

### Issue: Compilation Errors

**Error:** `WiFi.h: No such file or directory`

**Solution:** Install ESP32 board support package (see Step 3)

**Error:** `ArduinoJson.h: No such file or directory`

**Solution:** Install ArduinoJson library:
- Arduino IDE: `Tools > Manage Libraries > search "ArduinoJson"`
- PlatformIO: Add to `platformio.ini`:
  ```ini
  lib_deps = bblanchon/ArduinoJson@^6.21.3
  ```

### Issue: Upload Failed

**Error:** `serial.serialutil.SerialException: [Errno 16] could not open port /dev/cu.usbserial-XXX`

**Solutions:**
1. Check USB cable is properly connected
2. Press and hold the **BOOT** button on ESP32 while uploading
3. Try a different USB cable (some cables are charge-only)
4. Check that no other serial monitor is open

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 📊 NATS Subjects Reference

The ESP32 firmware uses these NATS subjects:

| Subject | Direction | Purpose | Frequency |
|---------|-----------|---------|-----------|
| `blackroad.devices.esp32.status` | Publish | Device status, IP, heap | On connect |
| `blackroad.devices.esp32.sensors` | Publish | Sensor data (temp, humidity) | Every 5s |
| `blackroad.devices.esp32.heartbeat` | Publish | Heartbeat / keepalive | Every 30s |
| `blackroad.devices.esp32.commands` | Subscribe | Receive commands | Real-time |

### Supported Commands:

```json
{"command": "ping"}      // Request status update
{"command": "status"}    // Request full device status
{"command": "reboot"}    // Reboot ESP32 (3 second delay)
```

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 🎯 Next Steps

### 1. Add Real Sensors

Replace mock sensor data with real sensors:

```cpp
// Replace lines 310-311 with real sensor readings
#include <DHT.h>  // Temperature/humidity sensor

DHT dht(GPIO_PIN, DHT22);

void publishSensorData() {
  doc["temperature_c"] = dht.readTemperature();
  doc["humidity_pct"] = dht.readHumidity();
  // ...
}
```

### 2. Implement JetStream Persistence

Enable message persistence for critical data:

```cpp
// Add to NATS connection
String sub = "SUB blackroad.devices.esp32.commands 1\r\n";
sub += "STREAM CREATE sensor_data\r\n";
natsClient.print(sub);
```

### 3. Add TLS Encryption

For production, enable TLS:

```cpp
#include <WiFiClientSecure.h>

WiFiClientSecure natsClient;
natsClient.setCACert(ca_cert);  // Add CA certificate
natsClient.connect(NATS_SERVER, NATS_TLS_PORT);  // Use port 4443
```

### 4. Build Dashboard

Subscribe to ESP32 topics and visualize data:

```bash
# Real-time monitoring
nats sub "blackroad.devices.esp32.>" --creds=...

# Stream to log file
nats sub "blackroad.devices.esp32.sensors" --creds=... >> sensors.log

# Query historical data (with JetStream)
nats stream view sensor_data
```

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 📚 Additional Resources

- **NATS Documentation:** https://docs.nats.io/
- **ESP32 Arduino Core:** https://docs.espressif.com/projects/arduino-esp32/
- **ArduinoJson Library:** https://arduinojson.org/
- **BlackRoad Infrastructure Docs:** `/tmp/INFRASTRUCTURE_DEPLOYMENT_COMPLETE.md`
- **NKEYS Guide:** https://docs.nats.io/running-a-nats-service/configuration/securing_nats/auth_intro/nkey_auth

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## ✅ Quick Start Checklist

- [ ] Install Arduino IDE or PlatformIO
- [ ] Install ESP32 board support
- [ ] Install ArduinoJson library
- [ ] Update WiFi credentials in firmware (lines 17-18)
- [ ] Connect ESP32 via USB-C
- [ ] Select correct board and port
- [ ] Upload firmware
- [ ] Open Serial Monitor (115200 baud)
- [ ] Verify WiFi connection
- [ ] Verify NATS connection
- [ ] Subscribe to ESP32 topics from Mac
- [ ] Test sending commands to ESP32

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

**🎉 You're ready to flash! Connect your ESP32 and let's go!**
