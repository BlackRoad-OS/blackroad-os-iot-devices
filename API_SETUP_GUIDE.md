# BlackRoad CEO Hub - API Backend Setup Guide

This guide shows you how to set up the backend APIs that the ESP32 CEO Hub connects to for live data.

---

## 🎯 Quick Start

The ESP32 currently connects to three APIs:
1. **Mesh Status API** - Tailscale network health
2. **CRM API** - BlackRoad CRM pipeline metrics
3. **AI Health API** - HuggingFace inference status

**Current State:** All APIs are using **static fallback data** until you deploy the backends.

---

## 1️⃣ BlackRoad CRM API (Already Built!)

### **Status:** ✅ Ready to Deploy

The CRM worker is already built and located at `/tmp/blackroad_worker`.

### **Deployment Steps**

```bash
cd /tmp/blackroad_worker

# Install dependencies
npm install

# Set your API secret
wrangler secret put CRM_SECRET
# Enter a strong secret when prompted (this is what the ESP32 will use)

# Deploy to Cloudflare Workers
wrangler deploy

# Verify deployment
curl https://crm.blackroad.io/api/stats \
  -H "Authorization: Bearer YOUR_SECRET_HERE"
```

### **Expected Response**
```json
{
  "total_contacts": 150,
  "hot_leads": 12,
  "open_deals": 8,
  "pipeline_value": 250000,
  "activity_24h": 45
}
```

### **ESP32 Configuration**

Update `src/secrets.h`:
```cpp
#define CRM_SECRET "your_actual_secret_here"
```

Then recompile and upload:
```bash
pio run --target upload
```

---

## 2️⃣ Mesh Status API (TODO)

### **Status:** ⚠️ Needs Implementation

This API should run on one of your Raspberry Pi nodes (recommended: lucidia).

### **Implementation Option A: Python Flask**

Create `/home/pi/mesh-status/app.py`:

```python
from flask import Flask, jsonify
import subprocess
import json

app = Flask(__name__)

@app.route('/mesh/status')
def mesh_status():
    try:
        # Get Tailscale status
        result = subprocess.run(
            ['tailscale', 'status', '--json'],
            capture_output=True,
            text=True,
            timeout=5
        )

        if result.returncode != 0:
            return jsonify({"error": "Tailscale not running"}), 500

        ts_data = json.loads(result.stdout)

        # Extract node information
        nodes = []
        for peer in ts_data.get('Peer', {}).values():
            nodes.append({
                "name": peer.get('HostName', 'unknown'),
                "ip": peer.get('TailscaleIPs', [''])[0],
                "hostname": peer.get('DNSName', ''),
                "online": peer.get('Online', False),
                "latency": peer.get('LastSeen', 0),  # Convert to ms
                "bandwidth": 0.0,  # TODO: Calculate from traffic stats
                "status": "active" if peer.get('Online') else "offline"
            })

        return jsonify({"nodes": nodes})

    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080)
```

### **Deployment**

```bash
# On lucidia (192.168.4.38)
ssh pi@192.168.4.38

# Install dependencies
sudo apt install python3-pip
pip3 install flask

# Create systemd service
sudo tee /etc/systemd/system/mesh-status.service > /dev/null <<EOF
[Unit]
Description=Mesh Status API
After=network.target

[Service]
Type=simple
User=pi
WorkingDirectory=/home/pi/mesh-status
ExecStart=/usr/bin/python3 /home/pi/mesh-status/app.py
Restart=always

[Install]
WantedBy=multi-user.target
EOF

# Enable and start service
sudo systemctl enable mesh-status
sudo systemctl start mesh-status

# Check status
sudo systemctl status mesh-status

# Test
curl http://localhost:8080/mesh/status
```

### **Implementation Option B: Node.js Express**

Create `/home/pi/mesh-status/server.js`:

```javascript
const express = require('express');
const { exec } = require('child_process');
const app = express();

app.get('/mesh/status', (req, res) => {
  exec('tailscale status --json', (error, stdout, stderr) => {
    if (error) {
      return res.status(500).json({ error: error.message });
    }

    try {
      const tsData = JSON.parse(stdout);
      const nodes = [];

      for (const [key, peer] of Object.entries(tsData.Peer || {})) {
        nodes.push({
          name: peer.HostName || 'unknown',
          ip: peer.TailscaleIPs ? peer.TailscaleIPs[0] : '',
          hostname: peer.DNSName || '',
          online: peer.Online || false,
          latency: peer.LastSeen || 0,
          bandwidth: 0.0,  // TODO: Calculate
          status: peer.Online ? 'active' : 'offline'
        });
      }

      res.json({ nodes });
    } catch (e) {
      res.status(500).json({ error: e.message });
    }
  });
});

app.listen(8080, '0.0.0.0', () => {
  console.log('Mesh Status API listening on port 8080');
});
```

**Deployment:**
```bash
cd /home/pi/mesh-status
npm install express
node server.js  # Or use pm2 for production
```

---

## 3️⃣ AI Health API (TODO)

### **Status:** ⚠️ Needs Implementation

This should run on your HuggingFace infrastructure server.

### **Implementation: Python FastAPI**

Create `/opt/hf-health/main.py`:

```python
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
import psutil
import GPUtil

app = FastAPI()

# Allow ESP32 to access
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["GET"],
    allow_headers=["*"],
)

@app.get("/health")
async def health():
    # Get GPU stats (if available)
    gpu_util = 0.0
    try:
        gpus = GPUtil.getGPUs()
        if gpus:
            gpu_util = gpus[0].load * 100
    except:
        pass

    # TODO: Get actual model stats from your inference server
    # This is placeholder data - integrate with your actual HF setup

    return {
        "model": "Lucidia-7B",
        "status": "running",
        "requests_today": 234,  # Get from logs or metrics
        "avg_latency": 145.2,   # Get from metrics
        "tokens_generated": 45678,  # Get from inference server
        "gpu_util": gpu_util
    }

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=80)
```

**Deployment:**
```bash
pip install fastapi uvicorn gputil psutil

# Run with uvicorn
uvicorn main:app --host 0.0.0.0 --port 80
```

### **DNS Setup**

Point `hf.blackroad.io` to your HF server IP:

```bash
# Cloudflare DNS
# A record: hf.blackroad.io → YOUR_HF_SERVER_IP
```

---

## 🔒 Security Considerations

### **CRM API**
- ✅ Uses Bearer token authentication
- ✅ Requires `CRM_SECRET` to access
- ✅ Deployed on Cloudflare Workers (DDoS protected)

### **Mesh Status API**
- ⚠️ Currently no authentication (local network only)
- 🔒 **Recommended:** Add API key or IP whitelist
- 🔒 **Recommended:** Use Tailscale to restrict access

### **AI Health API**
- ⚠️ Currently no authentication
- 🔒 **Recommended:** Add API key header
- 🔒 **Recommended:** Rate limiting

---

## 🧪 Testing Your APIs

### **Test CRM API**
```bash
curl https://crm.blackroad.io/api/stats \
  -H "Authorization: Bearer YOUR_SECRET" | jq
```

Expected: JSON with `total_contacts`, `hot_leads`, etc.

### **Test Mesh Status API**
```bash
curl http://lucidia.blackroad.network:8080/mesh/status | jq
```

Expected: JSON with `nodes` array

### **Test AI Health API**
```bash
curl https://hf.blackroad.io/health | jq
```

Expected: JSON with `model`, `status`, `requests_today`, etc.

---

## 📊 ESP32 Behavior

### **When All APIs Are Live**
- Purple indicator dots (L) on all screens
- Real-time data updates every 5 minutes
- Accurate metrics across all dashboards

### **When APIs Are Down**
- Amber indicator dots (S) on affected screens
- Static fallback data displayed
- Device continues to function normally
- No crashes or blank screens

### **Transition from Static to Live**
1. Deploy API backend
2. Wait up to 5 minutes for auto-refresh
3. Watch indicator switch from 'S' (amber) to 'L' (purple)
4. Data becomes live!

---

## 🔄 Auto-Refresh Behavior

The ESP32 will:
1. Check WiFi connection status
2. Every 5 minutes, attempt to fetch from all APIs
3. Update indicators based on success/failure
4. Use cached data if API is unreachable

**Manual Refresh:** Reboot the device to force immediate update.

**Future:** Add manual refresh button to force update on demand.

---

## 📈 Monitoring Your APIs

### **Recommended Tools**

1. **Uptime Robot** - Monitor API availability
   - https://uptimerobot.com
   - Set up HTTP monitors for each endpoint
   - Get alerts when APIs go down

2. **Cloudflare Analytics** - CRM API metrics
   - Built into Cloudflare Workers dashboard
   - Shows requests, errors, latency

3. **Grafana + Prometheus** - Custom metrics
   - Track API response times
   - Monitor request counts
   - Alert on errors

---

## 🎯 Next Steps

### **Priority 1: CRM API**
- [ ] Deploy CRM worker to Cloudflare
- [ ] Set CRM_SECRET in wrangler
- [ ] Test endpoint with curl
- [ ] Update ESP32 secrets.h with CRM_SECRET
- [ ] Recompile and upload firmware
- [ ] Watch CRM screen indicator turn purple!

### **Priority 2: Mesh Status API**
- [ ] Choose implementation (Python Flask or Node.js)
- [ ] Deploy to lucidia (192.168.4.38)
- [ ] Set up systemd service
- [ ] Test endpoint
- [ ] Watch Mesh VPN screen turn live!

### **Priority 3: AI Health API**
- [ ] Implement health endpoint on HF server
- [ ] Configure DNS for hf.blackroad.io
- [ ] Test endpoint
- [ ] Watch Infrastructure Dashboard update!

---

## 🐛 Troubleshooting

### **ESP32 Shows 'S' (Static) Instead of 'L' (Live)**

1. Check WiFi connection:
   ```cpp
   // Look for "CONNECTED" on status bar
   ```

2. Check API endpoint is reachable:
   ```bash
   curl http://lucidia.blackroad.network:8080/mesh/status
   ```

3. Check ESP32 serial output:
   ```bash
   pio device monitor --baud 115200
   ```
   Look for lines like:
   ```
   ✅ LIVE (4 nodes)
   ⚠️ STATIC (fallback data)
   ```

4. Check timeout settings in `dynamic_nav.h`:
   ```cpp
   http.setTimeout(3000);  // 3 seconds
   ```

### **API Returns 500 Error**

1. Check server logs
2. Verify Tailscale is running (for mesh API)
3. Check JSON format matches ESP32 expectations

### **ESP32 Crashes on Update**

1. Check serial output for stack trace
2. Verify JSON parsing doesn't overflow buffer
3. Increase `DynamicJsonDocument` size if needed

---

## 📚 Reference Documentation

- **BlackRoad CRM Worker:** `/tmp/blackroad_worker/README.md`
- **Dynamic Navigation:** `/Users/alexa/ceo-hub-esp32/DYNAMIC_NAVIGATION_SUMMARY.md`
- **Tailscale API:** https://tailscale.com/kb/1080/cli/#status
- **ESP32 Firmware:** `/Users/alexa/ceo-hub-esp32/src/main.cpp`

---

**Built for:** BlackRoad OS Sovereign Infrastructure
**Last Updated:** 2026-01-04
**Firmware Version:** 2.0 - Dynamic Infrastructure Edition

🚀 **Your infrastructure, live on your wrist.**
