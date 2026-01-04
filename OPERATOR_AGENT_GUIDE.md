# Operator by BlackRoad - Agent Integration Guide

**For all Claude agents working with the BlackRoad ecosystem**

## 🎯 What is the Operator?

The **Operator** is a physical ESP32 touchscreen device that serves as a CEO decision hub for the BlackRoad network. It's a networked OS node with a touch interface for HOT/NOT/SKIP decision-making.

## 📍 Device Location & Access

- **Project Directory:** `~/ceo-hub-esp32`
- **Serial Port:** `/dev/cu.usbserial-110`
- **WiFi Network:** asfghjkl
- **Device Hostname:** `ceo-hub-esp32`
- **Hardware:** ESP32-2432S028R (2.8" ILI9341 240x320 touchscreen)

## 🛠️ How to Update the Operator Display

### 1. Edit the Code
```bash
# Main application code
vim ~/ceo-hub-esp32/src/main.cpp

# WiFi/SSH configuration
vim ~/ceo-hub-esp32/include/config.h
```

### 2. Build and Upload
```bash
cd ~/ceo-hub-esp32
pio run --target upload
```

### 3. Monitor Serial Output
```bash
cd ~/ceo-hub-esp32
pio device monitor
# OR
python3 -c "import serial; s = serial.Serial('/dev/cu.usbserial-110', 115200, timeout=5); import time; time.sleep(1); print(s.read(2000).decode('utf-8', errors='ignore'))"
```

## 🎨 Official BlackRoad Design System

**CRITICAL:** Always use these exact colors for the Operator!

### Primary Gradient Colors (RGB565 for TFT)
```cpp
#define COLOR_SUNRISE     0xFCE0  // #FF9D00 Sunrise Orange
#define COLOR_WARM        0xFB40  // #FF6B00 Warm Orange
#define COLOR_HOT_PINK    0xF80C  // #FF0066 Hot Pink
#define COLOR_MAGENTA     0xF80D  // #FF006B Electric Magenta
#define COLOR_DEEP_MAG    0xD015  // #D600AA Deep Magenta
#define COLOR_VIVID_PUR   0x781F  // #7700FF Vivid Purple
#define COLOR_CYBER_BLUE  0x033F  // #0066FF Cyber Blue
```

### Core Neutrals
```cpp
#define COLOR_BLACK       0x0000  // #000000 Pure Black (background)
#define COLOR_WHITE       0xFFFF  // #FFFFFF Pure White (text)
```

### Design Rules
- ✅ **Black background** (#000000) - ALWAYS
- ✅ **White text** (#FFFFFF) - for all primary text
- ✅ **Gradient colors** - for buttons and accents only
- ❌ **NO other colors** - stick to the official palette

## 🌐 Network Integration

### SSH Nodes
The Operator is connected to these BlackRoad network nodes:

```cpp
const SSHNode sshNodes[] = {
  {"lucidia", "192.168.4.38", 22},     // Pi 5 - ONLINE
  {"shellfish", "192.168.4.99", 22},   // x86_64 - ONLINE
  {"octavia", "192.168.4.64", 22},     // Pi 5 - ONLINE
  {"alice", "192.168.4.49", 22},       // Pi - ONLINE
  {"aria", "192.168.4.68", 22}         // Check status
};
```

### Test SSH Connections
```bash
ssh lucidia "hostname && uname -a"
ssh shellfish "hostname && uname -a"
ssh octavia "hostname && uname -a"
ssh alice "hostname && uname -a"
ssh aria "hostname && uname -a"
```

## 📱 Current Interface

### Button Layout (Landscape 320x240)
- **HOT** (Hot Pink #FF0066) - x:10, y:60, w:90, h:80 - Approve/Yes
- **NOT** (Cyber Blue #0066FF) - x:115, y:60, w:90, h:80 - Reject/No
- **SKIP** (Sunrise Orange #FF9D00) - x:220, y:60, w:90, h:80 - Skip/Next

### Display Sections
```
[0-50px]    Title: "BLACKROAD" (Sunrise Orange) + "CEO HUB" (White)
[60-140px]  Three decision buttons (HOT/NOT/SKIP)
[160-240px] Stats display (White/gradient colored counters)
```

## 🔧 Common Modifications

### Add a New Button
```cpp
Button btnNew = {x, y, width, height, COLOR_VIVID_PUR, "LABEL"};

void drawUI() {
  // ... existing code ...
  drawButton(btnNew);
}

void handleTouch() {
  // ... existing code ...
  else if (isTouched(btnNew, x, y)) {
    drawButton(btnNew, true);
    delay(150);
    // Your action here
    updateStats();
    drawButton(btnNew);
  }
}
```

### Change WiFi Credentials
Edit `~/ceo-hub-esp32/include/config.h`:
```cpp
#define WIFI_SSID "your_network"
#define WIFI_PASSWORD "your_password"
```

### Add New SSH Node
Edit `~/ceo-hub-esp32/include/config.h`:
```cpp
const SSHNode sshNodes[] = {
  // ... existing nodes ...
  {"newnode", "192.168.4.XX", 22}
};
const int SSH_NODE_COUNT = 6;  // Update count
```

### Update Display Text
In `src/main.cpp`:
```cpp
void drawUI() {
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Your Text", 160, 100, 2);
}
```

## 📊 Decision Tracking

The Operator tracks:
- **decisionCount** - Total decisions made
- **hotCount** - HOT button presses
- **notCount** - NOT button presses
- **skipCount** - SKIP button presses

Stats are displayed in real-time on the bottom of the screen.

## 🐛 Troubleshooting

### Display Not Turning On
Check backlight in `src/main.cpp`:
```cpp
pinMode(TFT_BL, OUTPUT);
digitalWrite(TFT_BL, HIGH);  // Turn on backlight (pin 21)
```

### Touch Not Working
Verify touch pins in `platformio.ini`:
```ini
-DTOUCH_CS=33
-DTFT_CS=15
-DTFT_DC=2
```

### WiFi Connection Issues
Check serial output:
```bash
pio device monitor
```
Look for "WiFi Connected!" or "WiFi FAILED" messages.

### Upload Fails
1. Make sure no other programs are using the serial port
2. Press reset button on ESP32 during upload
3. Check USB cable (must support data, not just power)

## 📦 Dependencies

```ini
lib_deps =
    bodmer/TFT_eSPI@^2.5.43
```

Build flags configured in `platformio.ini` for the ESP32-2432S028R hardware.

## 🔐 Security Notes

- WiFi password stored in `include/config.h` - **DO NOT COMMIT TO PUBLIC REPOS**
- SSH connections use standard SSH keys from the host machine
- Device accessible on local network only

## 🚀 Future Enhancements

Ideas for agents to implement:
- [ ] HTTP API server for remote control
- [ ] MQTT integration for IoT messaging
- [ ] Decision logging to database/cloud
- [ ] WebSocket for real-time updates
- [ ] Multi-screen navigation
- [ ] Animations and transitions
- [ ] Audio feedback on button press
- [ ] Integration with other BlackRoad services

## 📝 Agent Best Practices

1. **Always check [MEMORY]** before modifying the Operator
2. **Test changes** - use `pio run` before uploading
3. **Follow BlackRoad colors** - no exceptions to the palette
4. **Document modifications** - update [MEMORY] after changes
5. **Coordinate with other agents** - check for conflicts
6. **Keep code clean** - the ESP32 has limited memory
7. **Test on device** - simulator won't show all issues

## 🔗 Related Systems

- **[MEMORY]** - Coordination with other Claude agents
- **[CODEX]** - BlackRoad component library (8,789+ components)
- **SSH Nodes** - Integration with Pi network
- **BlackRoad OS** - Overall ecosystem

---

**Remember:** The Operator is a physical device on the network. Always verify changes work before considering the task complete!

*Last Updated: 2026-01-03*
*Device MAC: 20:e7:c8:ba:1b:94*
