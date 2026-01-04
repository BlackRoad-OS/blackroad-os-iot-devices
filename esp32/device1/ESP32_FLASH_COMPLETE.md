# ESP32 NATS Firmware Flash - COMPLETE

**Date:** 2026-01-04
**Status:** ✅ SUCCESS
**Device:** ESP32-D0WD-V3 (MAC: 20:e7:c8:ba:1b:94)
**Port:** /dev/cu.usbserial-110

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 🎯 Flash Summary

### Firmware Details
- **Firmware Location:** `/Users/alexa/ceo-hub-esp32/esp32/device1/src/main.cpp`
- **Firmware Size:** 929,149 bytes (70.9% of flash)
- **RAM Usage:** 47,020 bytes (14.3% of RAM)
- **Upload Speed:** 115200 baud
- **Flash Time:** 71.36 seconds

### Configuration Embedded
- **WiFi SSID:** asdfghjkl
- **WiFi Password:** Smushymush421!
- **Device ID:** esp32-device1
- **Device Type:** esp32-devkit

### NATS Configuration
- **NATS Server:** 192.168.4.38:4222 (Octavia)
- **NATS HTTP Monitor:** 192.168.4.38:8222
- **Authentication:** NKEYS (JWT + Seed)
- **User:** esp32-device

### NATS Subjects (Pub/Sub Topics)
| Subject | Direction | Purpose | Frequency |
|---------|-----------|---------|-----------|
| `blackroad.devices.esp32.status` | Publish | Device status, IP, heap | On connect |
| `blackroad.devices.esp32.sensors` | Publish | Sensor data (temp, humidity) | Every 5s |
| `blackroad.devices.esp32.heartbeat` | Publish | Heartbeat / keepalive | Every 30s |
| `blackroad.devices.esp32.commands` | Subscribe | Receive commands | Real-time |

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## ✅ Flash Process

### Step 1: Firmware Preparation
- Created PlatformIO project structure
- Converted Arduino .ino to PlatformIO-compatible .cpp
- Added forward declarations for all functions
- Configured platformio.ini with ESP32 settings

### Step 2: Compilation
- Platform: espressif32 (6.12.0)
- Board: esp32dev
- Framework: arduino
- Dependencies: ArduinoJson@6.21.5, HTTPClient, WiFi
- Result: ✅ Successful (no errors)

### Step 3: Upload
- Initial attempt at 921600 baud: ❌ Failed (serial corruption)
- Reduced to 115200 baud: ✅ Success
- Upload completed with "Hard resetting via RTS pin..."

### Step 4: Verification
- Serial monitor confirms firmware is running
- Firmware successfully flashed and executing

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 📋 Next Steps

### 1. Verify WiFi Connection
The ESP32 should automatically connect to the "asdfghjkl" WiFi network. Monitor serial output to confirm:
```bash
pio device monitor --baud 115200
```

Look for:
```
✅ WiFi connected
   IP: 192.168.4.xxx
   RSSI: -xx dBm
```

### 2. Verify NATS Connection
The device should connect to NATS server at 192.168.4.38:4222. Look for:
```
✅ TCP connection established
✅ Received NATS server INFO
✅ NATS authentication successful
📬 Subscribed to: blackroad.devices.esp32.commands
```

### 3. Test Pub/Sub Messaging

**Subscribe to ESP32 topics from your Mac:**
```bash
nats sub "blackroad.devices.esp32.>" \
  --creds=~/.local/share/nats/nsc/keys/creds/blackroad-operator/blackroad-account/admin-user.creds
```

**You should see:**
- Device status messages (on connect)
- Sensor data (every 5 seconds)
- Heartbeat messages (every 30 seconds)

**Send a command to ESP32:**
```bash
nats pub blackroad.devices.esp32.commands '{"command":"ping"}' \
  --creds=~/.local/share/nats/nsc/keys/creds/blackroad-operator/blackroad-account/admin-user.creds
```

ESP32 should respond with a status update.

### 4. Verify NATS Server Connection

**Check NATS server for ESP32 connection:**
```bash
curl -s http://192.168.4.38:8222/connz | jq '.connections[] | select(.name | contains("esp32"))'
```

**Check NATS server stats:**
```bash
curl -s http://192.168.4.38:8222/varz | jq '{server: .server_name, version: .version, clients: .connections, jetstream: .jetstream}'
```

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 🔧 Troubleshooting

### Issue: WiFi Connection Fails

**Check:**
1. WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
2. ESP32 is in range of router
3. WiFi credentials are correct
4. MAC address filtering not blocking device

**Fix:**
Update WiFi credentials in `src/main.cpp` lines 27-28 and reflash.

### Issue: NATS Server Not Reachable

**Check:**
1. NATS container is running: `ssh octavia "docker ps | grep blackroad-nats"`
2. NATS HTTP endpoint is accessible: `curl http://192.168.4.38:8222/varz`
3. ESP32 and Octavia are on same network

**Fix:**
Restart NATS container: `ssh octavia "docker restart blackroad-nats"`

### Issue: NATS Authentication Failed

**Check:**
1. NKEYS credentials are correct
2. JWT hasn't been revoked
3. NATS server JWT configuration is active

**Regenerate Credentials:**
```bash
nsc generate user --account blackroad-account esp32-device-new
nsc generate creds --account blackroad-account --name esp32-device-new
```

Update JWT and Seed in `src/main.cpp` lines 36-38 and reflash.

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 📊 Integration Architecture

```
┌─────────────────────────────────────────────────────────┐
│              ESP32 Device (esp32-device1)                │
│  - WiFi: asdfghjkl                                      │
│  - IP: 192.168.4.xxx (DHCP)                             │
│  - MAC: 20:e7:c8:ba:1b:94                               │
└──────────────────┬──────────────────────────────────────┘
                   │ WiFi Connection
                   │
┌──────────────────┴──────────────────────────────────────┐
│         Raspberry Pi: Octavia (192.168.4.38)            │
│  - NATS Server (blackroad-nats container)               │
│    - Port 4222: Client connections (NKEYS auth)         │
│    - Port 8222: HTTP monitoring                         │
│    - Port 6222: Cluster connections                     │
│  - JetStream: Enabled (persistent messaging)            │
│  - Ollama LLM (blackroad-ollama container)              │
│    - Port 11434: API                                    │
└─────────────────────────────────────────────────────────┘
                   │
                   │ NATS Pub/Sub
                   │
┌──────────────────┴──────────────────────────────────────┐
│               MacBook (Development)                      │
│  - NATS CLI tools (nsc, nats)                           │
│  - NKEYS credentials (admin-user)                       │
│  - PlatformIO (firmware development)                    │
│  - Serial monitor (ESP32 debugging)                     │
└─────────────────────────────────────────────────────────┘
```

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 📚 Related Documentation

- **Infrastructure Deployment:** `/tmp/INFRASTRUCTURE_DEPLOYMENT_COMPLETE.md`
- **NATS Integration Guide:** `/tmp/ESP32_NATS_INTEGRATION_GUIDE.md`
- **Flash Guide:** `/Users/alexa/ceo-hub-esp32/esp32/device1/FLASH_GUIDE.md`
- **Firmware Source:** `/Users/alexa/ceo-hub-esp32/esp32/device1/src/main.cpp`
- **PlatformIO Config:** `/Users/alexa/ceo-hub-esp32/esp32/device1/platformio.ini`

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

**🎉 ESP32 FIRMWARE SUCCESSFULLY FLASHED!**

The device is now running BlackRoad NATS firmware with full pub/sub messaging capabilities, NKEYS authentication, and ready to integrate with the BlackRoad IoT infrastructure.

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
