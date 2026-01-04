// WiFi Configuration for BlackRoad CEO Hub
// UPDATE THESE WITH YOUR WIFI CREDENTIALS

#ifndef CONFIG_H
#define CONFIG_H

// WiFi Settings
#define WIFI_SSID "asfghjkl"
#define WIFI_PASSWORD "Smushymush421!"

// Device Name
#define DEVICE_NAME "ceo-hub-esp32"

// SSH Node Configuration
// Add your network's SSH nodes here
struct SSHNode {
  const char* name;
  const char* ip;
  int port;
};

const SSHNode sshNodes[] = {
  {"lucidia", "192.168.4.38", 22},     // Pi 5 - ONLINE
  {"shellfish", "192.168.4.99", 22},   // x86_64 - ONLINE
  {"octavia", "192.168.4.64", 22},     // Pi 5 - ONLINE (aria IP)
  {"alice", "192.168.4.49", 22},       // Pi - ONLINE
  {"aria", "192.168.4.68", 22}         // Check status
};

const int SSH_NODE_COUNT = 5;

#endif
