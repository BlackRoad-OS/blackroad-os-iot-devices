# ESP32 CEO Hub - Serial Diagnostics Reference

## 🔌 Connection

Connect to the ESP32 via serial at **115200 baud**:

```bash
# macOS
screen /dev/cu.usbserial-110 115200

# Linux
screen /dev/ttyUSB0 115200

# PlatformIO
pio device monitor --baud 115200
```

---

## 📟 Available Commands

Type any command followed by ENTER. Commands are case-sensitive.

### 🆘 Emergency Pager Commands

| Command | Description | Example |
|---------|-------------|---------|
| `ALERT <message>` | Send emergency pager alert | `ALERT Server down!` |
| `ACK` | Acknowledge current alert | `ACK` |
| `STANDBY` | Clear alerts and return to standby | `STANDBY` |
| `STATUS` | Show pager status | `STATUS` |

### 📊 Performance Diagnostics

| Command | Description | Output |
|---------|-------------|--------|
| `PERF` | Full performance diagnostic report | Complete system health report |
| `DIAG` | Alias for PERF | Same as PERF |
| `HEAP` | Quick heap memory snapshot | Memory usage, fragmentation |
| `WIFI` | Quick WiFi status | RSSI, quality, connection info |
| `UPTIME` | Show uptime and boot reason | System uptime, last reboot cause |

### 🛠️ System Commands

| Command | Description | Action |
|---------|-------------|--------|
| `PING` | Check if device is responsive | Returns "READY" |
| `RESET` | Reboot device | Hard reset via ESP.restart() |
| `HELP` | Show command reference | Display all commands |

---

## 📋 Command Examples

### Full Diagnostic Report
```
> PERF

╔════════════════════════════════════════╗
║   BLACKROAD PERFORMANCE DIAGNOSTICS    ║
╠════════════════════════════════════════╣
║ MEMORY                                 ║
║   Free:       125 KB                   ║
║   Min Free:   115 KB                   ║
║   Total:      320 KB                   ║
║   Usage:       61%                     ║
║   Fragment:     8%                     ║
║ CPU & TIMING                           ║
║   Frequency:   240 MHz                 ║
║   Loop Time:   8592 µs                 ║
║   Avg Loop:    9124 µs                 ║
║   Max Loop:   15243 µs                 ║
║   Loop/Sec:    109                     ║
║ WIFI                                   ║
║   RSSI:        -42 dBm                 ║
║   Quality:      89% (Excellent)        ║
║ SYSTEM                                 ║
║   Uptime:     2h 34m                   ║
║   Last Boot:  Power On                 ║
║ WARNINGS                               ║
║   ✅ All systems nominal               ║
╚════════════════════════════════════════╝
```

### Quick Heap Check
```
> HEAP

━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   HEAP MEMORY SNAPSHOT
━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   Free:       128416 bytes (125 KB)
   Min Free:   117824 bytes (115 KB)
   Total:      327680 bytes (320 KB)
   Usage:      61%
   Fragment:   8%
━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

### WiFi Status
```
> WIFI

━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   WIFI STATUS
━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   Status:     Connected
   SSID:       asdfghjkl
   IP:         192.168.4.49
   RSSI:       -42 dBm
   Quality:    89% (Excellent)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

### Uptime Check
```
> UPTIME

Uptime: 2h 34m
Boot Reason: Power On
```

### System Ping
```
> PING
READY
```

### Help
```
> HELP

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   BLACKROAD ESP32 SERIAL COMMANDS
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   ALERT <msg>  - Send emergency pager alert
   ACK          - Acknowledge current alert
   STANDBY      - Clear alerts and return to standby
   PING         - Check if device is responsive
   STATUS       - Show pager status
   PERF         - Full performance diagnostic report
   DIAG         - Alias for PERF
   HEAP         - Quick heap memory snapshot
   WIFI         - Quick WiFi status
   UPTIME       - Show uptime and boot reason
   RESET        - Reboot device
   HELP         - Show this help message
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## 🔍 Monitoring Use Cases

### Debug Memory Leaks
Watch `Min Free` value decrease over time:
```bash
while true; do
  echo "HEAP"
  sleep 5
done | screen /dev/cu.usbserial-110 115200
```

### Monitor Loop Performance
Check for slow loops or UI lag:
```bash
echo "PERF" | screen /dev/cu.usbserial-110 115200
```
Look for:
- `Avg Loop` > 100ms = slow
- `Max Loop` > 500ms = UI lag

### Track WiFi Quality
Monitor signal strength and quality:
```bash
echo "WIFI" | screen /dev/cu.usbserial-110 115200
```

### Check Heap Fragmentation
Watch for fragmentation issues:
```bash
echo "HEAP" | screen /dev/cu.usbserial-110 115200
```
- Fragmentation < 20% = healthy
- Fragmentation > 30% = warning (may cause touch issues)
- Fragmentation > 50% = critical (likely to cause problems)

---

## 🚨 Warning Indicators

The `PERF` command shows warnings when:

### ⚠️ Low Memory
- **Trigger:** Free heap < 10KB
- **Impact:** May cause touch input failure
- **Action:** Identify memory-heavy code, disable features, or optimize

### ⚠️ High Fragmentation
- **Trigger:** Fragmentation > 30%
- **Impact:** Can lead to allocation failures
- **Action:** Reduce dynamic String usage, use char arrays, restart device

### ⚠️ Slow Loop
- **Trigger:** Average loop time > 100ms
- **Impact:** UI lag and touch unresponsiveness
- **Action:** Profile slow screens, optimize rendering, reduce API calls

### ⚠️ Weak WiFi Signal
- **Trigger:** WiFi quality < 40%
- **Impact:** May affect API connectivity
- **Action:** Move closer to router, check antenna, verify SSID

---

## 📈 Performance Thresholds

### Memory (Heap)
| Metric | Good | Warning | Critical |
|--------|------|---------|----------|
| Free Heap | > 50KB | 10-50KB | < 10KB |
| Fragmentation | < 20% | 20-30% | > 30% |
| Usage | < 50% | 50-70% | > 70% |

### CPU & Timing
| Metric | Good | Warning | Critical |
|--------|------|---------|----------|
| Loop Time | < 50ms | 50-100ms | > 100ms |
| Max Loop | < 200ms | 200-500ms | > 500ms |
| Loops/Sec | > 20 | 10-20 | < 10 |

### WiFi Quality
| Metric | Good | Warning | Critical |
|--------|------|---------|----------|
| RSSI | > -50 dBm | -50 to -70 dBm | < -70 dBm |
| Quality | > 60% | 40-60% | < 40% |

---

## 🛠️ Automation Scripts

### Continuous Monitoring (macOS)
Save as `monitor-esp32.sh`:
```bash
#!/bin/bash
while true; do
  echo -e "\n═══ $(date) ═══"
  echo "PERF" > /dev/cu.usbserial-110
  sleep 30
done
```

### Log Performance to File
```bash
echo "PERF" | screen -L /dev/cu.usbserial-110 115200
# Log saved to screenlog.0
```

### Periodic Health Check
```bash
#!/bin/bash
# Check every minute
while true; do
  echo "HEAP" > /dev/cu.usbserial-110
  sleep 60
done
```

---

## 🔗 Integration with Monitoring Systems

### Send to Prometheus/Grafana
Parse serial output and expose as metrics:
```python
import serial
import re

ser = serial.Serial('/dev/cu.usbserial-110', 115200)

while True:
    ser.write(b'HEAP\n')
    output = ser.read_until(b'━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n')

    # Parse metrics
    free_heap = re.search(r'Free:\s+(\d+)\s+bytes', output)
    fragmentation = re.search(r'Fragment:\s+(\d+)%', output)

    # Export to Prometheus format
    print(f'esp32_free_heap_bytes {free_heap.group(1)}')
    print(f'esp32_heap_fragmentation_percent {fragmentation.group(1)}')

    time.sleep(10)
```

### Slack/Discord Notifications
Send alerts when thresholds exceeded:
```python
import serial
import requests

ser = serial.Serial('/dev/cu.usbserial-110', 115200)

while True:
    ser.write(b'HEAP\n')
    output = ser.read_until(b'━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n').decode()

    # Check for low memory
    if 'Free:' in output:
        free_kb = int(output.split('Free:')[1].split('KB')[0].strip())
        if free_kb < 50:
            # Send Slack alert
            requests.post(SLACK_WEBHOOK, json={
                'text': f'⚠️ ESP32 Low Memory Alert: {free_kb}KB free'
            })

    time.sleep(300)  # Check every 5 minutes
```

---

## 📚 Related Documentation

- [PERFORMANCE_MONITOR_SUMMARY.md](PERFORMANCE_MONITOR_SUMMARY.md) - Full performance monitor system
- [src/performance.h](src/performance.h) - Performance monitoring implementation
- [QUICK_REFERENCE.md](QUICK_REFERENCE.md) - Device quick reference guide

---

**Last Updated:** 2026-01-05
**Firmware Version:** v1.5 (with Serial Diagnostics)
**Baud Rate:** 115200
**Protocol:** ASCII text commands
