# ESP32 CEO Hub - BlackRoad AI Quantum Device

**Enterprise-grade CEO dashboard + Emergency Pager + Performance Monitor on ESP32 hardware.**

> *"No iPhone can do this. This is what sovereignty looks like."*

---

## 🎯 What Is This?

An ESP32-2432S028R touchscreen device that provides:

1. **CEO Dashboard** - Real-time infrastructure monitoring across mesh network, CRM, AI systems
2. **Emergency Pager** - Receive and acknowledge critical alerts via serial commands
3. **Performance Monitor** - Enterprise-grade system health diagnostics
4. **Dynamic Navigation** - Live API integration with graceful static fallbacks
5. **Sovereign Stack Monitor** - Track infrastructure sovereignty score and forkability
6. **Serial Diagnostics** - Remote monitoring via UART commands

This is the world's first **embedded CEO control center** with infrastructure sovereignty tracking and real-time performance analytics.

---

## 📊 Current Stats

**Firmware Version:** v1.5 (Performance Monitor + Serial Diagnostics)
**Last Build:** 2026-01-05

| Metric | Value | % Used |
|--------|-------|--------|
| Flash | 929,869 bytes | 70.9% |
| RAM | 51,928 bytes | 15.8% |
| CPU | 240MHz (ESP32 Dual Core) | - |
| Display | 240x320 (76,800 pixels) | - |

---

## 📚 Complete Documentation

| Document | Description |
|----------|-------------|
| [PERFORMANCE_MONITOR_SUMMARY.md](PERFORMANCE_MONITOR_SUMMARY.md) | Performance system architecture |
| [SERIAL_DIAGNOSTICS_GUIDE.md](SERIAL_DIAGNOSTICS_GUIDE.md) | Serial command reference |
| [DYNAMIC_NAVIGATION_SUMMARY.md](DYNAMIC_NAVIGATION_SUMMARY.md) | API integration guide |
| [API_SETUP_GUIDE.md](API_SETUP_GUIDE.md) | Configure external APIs |
| [QUICK_REFERENCE.md](QUICK_REFERENCE.md) | One-page cheat sheet |

---

## 🚀 Quick Start

```bash
# 1. Configure WiFi
# Edit src/secrets.h:
#   WIFI_SSID, WIFI_PASSWORD

# 2. Compile and upload
pio run --target upload

# 3. Monitor (optional)
pio device monitor --baud 115200
```

---

## 📱 Key Features

### ✅ Production Ready
- [x] Touch UI with BlackRoad design system
- [x] WiFi auto-connect
- [x] Performance monitoring (heap, CPU, WiFi)
- [x] Serial diagnostics (8 commands)
- [x] Emergency pager system
- [x] Dynamic API integration
- [x] Mesh VPN monitoring
- [x] CRM dashboard
- [x] Sovereignty tracking

### 🔌 Serial Commands

```bash
# Full diagnostics
> PERF

# Quick checks
> HEAP     # Memory snapshot
> WIFI     # WiFi status
> UPTIME   # System uptime

# Help
> HELP     # Show all commands
```

---

## 🎨 Design System

**BlackRoad Official Colors:**
- Amber: `#F5A623`
- Hot Pink: `#FF1D6C`
- Electric Blue: `#2979FF`
- Violet: `#9C27B0`

Custom monospaced font with technical labels and card-based UI.

---

## 📊 Performance Monitor

Real-time tracking:
- Memory health (heap usage, fragmentation)
- CPU metrics (loop timing, frequency)
- WiFi quality (RSSI, signal strength)
- System info (uptime, boot reason)
- Health warnings (low memory, fragmentation, slow loops)

**Warning Thresholds:**
- ⚠️ Low Memory: < 10KB free
- ⚠️ High Fragmentation: > 30%
- ⚠️ Slow Loop: > 100ms avg
- ⚠️ Weak WiFi: < 40% quality

---

## 🌐 Dynamic Navigation

Fetches live data from:
- **Mesh VPN API** - Node status, latency, bandwidth
- **CRM API** - Hot leads, deals, pipeline value
- **AI API** - Model health, inference counts

Graceful degradation to static data when APIs unavailable.

---

## 🛡️ Sovereign Stack

Tracks **42 infrastructure components** across layers:
- Self-hosted (Headscale, Keycloak, PostgreSQL)
- External services (Cloudflare, GitHub, OpenAI)

**Current Sovereignty Score:** ~67% (HYBRID status)

---

## 🐛 Troubleshooting

### Touch Not Working
- Check PERF screen for low memory warning
- Run `HEAP` command via serial
- If fragmentation > 30%, reboot device

### APIs Showing Static Data
- Verify WiFi connection (WIFI screen)
- Check API endpoints in `dynamic_nav.h`
- Wait 5 minutes for auto-retry

### Device Keeps Rebooting
- Run `UPTIME` to check boot reason
- Watchdog = slow loop (optimize code)
- Brownout = power issue (better USB cable)

---

## 🏆 Why This Matters

### No iPhone Can Do This

- Run arbitrary C++ code with hardware access
- Monitor distributed infrastructure in real-time
- Calculate sovereignty scores
- Provide enterprise diagnostics via UART
- Operate completely offline
- Cost $35

### Real CEO Tool

Production dashboard monitoring:
- $626K+ CRM pipeline
- 4-node mesh network
- 5 AI models
- Critical infrastructure alerts

---

## 📄 License

**Proprietary** - BlackRoad OS, Inc.

Contact: blackroad.systems@gmail.com

---

**Last Updated:** 2026-01-05
**Status:** ✅ Production Ready
**The future is decentralized. The future is sovereign. The future is BlackRoad.**
