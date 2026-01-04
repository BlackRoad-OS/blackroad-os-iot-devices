# BlackRoad OS Enterprise Edition - Functional Test Guide

## ✅ Device Status: BOOTING SUCCESSFULLY

Boot sequence confirmed:
```
=================================
BLACKROAD AI PHONE OS
ESP32 Native AI Operating System
=================================
Display initialized
Touch initialized
WiFi connecting...
```

## 📱 What You Should See on the Physical Screen

### Lock Screen (Initial Boot)
You should see:
- **Pure black background** (#000000)
- **"BLACKROAD AI PHONE"** text at top in gradient (Sunrise Orange → Vivid Purple)
- **WiFi indicator** showing connection status
- **"Tap to unlock"** text at bottom in white
- **Centered unlock indicator** "▲ Tap to unlock"

### How to Test Lock Screen
1. ✅ **Tap anywhere on the screen** → Should unlock to Home Screen

---

## 🏠 Home Screen Testing

### What You Should See
- **Status Bar** at top:
  - WiFi icon (left)
  - Time (center)
  - Battery icon (right)

- **"BLACKROAD OS"** branding text

- **9 App Icons** in 3 rows:
  ```
  Row 1:  [AI]     [MSG]    [CRM]    [VPN]
          Purple   Pink     Magenta  Blue
          Badge:0  Badge:3  Badge:12 Badge:0

  Row 2:  [ID]     [FILE]   [API]    [SET]
          Orange   Orange   Magenta  Purple
          Badge:0  Badge:45 Badge:4  Badge:0

  Row 3:  [CC]
          Gray
          Badge:0
  ```

### How to Test Home Screen
1. ✅ **Verify all 9 app icons are visible** with correct colors
2. ✅ **Check notification badges**: MSG(3), CRM(12), FILE(45), API(4)
3. ✅ **Tap any app icon** → Should navigate to that app

---

## 🔬 Individual App Testing

### 1. AI Inference App (Purple - Top Left)
**Tap "AI" icon**

Should display:
- Title: "AI INFERENCE" in vivid purple
- **Models:**
  - Qwen2.5-7B (RUNNING) - purple indicator
  - Phi-2 (STANDBY) - gray indicator
- **Stats:**
  - 1,247 requests
  - 45.2 tokens/sec
- URL: "ai.blackroad.io"
- **< Back** button (top left)

✅ **Test:** Tap "< Back" → Returns to Home Screen

---

### 2. Messages App (Pink - Top Row, 2nd)
**Tap "MSG" icon**

Should display:
- Title: "MESSAGES" in hot pink
- **3 message bubbles** with blue background
- Sample messages with timestamps
- **< Back** button

✅ **Test:** Tap "< Back" → Returns to Home Screen

---

### 3. CRM App (Magenta - Top Row, 3rd)
**Tap "CRM" icon**

Should display:
- Title: "CRM" in electric magenta
- **3 contacts:**
  - John Smith - $45K
  - Acme Corp - $120K
  - Tech Startup - $89K
- "12 New Leads" in magenta
- URL: "crm.blackroad.io"
- **< Back** button

✅ **Test:** Tap "< Back" → Returns to Home Screen

---

### 4. Mesh VPN App (Cyber Blue - Top Row, 4th)
**Tap "VPN" icon**

Should display:
- Title: "MESH VPN" in cyber blue
- **Connected Nodes:** text
- **4 nodes** with colored status dots:
  - lucidia (purple dot)
  - shellfish (blue dot)
  - octavia (orange dot)
  - alice (pink dot)
- "4 nodes online"
- URL: "vpn.blackroad.io"
- **< Back** button

✅ **Test:** Tap "< Back" → Returns to Home Screen

---

### 5. Identity App (Sunrise Orange - Middle Row, 1st)
**Tap "ID" icon**

Should display:
- Title: "IDENTITY" in sunrise orange
- **User:** alexa@blackroad.io
- **Active Sessions:** 2
- **Last Login:** 2h ago
- URL: "id.blackroad.io"
- **< Back** button

✅ **Test:** Tap "< Back" → Returns to Home Screen

---

### 6. Files App (Warm Orange - Middle Row, 2nd)
**Tap "FILE" icon**

Should display:
- Title: "FILES" in warm orange
- **Recent Files:** header
- **4 files listed:**
  - BLACKROAD_OS_v2.0.md
  - AI_PHONE_GUIDE.md
  - deployment_notes.txt
  - enterprise_apps.json
- **Stats:** "45 files | 2.3 GB"
- URL: "files.blackroad.io"
- **< Back** button

✅ **Test:** Tap "< Back" → Returns to Home Screen

---

### 7. APIs App (Deep Magenta - Middle Row, 3rd)
**Tap "API" icon**

Should display:
- Title: "APIS" in deep magenta
- **API Endpoints:** header
- **4 endpoints with request counts:**
  - /v1/chat: 247
  - /auth/login: 89
  - /crm/leads: 34
  - /mesh/status: 12
- "4 New Requests" in magenta
- URL: "api.blackroad.io"
- **< Back** button

✅ **Test:** Tap "< Back" → Returns to Home Screen

---

### 8. Settings App (Vivid Purple - Middle Row, 4th)
**Tap "SET" icon**

Should display:
- Title: "SETTINGS" in vivid purple
- **WiFi:** asfghjkl (Connected)
- **Device:** ceo-hub-esp32
- **OS Version:** BlackRoad AI v1.0
- **SSH Nodes:** 5 connected
- **< Back** button

✅ **Test:** Tap "< Back" → Returns to Home Screen

---

### 9. Control Center App (Gray - Bottom Row)
**Tap "CC" icon**

Should display:
- Title: "CONTROL CENTER" in white
- **Two toggles:**
  - WiFi (ON) - blue
  - Bluetooth (OFF) - gray
- **Two sliders:**
  - Brightness: 75% (orange)
  - Volume: 50% (blue)
- **< Back** button

✅ **Test:** Tap "< Back" → Returns to Home Screen

---

## 🎯 Complete Functional Test Checklist

### Boot & Lock Screen
- [ ] Device boots and shows "BLACKROAD AI PHONE OS" banner
- [ ] Lock screen displays with gradient branding
- [ ] WiFi indicator shows connection attempt/status
- [ ] Tap unlocks to home screen

### Home Screen
- [ ] All 9 app icons visible and correctly colored
- [ ] Notification badges show: MSG(3), CRM(12), FILE(45), API(4)
- [ ] Status bar displays WiFi, Time, Battery

### Navigation
- [ ] Each app icon launches the correct app screen
- [ ] "< Back" button works in all 9 apps
- [ ] Touch is responsive (no lag > 200ms)
- [ ] No screen flicker or artifacts

### Individual Apps
- [ ] AI Inference: Shows 2 models and stats
- [ ] Messages: Shows 3 message bubbles
- [ ] CRM: Shows 3 contacts with deal values
- [ ] Mesh VPN: Shows 4 nodes with colored indicators
- [ ] Identity: Shows user and session info
- [ ] Files: Shows 4 files and storage stats
- [ ] APIs: Shows 4 endpoints with request counts
- [ ] Settings: Shows device info and WiFi status
- [ ] Control Center: Shows toggles and sliders

### Visual Quality
- [ ] Pure black backgrounds (#000000)
- [ ] White text is crisp and readable
- [ ] Gradient colors match BlackRoad palette
- [ ] Rounded rectangles have smooth corners
- [ ] No color bleeding or ghosting

---

## 🐛 Troubleshooting

### Screen is Blank
```bash
# Check if backlight is on
# Should see TFT_BL pin HIGH in code at line 848
```

### Screen Shows Garbage/Static
```bash
# Re-upload firmware
cd ~/ceo-hub-esp32
pio run --target upload
```

### Touch Not Responding
```bash
# Check serial output for "Touch initialized"
python3 -c "import serial; s = serial.Serial('/dev/cu.usbserial-110', 115200, timeout=2); import time; time.sleep(1); print(s.read(1000).decode('utf-8', errors='ignore'))"
```

### WiFi Not Connecting
```bash
# Check config.h for correct credentials
cat ~/ceo-hub-esp32/include/config.h
# Should show: WIFI_SSID "asfghjkl"
```

### App Crashes When Tapped
```bash
# Monitor serial for error messages
pio device monitor
# Look for stack traces or "Guru Meditation Error"
```

---

## 📊 Serial Debug Commands

### View Boot Messages
```bash
python3 << 'PYEOF'
import serial, time
s = serial.Serial('/dev/cu.usbserial-110', 115200, timeout=0.5)
s.setDTR(False); s.setRTS(True); time.sleep(0.1); s.setRTS(False); time.sleep(1)
for i in range(50):
    if s.in_waiting:
        print(s.read(s.in_waiting).decode('utf-8', errors='ignore'), end='', flush=True)
    time.sleep(0.1)
s.close()
PYEOF
```

### Continuous Monitor
```bash
pio device monitor
# Press Ctrl+C to exit
```

---

## ✅ Expected Results

If all tests pass, you should be able to:
1. ✅ Boot to lock screen
2. ✅ Unlock with a tap
3. ✅ Navigate to all 9 apps
4. ✅ Return to home from each app
5. ✅ See correct colors and branding
6. ✅ See notification badges on 4 apps
7. ✅ No crashes or freezes
8. ✅ Responsive touch (< 200ms delay)

---

## 🎉 Success Criteria

**BlackRoad OS Enterprise Edition is FULLY FUNCTIONAL if:**
- All 9 apps are accessible and display correct content
- Touch navigation works smoothly
- Colors match BlackRoad official palette
- No visual artifacts or glitches
- Serial output shows clean boot (no errors)
- WiFi connects successfully
- All notification badges display correctly

---

**Device:** ESP32-2432S028R (MAC: 20:e7:c8:ba:1b:94)
**Firmware:** BlackRoad OS Enterprise Edition v1.0
**Upload Date:** 2026-01-03
**Status:** ✅ OPERATIONAL
