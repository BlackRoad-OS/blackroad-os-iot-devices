# blackroad-os-iot-devices

[![GitHub](https://img.shields.io/badge/GitHub-BlackRoad-OS-purple?style=for-the-badge&logo=github)](https://github.com/BlackRoad-OS/blackroad-os-iot-devices)
[![Status](https://img.shields.io/badge/Status-Active-success?style=for-the-badge)](https://github.com/BlackRoad-OS/blackroad-os-iot-devices)
[![BlackRoad](https://img.shields.io/badge/BlackRoad-OS-black?style=for-the-badge)](https://blackroad.io)

# BlackRoad OS - IoT Devices

**Production-Grade IoT Infrastructure for Fortune 500 Operations**

## Overview

This repository contains firmware, documentation, and configuration for all BlackRoad OS IoT devices including ESP32-based operator consoles, Raspberry Pi servers, and edge computing infrastructure.

## Devices

### ESP32 Operator Devices

#### Device 1 - CEO Hub
- **Location:** `esp32/device1/`
- **Model:** ESP32-2432S028R (ESP32-D0WD-V3)
- **MAC:** 20:e7:c8:ba:1b:94
- **Status:** ✅ PRODUCTION
- **Features:** Live API integrations (GitHub, Crypto), touchscreen UI, auto-fetch system
- **[→ View Documentation](esp32/device1/README.md)**

## Features

### Live API Integrations
- **GitHub API** - Real-time repository stats across 15 organizations
- **CoinGecko Crypto** - Live BTC/ETH/SOL price tracking
- **Unified Dashboard** - Comprehensive API status monitoring
- **Weather API** - Ready to integrate (needs API key)
- **Stripe API** - Ready to integrate (needs API key)

### Production Quality
- ✅ Rate limiting per API
- ✅ Error handling & retry logic
- ✅ Self-monitoring metrics
- ✅ Non-blocking async updates
- ✅ Memory-efficient JSON parsing
- ✅ Network resilience (auto-reconnect)

## Quick Start

### ESP32 Device Setup

```bash
# Clone repository
git clone https://github.com/BlackRoad-OS/blackroad-os-iot-devices.git
cd blackroad-os-iot-devices/esp32/device1

# Install PlatformIO
pip3 install platformio

# Configure WiFi (edit src/secrets.h)
# Add your network credentials

# Build & flash
pio run -t upload

# Monitor output
pio device monitor --baud 115200
```

## Infrastructure

### Raspberry Pi Servers
- **Octavia** (192.168.4.38) - Main server, BlackRoad OS Dashboard
- **Alice** (192.168.4.49) - SSH server
- **Aria** (192.168.4.27) - API service (port 5000)
- **Lucidia** (192.168.4.99) - Offline
- **BlackRoad Pi** (192.168.4.64) - Offline

### Cloud Infrastructure
- **Cloudflare Pages** - blackroad.io, lucidia.earth, blackroadai.com
- **Railway** - 12+ projects
- **DigitalOcean** - 159.65.43.12 (codex-infinity)

## Documentation

### ESP32 Devices
- [Device 1 README](esp32/device1/README.md) - Complete setup guide
- [Build Status](esp32/device1/docs/ESP32_CRYPTO_DASHBOARD_STATUS.md) - Integration details
- [Navigation Guide](esp32/device1/docs/ESP32_NAVIGATION_GUIDE.md) - UI usage
- [Collaboration Notes](esp32/device1/docs/ESP32_COLLABORATION_NOTES.md) - Architecture

## Development

### Building Firmware
```bash
cd esp32/device1

# Clean build
pio run -t clean

# Build only
pio run

# Upload to device
pio run -t upload
```

### Testing
```bash
# Monitor serial output
pio device monitor --baud 115200

# Expected output: WiFi connection, API updates, touch events
```

## API Configuration

Edit `esp32/device1/src/secrets.h`:

```cpp
// WiFi
#define WIFI_SSID "your-network"
#define WIFI_PASSWORD "your-password"

// GitHub (optional - higher rate limits)
#define GITHUB_TOKEN "ghp_your_token_here"

// Weather (ready to enable)
#define OPENWEATHER_API_KEY "your_key_here"
#define ENABLE_WEATHER_API true

// Stripe (ready to enable)
#define STRIPE_API_KEY "sk_test_your_key_here"
#define ENABLE_STRIPE_API true
```

## Architecture

### Auto-Fetch System
- **GitHub Stats:** Every 5 minutes
- **Crypto Prices:** Every 60 seconds
- **Dashboard Report:** Every 2 minutes

### Navigation System
- Bottom navigation bar on all screens
- LEFT button: BACK/HOME navigation
- RIGHT button: RECENT APPS switcher
- Touch gestures for app launching

### API Monitoring
- Per-API request tracking
- Success rate calculation
- Average response time
- Last update timestamps
- Rate limit enforcement

## Contributing

### For Claude Agents

Before working on this repository:
1. Read [Collaboration Notes](esp32/device1/docs/ESP32_COLLABORATION_NOTES.md)
2. Check existing implementations in `src/api_functions.h`
3. Follow production quality patterns (rate limiting, error handling)
4. Test on real hardware when possible
5. Update documentation

### Code Standards
- Use DynamicJsonDocument for JSON parsing
- Implement rate limiting for all external APIs
- Add comprehensive serial logging
- Handle WiFi disconnections gracefully
- Use millis() for non-blocking timers

## Troubleshooting

### Device Not Responding
1. Check WiFi credentials in `secrets.h`
2. Verify serial output (115200 baud)
3. Press physical RESET button
4. Re-flash firmware

### API Errors
- Check WiFi connection status
- Verify API keys are valid
- Review rate limits (GitHub: 60/hr, Crypto: 50/min)
- Check serial output for HTTP status codes

### Build Errors
```bash
# Clean build directory
pio run -t clean

# Re-install libraries
pio lib install

# Verify platformio.ini dependencies
```

## License

Copyright © 2026 BlackRoad OS
All rights reserved.

## Contact

- **Email:** blackroad.systems@gmail.com
- **GitHub:** https://github.com/BlackRoad-OS
- **Website:** https://blackroad.io

---

**Last Updated:** 2026-01-04
**Verification:** cadence-esp32-ux-master-1767499386-37bf3efd
**Build:** 896,341 bytes @ 2026-01-04T18:33:00Z

---

## 📜 License & Copyright

**Copyright © 2026 BlackRoad OS, Inc. All Rights Reserved.**

**CEO:** Alexa Amundson

**PROPRIETARY AND CONFIDENTIAL**

This software is the proprietary property of BlackRoad OS, Inc. and is **NOT for commercial resale**.

### ⚠️ Usage Restrictions:
- ✅ **Permitted:** Testing, evaluation, and educational purposes
- ❌ **Prohibited:** Commercial use, resale, or redistribution without written permission

### 🏢 Enterprise Scale:
Designed to support:
- 30,000 AI Agents
- 30,000 Human Employees
- One Operator: Alexa Amundson (CEO)

### 📧 Contact:
For commercial licensing inquiries:
- **Email:** blackroad.systems@gmail.com
- **Organization:** BlackRoad OS, Inc.

See [LICENSE](LICENSE) for complete terms.
