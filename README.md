# BlackRoad OS - CEO Hub ESP32

**A networked decision-making node for the BlackRoad ecosystem**

## Hardware
- ESP32-2432S028R
- 2.8" TFT LCD Display (ILI9341) 240x320
- Resistive Touch (XPT2046)
- WiFi + Bluetooth

## Features
- ✨ Official BlackRoad gradient colors
- 📱 Touch interface with HOT/NOT/SKIP buttons
- 🌐 WiFi connectivity
- 🖥️ SSH node integration (lucidia, shellfish, octavia, alice, aria)
- 📊 Real-time decision tracking
- 🎨 Black background with white text and gradient accents

## Setup

### 1. Configure WiFi
Edit `include/config.h` and update:
```cpp
#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

### 2. Build and Upload
```bash
cd ~/ceo-hub-esp32
pio run --target upload
```

### 3. Monitor Serial Output
```bash
pio device monitor
```

## Official BlackRoad Colors

### Primary Gradient
- `#FF9D00` — Sunrise Orange
- `#FF6B00` — Warm Orange
- `#FF0066` — Hot Pink
- `#FF006B` — Electric Magenta
- `#D600AA` — Deep Magenta
- `#7700FF` — Vivid Purple
- `#0066FF` — Cyber Blue

### Core Neutrals
- `#000000` — Pure Black (background)
- `#FFFFFF` — Pure White (text)

## SSH Nodes
The hub connects to these BlackRoad network nodes:
- **lucidia** (192.168.4.38:22)
- **shellfish** (192.168.4.99:22)
- **octavia** (192.168.4.64:22)
- **alice** (192.168.4.49:22)
- **aria** (192.168.4.68:22)

## Usage
1. Power on the device
2. Wait for WiFi connection (shows on screen)
3. Touch buttons to make decisions:
   - **HOT** (Hot Pink) - Approve/Yes
   - **NOT** (Cyber Blue) - Reject/No
   - **SKIP** (Sunrise Orange) - Skip/Next
4. Stats update in real-time at the bottom

## Project Structure
```
ceo-hub-esp32/
├── include/
│   └── config.h          # WiFi and SSH configuration
├── src/
│   └── main.cpp          # Main application code
├── platformio.ini        # PlatformIO configuration
└── README.md
```

## Development
- Platform: ESP32
- Framework: Arduino
- Libraries: TFT_eSPI
- Upload Port: /dev/cu.usbserial-110

## BlackRoad Ecosystem Integration
This device is a node in the BlackRoad OS network, designed to integrate with:
- SSH servers across the infrastructure
- Decision logging systems
- Real-time collaboration tools
- The broader BlackRoad AI ecosystem

---

**Built with BlackRoad OS**
*Where gradient colors meet decision-making at the edge*
