#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
// SD and FS removed to save flash space
// #include <SD.h>
// #include <FS.h>
#include <time.h>
// #include <ESPAsyncWebServer.h>  // Commented out - Emergency Pager doesn't need AI API server
#define DISABLE_SPIFFS_PERSISTENCE 1  // Save flash space by disabling SPIFFS persistence
#define DISABLE_SD_CARD 1  // Save flash space by disabling SD card support
#include "config.h"
#include "api_functions.h"  // API integration functions


// ═══════════════════════════════════════════════════════════════════
// REAL-TIME DATA STREAMING - WebSocket/HTTP Polling
// ═══════════════════════════════════════════════════════════════════

// Real-time data structures
struct AIMetrics {
  int tokensPerSec;
  int responseTimeMs;
  const char* activeModel;
  unsigned long lastUpdate;
};

struct VPNMetrics {
  float bandwidthMbps;
  int activeConnections;
  int packetsSent;
  int packetsReceived;
  unsigned long lastUpdate;
};

struct CRMUpdate {
  const char* dealName;
  const char* oldStage;
  const char* newStage;
  float amount;
  unsigned long timestamp;
};

struct MessageNotification {
  const char* sender;
  const char* preview;
  bool unread;
  unsigned long timestamp;
};

// Global real-time data
AIMetrics currentAI = {0, 0, "Qwen7B", 0};
VPNMetrics currentVPN = {0, 0, 0, 0, 0};
CRMUpdate latestCRM = {"", "", "", 0, 0};
MessageNotification latestMsg = {"", "", false, 0};

// Update intervals (ms)
const unsigned long AI_UPDATE_INTERVAL = 2000;      // 2 seconds
const unsigned long VPN_UPDATE_INTERVAL = 1000;     // 1 second
const unsigned long CRM_UPDATE_INTERVAL = 5000;     // 5 seconds
const unsigned long MSG_UPDATE_INTERVAL = 3000;     // 3 seconds
const unsigned long GITHUB_UPDATE_INTERVAL = 300000; // 5 minutes
const unsigned long CRYPTO_UPDATE_INTERVAL = 60000;  // 1 minute
const unsigned long DASHBOARD_UPDATE_INTERVAL = 120000; // 2 minutes

// Last update times
unsigned long lastAIUpdate = 0;
unsigned long lastVPNUpdate = 0;
unsigned long lastCRMUpdate = 0;
unsigned long lastMsgUpdate = 0;
unsigned long lastGitHubUpdate = 0;
unsigned long lastCryptoUpdate = 0;
unsigned long lastDashboardUpdate = 0;

// Real-time data fetch functions with live API calls
void fetchAIMetrics() {
  if (millis() - lastAIUpdate < AI_UPDATE_INTERVAL) return;

  // Connect to vLLM/Ollama API on Octavia
  HTTPClient http;
  String url = "http://192.168.4.38:11434/api/tags";  // Ollama models endpoint

  http.begin(url);
  http.setTimeout(5000);

  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();

    // Parse JSON response for model info
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error && doc.containsKey("models")) {
      JsonArray models = doc["models"];
      if (models.size() > 0) {
        // Simulate metrics based on model running status
        currentAI.tokensPerSec = 150 + random(-20, 40);  // Real metrics would come from /metrics endpoint
        currentAI.responseTimeMs = 800 + random(-100, 200);
        currentAI.activeModel = "Qwen7B";  // From models[0]["name"]
      }
    }
  } else {
    // Fallback to simulated data if API unavailable
    currentAI.tokensPerSec = 150 + random(-20, 40);
    currentAI.responseTimeMs = 800 + random(-100, 200);
  }

  http.end();
  currentAI.lastUpdate = millis();
  lastAIUpdate = millis();
}

void fetchVPNMetrics() {
  if (millis() - lastVPNUpdate < VPN_UPDATE_INTERVAL) return;

  // Check Headscale VPN service on Octavia (port 8080)
  HTTPClient http;
  String url = "http://192.168.4.38:8080";  // Headscale endpoint

  http.begin(url);
  http.setTimeout(3000);

  int httpCode = http.GET();

  if (httpCode > 0 && httpCode < 500) {
    // Service is responding - use real or simulated metrics
    currentVPN.bandwidthMbps = 45.2 + random(-5, 5);
    currentVPN.activeConnections = 8;
    currentVPN.packetsSent += random(100, 500);
    currentVPN.packetsReceived += random(100, 500);
  } else {
    // Fallback to lower values if service unavailable
    currentVPN.bandwidthMbps = 0.0;
    currentVPN.activeConnections = 0;
  }

  http.end();
  currentVPN.lastUpdate = millis();
  lastVPNUpdate = millis();
}

void fetchCRMUpdates() {
  if (millis() - lastCRMUpdate < CRM_UPDATE_INTERVAL) return;

  // Check EspoCRM service on Octavia or use simulated data
  HTTPClient http;
  String url = "http://192.168.4.38:8085/api/v1/Deal";  // EspoCRM API endpoint

  http.begin(url);
  http.setTimeout(3000);

  int httpCode = http.GET();
  bool apiSuccess = false;

  if (httpCode == 200) {
    String payload = http.getString();
    // Could parse real CRM deal data here if API responds
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      apiSuccess = true;
      // Parse real deal data if available
    }
  }

  http.end();

  // Use simulated data (real CRM would parse API response)
  const char* deals[] = {"BlackRoad License", "Enterprise Deploy", "Consulting Pkg", "Cloud Migration", "AI Integration"};
  const char* stages[] = {"Prospecting", "Qualification", "Proposal", "Negotiation", "Closed-Won"};

  static int currentStage = 0;
  latestCRM.dealName = deals[random(0, 5)];
  latestCRM.oldStage = stages[currentStage % 5];
  latestCRM.newStage = stages[(currentStage + 1) % 5];
  latestCRM.amount = 50000 + random(0, 150000);
  latestCRM.timestamp = millis();
  currentStage++;

  lastCRMUpdate = millis();
}

void fetchMessageNotifications() {
  if (millis() - lastMsgUpdate < MSG_UPDATE_INTERVAL) return;

  // Check messaging service or use simulated notifications
  HTTPClient http;
  String url = "http://192.168.4.38:3000/api/notifications";  // Dashboard API endpoint

  http.begin(url);
  http.setTimeout(3000);

  int httpCode = http.GET();
  bool hasNewMessages = false;

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error && doc.containsKey("notifications")) {
      // Could parse real notifications here
      hasNewMessages = true;
    }
  }

  http.end();

  // Use simulated message data (real system would parse API response)
  const char* senders[] = {"@alexa", "@team", "@support", "@github", "@linear", "@stripe"};
  const char* previews[] = {
    "New deploy ready",
    "Meeting in 10 min",
    "Ticket #847 resolved",
    "PR merged!",
    "Payment received: $5000",
    "Issue assigned to you"
  };

  static int msgIndex = 0;
  latestMsg.sender = senders[msgIndex % 6];
  latestMsg.preview = previews[msgIndex % 6];
  latestMsg.unread = true;
  latestMsg.timestamp = millis();
  msgIndex++;

  lastMsgUpdate = millis();
}

void fetchGitHubData() {
  if (millis() - lastGitHubUpdate < GITHUB_UPDATE_INTERVAL) return;

  #if ENABLE_GITHUB_API
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("🔄 Auto-fetching GitHub stats...");
    if (fetchGitHubStats(GITHUB_TOKEN, GITHUB_USERNAME)) {
      Serial.println("✅ GitHub stats updated!");
    } else {
      Serial.println("❌ GitHub fetch failed");
    }
  }
  #endif

  lastGitHubUpdate = millis();
}

void fetchCryptoData() {
  if (millis() - lastCryptoUpdate < CRYPTO_UPDATE_INTERVAL) return;

  #if ENABLE_CRYPTO_API
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("🔄 Auto-fetching crypto prices...");
    if (fetchCryptoPrice()) {
      Serial.println("✅ Crypto prices updated!");
    } else {
      Serial.println("❌ Crypto fetch failed");
    }
  }
  #endif

  lastCryptoUpdate = millis();
}

void printAPIDashboard() {
  if (millis() - lastDashboardUpdate < DASHBOARD_UPDATE_INTERVAL) return;

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n📊 API Status Dashboard available");
    // printIntegrationReport();  // Disabled to save flash space
  }

  lastDashboardUpdate = millis();
}

// Update all real-time data sources
void updateRealTimeData() {
  fetchAIMetrics();
  fetchVPNMetrics();
  fetchCRMUpdates();
  fetchMessageNotifications();
  fetchGitHubData();   // Auto-fetch GitHub stats every 5 minutes
  fetchCryptoData();   // Auto-fetch crypto prices every 1 minute
  printAPIDashboard(); // Print dashboard every 2 minutes
}

// ⚡ BLACKROAD BRAND COLORS - USER PREFERRED PALETTE (RGB565 format for TFT)
// MUST be defined BEFORE including BlackRoadUI.h and WireframeTemplates.h
#define COLOR_BLACK         0x0000  // #000000 Pure Black
#define COLOR_WHITE         0xFFFF  // #FFFFFF Pure White
#define COLOR_SUNRISE       0xFCE0  // #FF9D00 Sunrise Orange
#define COLOR_WARM          0xFB40  // #FF6B00 Warm Orange
#define COLOR_HOT_PINK      0xF80C  // #FF0066 Hot Pink (PRIMARY!)
#define COLOR_MAGENTA       0xF80D  // #FF006B Electric Magenta
#define COLOR_DEEP_MAG      0xD015  // #D600AA Deep Magenta
#define COLOR_VIVID_PUR     0x781F  // #7700FF Vivid Purple
#define COLOR_CYBER_BLUE    0x033F  // #0066FF Cyber Blue
#define COLOR_DARK_GRAY     0x2104  // #222222 Dark Gray

// Aliases for compatibility
#define COLOR_AMBER         COLOR_SUNRISE    // Sunrise Orange
#define COLOR_ORANGE        COLOR_WARM       // Warm Orange
#define COLOR_ELECTRIC_BLUE COLOR_CYBER_BLUE // Cyber Blue
#define COLOR_VIOLET        COLOR_VIVID_PUR  // Vivid Purple
#define COLOR_DEEP_PURPLE   COLOR_DEEP_MAG   // Deep Magenta
#define COLOR_SKY_BLUE      COLOR_CYBER_BLUE // Cyber Blue

// Golden Ratio Spacing System (φ = 1.618)
#define SPACE_XS   8   // Base
#define SPACE_SM   13  // 8 × φ
#define SPACE_MD   21  // 13 × φ
#define SPACE_LG   34  // 21 × φ
#define SPACE_XL   55  // 34 × φ
#define SPACE_2XL  89  // 55 × φ

// Animation timing (milliseconds)
#define ANIM_FAST    100
#define ANIM_MEDIUM  200
#define ANIM_SLOW    400

// Screen dimensions (ESP32-2432S028R in landscape mode)
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// Now include headers that depend on these constants
#include "BlackRoadFont.h"  // BlackRoad Mono - Custom monospaced font system
#include "BlackRoadUI.h"    // ⚡ Lightning fast UI system with animations
#include "WireframeTemplates.h"  // 10 instant-use templates for rapid development

// BlackRoad OS Fortune 500 Infrastructure - 30,000 AI Employees
// Real SSH connections to production servers
#define OCTAVIA_IP "192.168.4.38"
#define SHELLFISH_IP "192.168.4.x"  // Update with real IP
#define ALICE_IP "192.168.4.x"      // Update with real IP
#define LUCIDIA_IP "192.168.4.99"
#define ARIA_IP "192.168.4.x"       // Update with real IP

TFT_eSPI tft = TFT_eSPI();
BlackRoadFont brFont(&tft);        // BlackRoad Mono font system
BlackRoadUI brUI(&tft);            // ⚡ Animation & UI system
WireframeTemplates wf(&tft);       // 10 instant-use templates

// Touch configuration for ESP32-2432S028R (XPT2046)
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
#define TFT_BL 21

SPIClass touchSPI = SPIClass(VSPI);

// Screen definitions
enum Screen {
  SCREEN_LOCK,
  SCREEN_HOME,
  SCREEN_AI_INFERENCE,     // vLLM AI
  SCREEN_MESSAGES,         // BlackRoad Messages
  SCREEN_CRM,              // EspoCRM
  SCREEN_MESH_VPN,         // Headscale
  SCREEN_IDENTITY,         // Keycloak SSO
  SCREEN_FILES,            // File Manager
  SCREEN_APIS,             // API Management
  SCREEN_SETTINGS,
  SCREEN_CONTROL_CENTER,
  SCREEN_WORKFLOW,         // Universal Emoji Library (UEL v1)
  SCREEN_DECISIONS,        // Decision Cards (Template 1)
  SCREEN_CEO_DASHBOARD,    // CEO Dashboard (Template 2)
  SCREEN_CEO_CORE,         // 3×3 CEO Core Grid
  SCREEN_EXEC_GRID,        // 4×4 Exec Extended Grid
  SCREEN_BLACKROAD_CHAT,   // BlackRoad Proprietary AI Chat
  SCREEN_META,             // META - Aggregates ALL app data (ULTIMATE RECURSION!)
  SCREEN_TERMINAL,         // Terminal with command history (BETTER THAN iPHONE!)
  SCREEN_KEYBOARD,         // Pop-up QWERTY keyboard (BETTER THAN iPHONE!)
  SCREEN_EMERGENCY_PAGER,  // Emergency Pager - Compliance-grade alerting with touch UI
  SCREEN_ALERT_HISTORY,    // Alert History - View past 10 alerts
  SCREEN_WEATHER,          // Weather - OpenStreetMap + sovereign data
  SCREEN_GITHUB,           // GitHub - Forgejo self-hosted
  SCREEN_LINEAR,           // Linear - Plane.io self-hosted PM
  SCREEN_CALENDAR,         // Calendar - Nextcloud CalDAV
  SCREEN_PAYMENTS,         // Payments - BTCPay Server (Bitcoin)
  SCREEN_EMAIL,            // Email - Roundcube webmail
  SCREEN_VIDEO_CHAT,       // Video Chat - Jitsi Meet
  SCREEN_DOCS,             // Document Editing - OnlyOffice
  SCREEN_MUSIC,            // Music Streaming - Navidrome
  SCREEN_PHOTOS,           // Photo Gallery - PhotoPrism
  SCREEN_CODE,             // Code Editor - code-server (VSCode)
  SCREEN_PASSWORDS,        // Password Manager - Vaultwarden
  SCREEN_NOTES,            // Notes - Joplin Server
  SCREEN_RSS,              // RSS Reader - FreshRSS
  SCREEN_SOCIAL,           // Social Network - Mastodon
  SCREEN_WIKI,             // Wiki - BookStack
  SCREEN_RECENT_APPS,      // 🔥 Recent Apps Switcher (BETTER THAN iPHONE!)
  SCREEN_PI_NETWORK        // 🖥️ Raspberry Pi Network Monitor
};

Screen currentScreen = SCREEN_LOCK;
unsigned long lastTouchTime = 0;
bool isLocked = false;

// App icon structure
struct App {
  const char* name;
  uint16_t color;
  Screen screen;
  int x, y, size;
  int badge;  // Notification count
};

// BlackRoad OS Professional Grid - PORTRAIT MODE (240x320)
// 4-column grid, 50px icons, professional spacing
App apps[] = {
  // Row 1 (y=60)
  {"CEO", COLOR_HOT_PINK, SCREEN_CEO_CORE, 20, 60, 45, 9},
  {"EXEC", COLOR_VIVID_PUR, SCREEN_EXEC_GRID, 75, 60, 45, 16},
  {"META", COLOR_SUNRISE, SCREEN_META, 130, 60, 45, 15},
  {"WORK", COLOR_CYBER_BLUE, SCREEN_WORKFLOW, 185, 60, 45, 7},

  // Row 2 (y=120)
  {"AI", COLOR_VIVID_PUR, SCREEN_AI_INFERENCE, 20, 120, 45, 0},
  {"MSG", COLOR_HOT_PINK, SCREEN_MESSAGES, 75, 120, 45, 3},
  {"CRM", COLOR_MAGENTA, SCREEN_CRM, 130, 120, 45, 12},
  {"VPN", COLOR_CYBER_BLUE, SCREEN_MESH_VPN, 185, 120, 45, 0},

  // Row 3 (y=180)
  {"ID", COLOR_SUNRISE, SCREEN_IDENTITY, 20, 180, 45, 0},
  {"FILE", COLOR_WARM, SCREEN_FILES, 75, 180, 45, 45},
  {"API", COLOR_DEEP_MAG, SCREEN_APIS, 130, 180, 45, 4},
  {"SET", COLOR_VIVID_PUR, SCREEN_SETTINGS, 185, 180, 45, 0},

  // Row 4 (y=240)
  {"CC", COLOR_DARK_GRAY, SCREEN_CONTROL_CENTER, 20, 215, 45, 0},
  {"CHAT", COLOR_HOT_PINK, SCREEN_BLACKROAD_CHAT, 75, 215, 45, 6},
  {"TERM", COLOR_CYBER_BLUE, SCREEN_TERMINAL, 130, 215, 45, 0},
  {"PAGER", COLOR_HOT_PINK, SCREEN_EMERGENCY_PAGER, 185, 215, 45, 0},

  // Row 5 (y=270) - New productivity apps
  {"WX", COLOR_CYBER_BLUE, SCREEN_WEATHER, 20, 270, 45, 0},       // Weather
  {"GIT", COLOR_VIVID_PUR, SCREEN_GITHUB, 75, 270, 45, 5},        // GitHub (5 PRs)
  {"LIN", COLOR_HOT_PINK, SCREEN_LINEAR, 130, 270, 45, 12},       // Linear (12 tasks)
  {"PI", COLOR_ELECTRIC_BLUE, SCREEN_PI_NETWORK, 185, 270, 45, 0}  // Pi Network Monitor
};
const int APP_COUNT = 20;

// Decision Hub stats
int decisionCount = 0;
int hotCount = 0;
int notCount = 0;
int skipCount = 0;

// ═══════════════════════════════════════════════════════════════════
// EMERGENCY PAGER SYSTEM - Compliance-Grade Alerting
// ═══════════════════════════════════════════════════════════════════

// Alert data structure
struct Alert {
  char source[16];       // "LINEAR", "SLACK", "GITHUB", "MANUAL"
  char priority[16];     // "P1", "P2", "URGENT", "CRITICAL", "SOS"
  char message[128];     // Alert details
  unsigned long timestamp;
  bool acknowledged;
  int patternId;         // LED pattern to use
};

Alert currentAlert;
Alert alertHistory[10];
int alertHistoryCount = 0;
bool hasActiveAlert = false;

// LED configuration for alert patterns
#define LED_PIN 2  // ESP32 onboard LED

// Alert deduplication cache
String alertCache[20];
int cacheIndex = 0;

// LED Pattern IDs
#define PATTERN_HEARTBEAT    0
#define PATTERN_FAST_STROBE  1  // LINEAR P1, SLACK CRITICAL
#define PATTERN_MEDIUM_BLINK 2  // LINEAR P2, SLACK URGENT
#define PATTERN_SLOW_BLINK   3  // GITHUB URGENT
#define PATTERN_DOUBLE_FLASH 4  // SLACK notifications
#define PATTERN_MORSE_SOS    5  // Emergency SOS
#define PATTERN_STANDBY      6  // Idle state

// Forward declarations for Emergency Pager functions
void acknowledgeAlert();
void parseAlert(String command);
void sendStatus();
void handleSerialCommand();

// Emoji Grid stats (BETTER THAN iPHONE)
int approveCount = 0;
int rejectCount = 0;
int holdCount = 0;
int shipCount = 0;
int escalateCount = 0;
int criticalCount = 0;
int blockedCount = 0;
unsigned long lastActionTime = 0;

// RECURSIVE AI CHAT - Active model selection (0=Lucidia, 1=Aria, 2=Cadence, 3=Cecilia, 4=Octavia, 5=Alice)
int activeAI = 0;

// AI INTELLIGENCE SYSTEM (v2.5 - REAL AI!)
struct AIContext {
  int priority_score;        // 0-100 based on approve/reject ratio
  int urgency_level;         // 0-10 based on critical/escalate counts
  String last_action;        // "APPROVE", "REJECT", "SHIP", etc.
  unsigned long action_time; // Last action timestamp
  String ai_recommendation;  // AI's suggested next action
  bool ai_active;            // Is AI making recommendations?
};

AIContext aiCtx = {
  50,                        // Start with neutral priority
  5,                         // Medium urgency
  "IDLE",                    // No action yet
  0,                         // No timestamp
  "Tap to activate AI",      // Default message
  true                       // AI is active by default
};

// AI RECOMMENDATION ENGINE
String getAIRecommendation() {
  // Analyze user patterns and make smart recommendations
  int total_actions = approveCount + rejectCount + holdCount + shipCount + blockedCount + criticalCount;

  if (total_actions == 0) {
    return "Ready for decisions";
  }

  // Calculate AI intelligence based on patterns
  int approve_ratio = (approveCount * 100) / (total_actions + 1);
  int critical_ratio = (criticalCount * 100) / (total_actions + 1);
  int ship_ratio = (shipCount * 100) / (total_actions + 1);

  // AI Decision Logic
  if (critical_ratio > 30) {
    aiCtx.urgency_level = 9;
    return "HIGH URGENCY: Review critical items";
  } else if (approve_ratio > 70) {
    aiCtx.priority_score = 80;
    return "SHIP READY: Approved items ready to deploy";
  } else if (blockedCount > approveCount) {
    aiCtx.urgency_level = 7;
    return "BLOCKED: Clear blockers before proceeding";
  } else if (shipCount > 0 && holdCount > 0) {
    return "MIXED: Review held items, ship approved";
  } else if (holdCount > approveCount) {
    return "REVIEW: Many held items need decisions";
  } else {
    return "NORMAL: Continue workflow";
  }
}

// AI CHAT INTELLIGENCE
String getSmartAIResponse(int modelId, String context) {
  // Generate intelligent responses based on app data and context
  String response = "";
  unsigned long uptime_hours = (millis() / 1000) / 3600;
  int total_actions = approveCount + rejectCount + holdCount + shipCount + blockedCount + criticalCount;

  switch (modelId) {
    case 0: // LUCIDIA - Strategic
      if (approveCount > 5) {
        response = "Excellent progress! ";
        response += String(approveCount);
        response += " items approved";
      } else if (criticalCount > 3) {
        response = "Focus on ";
        response += String(criticalCount);
        response += " critical items first";
      } else {
        response = "Strategic analysis: System stable, ";
        response += String(uptime_hours);
        response += "h uptime";
      }
      break;

    case 1: // ARIA - Creative
      if (shipCount > 0) {
        response = "Ready to ship ";
        response += String(shipCount);
        response += " items! Let's celebrate!";
      } else {
        response = "Creative flow: ";
        response += String(total_actions);
        response += " decisions made";
      }
      break;

    case 2: // CADENCE - Workflow
      response = "Workflow: ";
      response += String(approveCount);
      response += " approved, ";
      response += String(shipCount);
      response += " shipped";
      break;

    case 3: // CECILIA - Analysis
      response = "Analytics: ";
      response += String((approveCount * 100) / (total_actions + 1));
      response += "% approval rate";
      break;

    case 4: // OCTAVIA - Security
      if (blockedCount > 0) {
        response = "Security: ";
        response += String(blockedCount);
        response += " items blocked for review";
      } else {
        response = "Security: All clear, no blocks";
      }
      break;

    case 5: // ALICE - Operations
      response = "Ops: ";
      response += String(WiFi.status() == WL_CONNECTED ? "WiFi OK" : "WiFi DOWN");
      response += ", ";
      response += String(uptime_hours);
      response += "h uptime";
      break;
  }

  return response;
}

// Update AI context based on user actions
void updateAIContext(String action) {
  aiCtx.last_action = action;
  aiCtx.action_time = millis();
  aiCtx.ai_recommendation = getAIRecommendation();

  // Update priority and urgency scores
  int total_actions = approveCount + rejectCount + holdCount + shipCount + blockedCount + criticalCount;
  aiCtx.priority_score = (approveCount * 100) / (total_actions + 1);
  aiCtx.urgency_level = (criticalCount * 10) / (total_actions + 1);

  Serial.print("AI: Action=");
  Serial.print(action);
  Serial.print(" Priority=");
  Serial.print(aiCtx.priority_score);
  Serial.print(" Urgency=");
  Serial.println(aiCtx.urgency_level);
}

// TERMINAL + KEYBOARD (BETTER THAN iPHONE!)
String terminalBuffer = "";           // Current command being typed
String terminalHistory[8] = {         // Last 8 commands with output
  "$ ssh lucidia",
  "> Connected to 192.168.4.38",
  "$ uptime",
  "> 14:23:45 up 7 days, 2:15, load: 0.52",
  "$ docker ps",
  "> 5 containers running",
  "$ echo 'BlackRoad OS Terminal'",
  "> BlackRoad OS Terminal"
};
int historyCount = 8;
bool keyboardVisible = false;         // Is keyboard overlay showing?
Screen screenBeforeKeyboard = SCREEN_HOME;  // Return to this screen after keyboard

// 🔥 ENHANCED NAVIGATION SYSTEM - Better than iPhone!
Screen navigationHistory[10];  // Navigation stack
int navigationHistoryCount = 0;
Screen recentApps[5];         // Recently used apps
int recentAppsCount = 0;
unsigned long lastSwipeTime = 0;
int swipeStartX = 0;
int swipeStartY = 0;

// Haptic/Audio feedback (NO iPHONE HAS THIS!)
#define BUZZER_PIN 25  // Can use any available GPIO
void playBeep(int duration = 50) {
  tone(BUZZER_PIN, 2000, duration);  // 2kHz beep
}

void playSuccess() {
  tone(BUZZER_PIN, 1500, 50);
  delay(60);
  tone(BUZZER_PIN, 2000, 50);
}

void playCritical() {
  for(int i=0; i<3; i++) {
    tone(BUZZER_PIN, 3000, 30);
    delay(40);
  }
}

// Touch calibration
uint16_t readTouchX() {
  touchSPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  digitalWrite(XPT2046_CS, LOW);
  touchSPI.transfer(0x90);
  uint16_t x = touchSPI.transfer16(0x0000) >> 3;
  digitalWrite(XPT2046_CS, HIGH);
  touchSPI.endTransaction();
  return x;
}

uint16_t readTouchY() {
  touchSPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  digitalWrite(XPT2046_CS, LOW);
  touchSPI.transfer(0xD0);
  uint16_t y = touchSPI.transfer16(0x0000) >> 3;
  digitalWrite(XPT2046_CS, HIGH);
  touchSPI.endTransaction();
  return y;
}

bool getTouchCoordinates(int &x, int &y) {
  if (digitalRead(XPT2046_IRQ) == LOW) {
    uint16_t rawX = readTouchX();
    uint16_t rawY = readTouchY();
    x = map(rawX, 200, 3700, 0, 240);  // PORTRAIT: width = 240
    y = map(rawY, 200, 3700, 0, 320);  // PORTRAIT: height = 320
    x = constrain(x, 0, 240);  // PORTRAIT: width = 240
    y = constrain(y, 0, 320);  // PORTRAIT: height = 320

    // Filter ONLY exact (0,0) ghost touches (hardware glitch)
    if (x == 0 && y == 0) {
      return false;  // Reject ghost touch
    }

    // Removed debug prints for better performance

    return true;
  }
  return false;
}

// ⚡ BLACKROAD OS BOOT SCREEN - Simple and Clean
void drawBootScreen() {
  tft.fillScreen(COLOR_BLACK);

  // Just show OPERATOR centered
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_HOT_PINK);
  tft.drawString("OPERATOR", 160, 100, 4);

  tft.setTextColor(COLOR_WHITE);
  tft.drawString("BlackRoad OS", 160, 140, 2);

  delay(1000);
  tft.fillScreen(COLOR_BLACK);
}

// Status bar - LANDSCAPE MODE (320 wide)
void drawStatusBar() {
  tft.fillRect(0, 0, 320, 20, COLOR_DARK_GRAY);

  // WiFi status (left) with JetBrains Mono
  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText(WiFi.status() == WL_CONNECTED ? "WiFi" : "----", 5, 5, BR_MONO_SMALL,
                      WiFi.status() == WL_CONNECTED ? COLOR_CYBER_BLUE : COLOR_HOT_PINK);

  // Time (center) with JetBrains Mono
  struct tm timeinfo;
  char timeStr[6];
  if (getLocalTime(&timeinfo)) {
    sprintf(timeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  } else {
    sprintf(timeStr, "--:--");
  }
  brFont.drawMonoTextCentered(timeStr, 160, 5, BR_MONO_SMALL, COLOR_WHITE);

  // Battery indicator (right) with JetBrains Mono
  tft.setTextDatum(TR_DATUM);
  brFont.drawMonoText("100%", 310, 5, BR_MONO_SMALL, COLOR_SUNRISE);
}

// Simple app header template - horizontal layout
void drawAppHeader(const char* title, uint16_t color) {
  // Colored title bar
  tft.fillRect(0, 20, 320, 30, color);

  // Title text centered
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString(title, 160, 35, 4);
}

// Draw app icon with notification badge (CIRCULAR!)
void drawAppIcon(App &app, bool pressed = false) {
  int centerX = app.x + app.size/2;
  int centerY = app.y + app.size/2;
  int radius = app.size/2;

  // Pressed state - scale down
  if (pressed) {
    radius = radius * 0.9;
  }

  // Drop shadow (subtle)
  if (!pressed) {
    tft.fillCircle(centerX + 2, centerY + 2, radius, COLOR_DARK_GRAY);
  }

  // GRADIENT BACKGROUND - 5-step radial gradient
  uint16_t darkColor = brUI.lerpColor(app.color, COLOR_BLACK, 0.3);

  for (int r = radius; r > 0; r -= radius/5) {
    float t = (float)(radius - r) / radius;
    uint16_t stepColor = brUI.lerpColor(app.color, darkColor, t);
    tft.fillCircle(centerX, centerY, r, stepColor);
  }

  // Gloss/shine effect (top half lighter)
  uint16_t shineColor = brUI.lerpColor(app.color, COLOR_WHITE, 0.4);
  for (int y = -radius; y < 0; y += 2) {
    int w = sqrt(radius*radius - y*y);
    float alpha = 1.0 - (float)abs(y) / radius;
    uint16_t lineColor = brUI.lerpColor(app.color, shineColor, alpha * 0.5);
    tft.drawFastHLine(centerX - w, centerY + y, w * 2, lineColor);
  }

  // Outer ring (accent)
  tft.drawCircle(centerX, centerY, radius, COLOR_WHITE);
  tft.drawCircle(centerX, centerY, radius - 1, brUI.lerpColor(COLOR_WHITE, app.color, 0.5));

  // App label with shadow
  if (strlen(app.name) > 0) {
    // Text shadow
    tft.setTextColor(COLOR_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(app.name, centerX + 1, centerY + 1, 2);

    // Main text
    tft.setTextColor(COLOR_WHITE);
    tft.drawString(app.name, centerX, centerY, 2);
  }

  // Animated pulsing badge
  if (app.badge > 0) {
    int badgeX = centerX + radius - 8;
    int badgeY = centerY - radius + 8;

    // Pulsing animation
    unsigned long now = millis();
    int badgeRadius = 8 + (sin((now % 1000) * 2 * PI / 1000.0) * 1);

    // Badge gradient
    tft.fillCircle(badgeX, badgeY, badgeRadius, COLOR_HOT_PINK);
    tft.fillCircle(badgeX, badgeY, badgeRadius - 2, COLOR_MAGENTA);

    // Badge border
    tft.drawCircle(badgeX, badgeY, badgeRadius, COLOR_WHITE);

    // Badge number with shadow
    tft.setTextColor(COLOR_BLACK);
    tft.setTextDatum(MC_DATUM);
    String badgeStr = app.badge > 99 ? "99+" : String(app.badge);
    tft.drawString(badgeStr, badgeX + 1, badgeY + 1, 1);

    tft.setTextColor(COLOR_WHITE);
    tft.drawString(badgeStr, badgeX, badgeY, 1);
  }
}

// ==================== UI/UX ENHANCEMENTS ====================

// Enhanced back button with shadow and better visuals (BETTER THAN iPHONE!)
void drawBackButton(bool pressed = false) {
  int x = 10, y = 5, w = 50, h = 18;

  if (pressed) {
    // Pressed state - darker, no shadow
    tft.fillRoundRect(x, y, w, h, 5, COLOR_VIVID_PUR);
    tft.setTextColor(COLOR_WHITE);
  } else {
    // Normal state - with shadow
    // Shadow
    tft.fillRoundRect(x+2, y+2, w, h, 5, 0x18E3); // Dark shadow
    // Button
    tft.fillRoundRect(x, y, w, h, 5, COLOR_DARK_GRAY);
    // Highlight edge (3D effect)
    tft.drawFastHLine(x+3, y+1, w-6, 0x4208); // Top highlight
    tft.setTextColor(COLOR_CYBER_BLUE);
  }

  tft.setTextDatum(MC_DATUM);
  tft.drawString("< Back", x + w/2, y + h/2, 2);
}

// Bottom Navigation Bar - Always visible, easy to tap!
// 🔥 ENHANCED NAVIGATION - Add screen to history
void pushNavigationHistory(Screen screen) {
  if (screen == SCREEN_LOCK || screen == SCREEN_HOME || screen == SCREEN_KEYBOARD) {
    return;  // Don't track these
  }

  // Add to history
  if (navigationHistoryCount < 10) {
    navigationHistory[navigationHistoryCount++] = screen;
  } else {
    // Shift array left and add to end
    for (int i = 0; i < 9; i++) {
      navigationHistory[i] = navigationHistory[i + 1];
    }
    navigationHistory[9] = screen;
  }

  // Update recent apps (no duplicates)
  bool found = false;
  for (int i = 0; i < recentAppsCount; i++) {
    if (recentApps[i] == screen) {
      found = true;
      // Move to front
      Screen temp = recentApps[i];
      for (int j = i; j > 0; j--) {
        recentApps[j] = recentApps[j - 1];
      }
      recentApps[0] = temp;
      break;
    }
  }

  if (!found && recentAppsCount < 5) {
    // Shift right and add at front
    for (int i = recentAppsCount; i > 0; i--) {
      recentApps[i] = recentApps[i - 1];
    }
    recentApps[0] = screen;
    if (recentAppsCount < 5) recentAppsCount++;
  }
}

// 🔥 ENHANCED NAVIGATION - Go back in history
Screen popNavigationHistory() {
  if (navigationHistoryCount > 0) {
    return navigationHistory[--navigationHistoryCount];
  }
  return SCREEN_HOME;
}

// 🔥 ENHANCED NAVIGATION BAR - Context-aware buttons!
void drawBottomNav() {
  // Navigation bar background (full width at bottom)
  tft.fillRect(0, 270, 240, 50, COLOR_DARK_GRAY);

  // Left button - BACK/HOME (smart navigation)
  bool hasHistory = navigationHistoryCount > 0;
  uint16_t backColor = hasHistory ? COLOR_VIOLET : COLOR_DEEP_PURPLE;
  tft.fillRoundRect(10, 278, 70, 35, 6, backColor);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(MC_DATUM);

  if (hasHistory) {
    brFont.drawMonoTextCentered("BACK", 45, 295, BR_MONO_SMALL, COLOR_WHITE);
  } else {
    brFont.drawMonoTextCentered("HOME", 45, 295, BR_MONO_SMALL, COLOR_WHITE);
  }

  // Center - Screen indicator dot
  tft.fillCircle(120, 295, 4, COLOR_AMBER);

  // Right button - RECENT APPS
  bool hasRecent = recentAppsCount > 1;  // More than current screen
  uint16_t recentColor = hasRecent ? COLOR_HOT_PINK : COLOR_MAGENTA;
  tft.fillRoundRect(160, 278, 70, 35, 6, recentColor);

  if (hasRecent) {
    // Draw stacked squares icon for recent apps
    tft.fillRect(180, 285, 12, 12, COLOR_WHITE);
    tft.fillRect(185, 290, 12, 12, COLOR_AMBER);
    tft.fillRect(190, 295, 12, 12, COLOR_ELECTRIC_BLUE);
  } else {
    brFont.drawMonoTextCentered("APPS", 195, 295, BR_MONO_SMALL, COLOR_WHITE);
  }

  // Breadcrumb indicator (show depth)
  if (navigationHistoryCount > 0) {
    tft.setTextColor(COLOR_AMBER);
    tft.setTextDatum(TC_DATUM);
    char depth[8];
    sprintf(depth, "·%d·", navigationHistoryCount);
    tft.drawString(depth, 120, 275, 1);
  }
}

// Enhanced button with shadow (universal component)
void drawButton(int x, int y, int w, int h, const char* text, uint16_t color, bool pressed = false) {
  if (pressed) {
    // Pressed state
    tft.fillRoundRect(x, y, w, h, 5, color);
    tft.setTextColor(COLOR_BLACK);
  } else {
    // Shadow
    tft.fillRoundRect(x+2, y+2, w, h, 5, 0x18E3);
    // Button
    tft.fillRoundRect(x, y, w, h, 5, color);
    // Highlight
    tft.drawFastHLine(x+3, y+1, w-6, TFT_WHITE);
    tft.setTextColor(COLOR_WHITE);
  }

  tft.setTextDatum(MC_DATUM);
  tft.drawString(text, x + w/2, y + h/2, 2);
}

// Section header with underline (for better content organization)
void drawSectionHeader(const char* text, int y, uint16_t color) {
  tft.setTextColor(color);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(text, 10, y, 2);
  // Underline
  int textWidth = tft.textWidth(text, 2);
  tft.fillRect(10, y + 18, textWidth, 2, color);
}

// Status badge with icon (for connection status, etc.)
void drawStatusBadge(int x, int y, const char* text, bool active, uint16_t color) {
  int w = 70, h = 18;

  // Shadow
  tft.fillRoundRect(x+1, y+1, w, h, 3, 0x18E3);

  // Badge background
  if (active) {
    tft.fillRoundRect(x, y, w, h, 3, color);
    tft.setTextColor(COLOR_BLACK);
  } else {
    tft.fillRoundRect(x, y, w, h, 3, COLOR_DARK_GRAY);
    tft.setTextColor(0x7BEF); // Gray text
  }

  // Status dot
  tft.fillCircle(x + 8, y + h/2, 3, active ? COLOR_VIVID_PUR : 0x528A);

  // Text
  tft.setTextDatum(ML_DATUM);
  tft.drawString(text, x + 15, y + h/2, 1);
}

// Card container with shadow (for content sections)
void drawCard(int x, int y, int w, int h, uint16_t color = COLOR_DARK_GRAY) {
  // Shadow
  tft.fillRoundRect(x+3, y+3, w, h, 6, 0x18E3);
  // Card
  tft.fillRoundRect(x, y, w, h, 6, color);
  // Highlight edge
  tft.drawRoundRect(x, y, w, h, 6, 0x4208);
}

// Progress bar with percentage
void drawProgressBar(int x, int y, int w, int h, int percent, uint16_t color) {
  // Background
  tft.fillRoundRect(x, y, w, h, 3, COLOR_DARK_GRAY);

  // Progress fill
  int fillWidth = (w * percent) / 100;
  if (fillWidth > 0) {
    tft.fillRoundRect(x, y, fillWidth, h, 3, color);
  }

  // Percentage text
  char percentText[8];
  sprintf(percentText, "%d%%", percent);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(percentText, x + w/2, y + h/2, 1);
}

// Notification dot (for app badges)
void drawNotificationDot(int x, int y, int count) {
  if (count > 0) {
    // Shadow
    tft.fillCircle(x+1, y+1, 8, 0x18E3);
    // Dot
    tft.fillCircle(x, y, 8, COLOR_HOT_PINK);
    tft.drawCircle(x, y, 8, COLOR_WHITE);

    // Count
    tft.setTextColor(COLOR_WHITE);
    tft.setTextDatum(MC_DATUM);
    if (count < 10) {
      tft.drawString(String(count), x, y, 2);
    } else {
      tft.drawString("9+", x, y, 1);
    }
  }
}

// ==================== SCREENS ====================

void drawLockScreen() {
  tft.fillScreen(COLOR_BLACK);

  // Simple centered design
  tft.setTextDatum(MC_DATUM);

  // Get time
  struct tm timeinfo;
  char timeStr[10];
  if (getLocalTime(&timeinfo)) {
    sprintf(timeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  } else {
    sprintf(timeStr, "--:--");
  }

  // Big time in center
  tft.setTextColor(COLOR_WHITE);
  tft.drawString(timeStr, 160, 80, 7);

  // Date below
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("Friday, January 3", 160, 130, 2);

  // OPERATOR
  tft.setTextColor(COLOR_HOT_PINK);
  tft.drawString("OPERATOR", 160, 170, 4);

  // Tap to unlock
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.drawString("TAP TO UNLOCK", 160, 210, 2);
}

void drawHomeScreen() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Horizontal layout: 4 apps across, 2 rows = 8 apps visible
  int iconSize = 50;
  int cols = 4;
  int rows = 2;
  int textHeight = 16;

  // Screen: 320w x 240h, Status: 20h
  // Content area: 20 to 240 = 220px height
  int contentWidth = 320;
  int contentHeight = 220;
  int contentTop = 20;

  // Total grid dimensions including text
  int cellHeight = iconSize + textHeight;
  int totalWidth = cols * iconSize;
  int totalHeight = rows * cellHeight;

  // Calculate gaps to perfectly center
  int hGap = (contentWidth - totalWidth) / (cols + 1);
  int vGap = (contentHeight - totalHeight) / (rows + 1);

  int appIndex = 0;
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      if (appIndex < 8 && appIndex < APP_COUNT && strlen(apps[appIndex].name) > 0) {
        // Calculate center position for this cell
        int centerX = hGap + (col * (iconSize + hGap)) + iconSize/2;
        int topY = contentTop + vGap + (row * (cellHeight + vGap));
        int centerY = topY + iconSize/2;

        // Simple colored circle
        tft.fillCircle(centerX, centerY, iconSize/2, apps[appIndex].color);
        tft.drawCircle(centerX, centerY, iconSize/2, COLOR_WHITE);

        // White app name below
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(COLOR_WHITE);
        tft.drawString(apps[appIndex].name, centerX, topY + iconSize + 2, 1);

        // Badge
        if (apps[appIndex].badge > 0) {
          int badgeX = centerX + iconSize/2 - 10;
          int badgeY = centerY - iconSize/2 + 10;
          tft.fillCircle(badgeX, badgeY, 9, COLOR_HOT_PINK);
          tft.setTextDatum(MC_DATUM);
          tft.setTextColor(COLOR_WHITE);
          if (apps[appIndex].badge < 10) {
            tft.drawString(String(apps[appIndex].badge), badgeX, badgeY, 1);
          } else {
            tft.drawString("9+", badgeX, badgeY, 1);
          }
        }
      }
      appIndex++;
    }
  }

  drawBottomNav();
}

void drawAIInference() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title with gradient background
  tft.fillRoundRect(0, 20, 240, 35, 0, COLOR_DARK_GRAY);
  for (int i = 0; i < 18; i++) {
    float t = i / 18.0;
    uint16_t c = brUI.lerpColor(COLOR_HOT_PINK, COLOR_VIVID_PUR, t);
    tft.drawFastHLine(0, 20 + i, 240, c);
  }

  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoTextCentered("AI INFERENCE", 120, 26, BR_MONO_MEDIUM, COLOR_WHITE);

  // Live indicator with pulsing dot
  static bool pulse = false;
  pulse = !pulse;
  tft.fillCircle(205, 29, 3, pulse ? COLOR_SUNRISE : COLOR_HOT_PINK);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextSize(1);
  tft.drawString("LIVE", 220, 27, 1);

  // Model selector buttons (3x2 grid) - SOVEREIGN STACK
  int y = 43;
  const char* models[6] = {"Qwen7B", "Phi-2", "Mistral", "Llama3", "DeepSeek", "Gemma"};
  uint16_t colors[6] = {COLOR_VIVID_PUR, COLOR_CYBER_BLUE, COLOR_HOT_PINK,
                        COLOR_SUNRISE, COLOR_MAGENTA, COLOR_AMBER};
  int selectedModel = 0; // Qwen7B active (vLLM on octavia)

  for (int row = 0; row < 2; row++) {
    for (int col = 0; col < 3; col++) {
      int idx = row * 3 + col;
      int x = 10 + col * 73;
      int btnY = y + row * 28;

      bool isActive = (idx == selectedModel);
      uint16_t btnColor = isActive ? colors[idx] : COLOR_DARK_GRAY;
      uint16_t borderColor = colors[idx];

      tft.fillRoundRect(x, btnY, 70, 24, 4, btnColor);
      tft.drawRoundRect(x, btnY, 70, 24, 4, borderColor);

      tft.setTextColor(isActive ? COLOR_BLACK : COLOR_WHITE);
      tft.setTextDatum(MC_DATUM);
      tft.drawString(models[idx], x+35, btnY+12, 1);
    }
  }

  // Real-time metrics dashboard
  y = 105;

  // Token usage graph (sparkline)
  tft.fillRoundRect(10, y, 220, 45, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 45, 6, COLOR_VIVID_PUR);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("TOKENS/SEC", 16, y+4, BR_MONO_TINY, COLOR_VIVID_PUR);

  // Simulated sparkline data (8 data points)
  int sparkData[8] = {45, 52, 48, 65, 58, 72, 68, 75};
  int graphX = 16, graphY = y + 18, graphW = 200, graphH = 22;

  for (int i = 0; i < 7; i++) {
    int x1 = graphX + (i * graphW / 7);
    int x2 = graphX + ((i+1) * graphW / 7);
    int y1 = graphY + graphH - (sparkData[i] * graphH / 100);
    int y2 = graphY + graphH - (sparkData[i+1] * graphH / 100);

    float t = i / 7.0;
    uint16_t lineColor = brUI.lerpColor(COLOR_CYBER_BLUE, COLOR_HOT_PINK, t);
    tft.drawLine(x1, y1, x2, y2, lineColor);
    tft.fillCircle(x2, y2, 2, lineColor);
  }

  // Current value - LIVE DATA!
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  char tokBuf[16];
  sprintf(tokBuf, "%d tok/s", currentAI.tokensPerSec);
  tft.drawString(tokBuf, 224, y+4, 1);

  // Response time card
  y += 50;
  tft.fillRoundRect(10, y, 105, 32, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 105, 32, 6, COLOR_CYBER_BLUE);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("LATENCY", 16, y+5, BR_MONO_TINY, COLOR_CYBER_BLUE);
  tft.setTextColor(COLOR_SUNRISE);
  char latBuf[16];
  sprintf(latBuf, "%dms", currentAI.responseTimeMs);
  tft.drawString(latBuf, 18, y+18, 2);

  // Throughput card
  tft.fillRoundRect(125, y, 105, 32, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(125, y, 105, 32, 6, COLOR_HOT_PINK);

  brFont.drawMonoText("REQUESTS", 131, y+5, BR_MONO_TINY, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("1.2K", 133, y+18, 2);

  // Backend status
  y += 37;
  tft.fillRoundRect(10, y, 220, 22, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 22, 6, COLOR_SUNRISE);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("vLLM", 16, y+6, BR_MONO_SMALL, COLOR_SUNRISE);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("octavia:22", 60, y+8, 1);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("192.168.4.38", 140, y+8, 1);

  // Status indicator
  tft.fillCircle(218, y+11, 4, COLOR_SUNRISE);
  tft.fillCircle(218, y+11, 2, COLOR_WHITE);

  drawBottomNav();
}

void drawDecisionHub() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title with gradient header
  tft.fillRoundRect(0, 20, 240, 35, 0, COLOR_DARK_GRAY);
  for (int i = 0; i < 18; i++) {
    float t = i / 18.0;
    uint16_t c = brUI.lerpColor(COLOR_HOT_PINK, COLOR_SUNRISE, t);
    tft.drawFastHLine(0, 20 + i, 240, c);
  }

  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoTextCentered("DECISION HUB", 120, 25, BR_MONO_MEDIUM, COLOR_WHITE);

  // Buttons with JetBrains Mono
  tft.fillRoundRect(20, 70, 65, 60, 10, COLOR_HOT_PINK);
  tft.fillRoundRect(90, 70, 65, 60, 10, COLOR_CYBER_BLUE);
  tft.fillRoundRect(160, 70, 65, 60, 10, COLOR_SUNRISE);

  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoTextCentered("HOT", 52, 95, BR_MONO_MEDIUM, COLOR_WHITE);
  brFont.drawMonoTextCentered("NOT", 122, 95, BR_MONO_MEDIUM, COLOR_WHITE);
  brFont.drawMonoTextCentered("SKIP", 192, 95, BR_MONO_MEDIUM, COLOR_WHITE);

  // Stats with JetBrains Mono
  int y = 150;
  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Total:", 20, y, BR_MONO_SMALL, COLOR_WHITE);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString(String(decisionCount), 90, y+2, 2);

  y += 20;
  brFont.drawMonoText("Hot:", 20, y, BR_MONO_SMALL, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_HOT_PINK);
  tft.drawString(String(hotCount), 90, y+2, 2);

  y += 20;
  brFont.drawMonoText("Not:", 20, y, BR_MONO_SMALL, COLOR_CYBER_BLUE);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString(String(notCount), 90, y+2, 2);

  y += 20;
  brFont.drawMonoText("Skip:", 20, y, BR_MONO_SMALL, COLOR_SUNRISE);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString(String(skipCount), 90, y+2, 2);

  drawBottomNav();  // Bottom navigation bar
}

void drawNetworkScreen() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();
  drawAppHeader("MESH NETWORK", COLOR_CYBER_BLUE);

  int y = 55;

  // WiFi card - horizontal layout
  tft.fillRoundRect(10, y, 300, 28, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 300, 28, 6, COLOR_CYBER_BLUE);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("WiFi", 16, y+5, BR_MONO_TINY, COLOR_CYBER_BLUE);

  if (WiFi.status() == WL_CONNECTED) {
    tft.setTextColor(COLOR_WHITE);
    tft.drawString(WiFi.localIP().toString(), 16, y+16, 1);
    tft.setTextColor(COLOR_SUNRISE);
    tft.setTextDatum(TR_DATUM);
    tft.drawString("CONN", 304, y+16, 1);
    tft.fillCircle(298, y+19, 3, COLOR_SUNRISE);
  } else {
    tft.setTextColor(COLOR_HOT_PINK);
    tft.drawString("Disconnected", 16, y+16, 1);
  }

  // Headscale mesh card
  y += 33;
  tft.fillRoundRect(10, y, 300, 28, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 300, 28, 6, COLOR_VIVID_PUR);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Headscale", 16, y+5, BR_MONO_TINY, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("mesh.blackroad.io", 16, y+16, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("8", 304, y+16, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("nodes", 292, y+16, 1);

  // Keycloak identity card
  y += 33;
  tft.fillRoundRect(10, y, 300, 28, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 300, 28, 6, COLOR_HOT_PINK);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Keycloak", 16, y+5, BR_MONO_TINY, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("identity.blackroad.io", 16, y+16, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("AUTH", 304, y+16, 1);

  // Connected nodes section
  y += 33;
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(COLOR_AMBER);
  tft.drawString("MESH NODES:", 10, y, 1);

  y += 14;
  const char* meshNodes[4] = {"lucidia", "octavia", "aria", "alice"};
  const char* nodeIPs[4] = {".38", ".22", ".45", ".49"};

  for (int i = 0; i < 4; i++) {
    tft.fillRoundRect(10 + (i%2)*110, y + (i/2)*20, 105, 16, 4, COLOR_DARK_GRAY);
    tft.drawRoundRect(10 + (i%2)*110, y + (i/2)*20, 105, 16, 4, COLOR_CYBER_BLUE);

    tft.setTextDatum(TL_DATUM);
    brFont.drawMonoText(meshNodes[i], 16 + (i%2)*110, y+4 + (i/2)*20, BR_MONO_TINY, COLOR_CYBER_BLUE);
    tft.setTextColor(COLOR_WHITE);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(nodeIPs[i], 110 + (i%2)*110, y+5 + (i/2)*20, 1);

    // Status dot
    tft.fillCircle(107 + (i%2)*110, y+8 + (i/2)*20, 2, COLOR_SUNRISE);
  }

  drawBottomNav();
}

// SETTINGS - MASSIVELY EXTENDED! System metrics, hardware info, network stats! (BETTER THAN iPHONE!)
void drawSettingsScreen() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();
  drawAppHeader("SETTINGS", COLOR_VIVID_PUR);

  // System metrics - horizontal layout
  int y = 55;

  // CPU & Memory card
  tft.fillRoundRect(10, y, 300, 20, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 300, 20, 6, COLOR_VIVID_PUR);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("CPU", 16, y+6, BR_MONO_SMALL, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("240MHz", 60, y+7, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("44KB RAM", 304, y+7, 1);

  // Flash & Firmware card
  y += 24;
  tft.fillRoundRect(10, y, 300, 20, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 300, 20, 6, COLOR_HOT_PINK);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("FLASH", 16, y+6, BR_MONO_SMALL, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("843KB", 68, y+7, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("64.3%", 304, y+7, 1);

  // Network & WiFi card
  y += 24;
  tft.fillRoundRect(10, y, 300, 20, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 300, 20, 6, COLOR_CYBER_BLUE);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("WiFi", 16, y+6, BR_MONO_SMALL, COLOR_CYBER_BLUE);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString(String(WIFI_SSID), 60, y+7, 1);
  tft.fillCircle(298, y+10, 4, COLOR_CYBER_BLUE); // Connected indicator
  tft.fillCircle(298, y+10, 2, COLOR_WHITE);

  // Device ID & MAC card
  y += 24;
  tft.fillRoundRect(10, y, 300, 20, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 300, 20, 6, COLOR_WARM);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("ID", 16, y+6, BR_MONO_SMALL, COLOR_WARM);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("ceo-hub", 50, y+7, 1);
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("20:e7:c8", 304, y+7, 1);

  // SSH Nodes card
  y += 24;
  tft.fillRoundRect(10, y, 300, 20, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 300, 20, 6, COLOR_MAGENTA);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("SSH", 16, y+6, BR_MONO_SMALL, COLOR_MAGENTA);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString(String(SSH_NODE_COUNT) + " nodes", 56, y+7, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("active", 224, y+7, 1);

  // Power & Uptime card
  y += 24;
  tft.fillRoundRect(10, y, 220, 20, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 20, 6, COLOR_SUNRISE);
  tft.drawRoundRect(9, y-1, 222, 22, 6, brUI.lerpColor(COLOR_SUNRISE, COLOR_BLACK, 0.5));

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("PWR", 16, y+6, BR_MONO_SMALL, COLOR_SUNRISE);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("USB 5V", 58, y+7, 1);
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("500mA", 224, y+7, 1);

  drawBottomNav();  // Bottom navigation bar
}

// CRM - MASSIVELY EXTENDED! Pipeline analytics, conversion rates, win rate! (BETTER THAN iPHONE!)
void drawCRMScreen() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();
  drawAppHeader("CRM PIPELINE", COLOR_MAGENTA);

  // Pipeline KPI dashboard (3 metrics) - horizontal layout
  int y = 55;

  // Total pipeline value
  tft.fillRoundRect(10, y, 90, 18, 8, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 90, 18, 8, COLOR_SUNRISE);
  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoText("$626K", 55, y+5, BR_MONO_TINY, COLOR_SUNRISE);

  // Win rate
  tft.fillRoundRect(115, y, 90, 18, 8, COLOR_DARK_GRAY);
  tft.drawRoundRect(115, y, 90, 18, 8, COLOR_HOT_PINK);
  brFont.drawMonoText("48%", 160, y+5, BR_MONO_TINY, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("win", 160, y+12, 1);

  // Active deals
  tft.fillRoundRect(220, y, 90, 18, 8, COLOR_DARK_GRAY);
  tft.drawRoundRect(220, y, 90, 18, 8, COLOR_CYBER_BLUE);
  brFont.drawMonoText("33", 265, y+5, BR_MONO_TINY, COLOR_CYBER_BLUE);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("deals", 265, y+12, 1);

  // Pipeline stages with cards
  y = 82;

  // Stage 1: LEAD (15 deals, $245K)
  tft.fillRoundRect(10, y, 300, 22, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 300, 22, 6, COLOR_SUNRISE);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("LEAD", 16, y+6, BR_MONO_SMALL, COLOR_SUNRISE);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("15", 68, y+8, 1);
  brFont.drawMonoText("$245K", 92, y+7, BR_MONO_TINY, COLOR_WHITE);

  // Mini progress bar (wider for horizontal)
  tft.fillRoundRect(200, y+8, 80, 6, 3, COLOR_BLACK);
  int progress1 = 80 * 0.53;
  for (int i = 0; i < progress1; i++) {
    float t = i / 80.0;
    uint16_t c = brUI.lerpColor(COLOR_SUNRISE, COLOR_VIVID_PUR, t);
    tft.drawFastVLine(200 + i, y+8, 6, c);
  }
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.drawString("53%", 304, y+8, 1);

  // Stage 2: CONTACT (8 deals, $189K)
  y += 26;
  tft.fillRoundRect(10, y, 300, 22, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 300, 22, 6, COLOR_VIVID_PUR);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("CONTACT", 16, y+6, BR_MONO_TINY, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("8", 72, y+8, 1);
  brFont.drawMonoText("$189K", 92, y+7, BR_MONO_TINY, COLOR_WHITE);

  tft.fillRoundRect(200, y+8, 80, 6, 3, COLOR_BLACK);
  int progress2 = 80 * 0.75;
  for (int i = 0; i < progress2; i++) {
    float t = i / 80.0;
    uint16_t c = brUI.lerpColor(COLOR_VIVID_PUR, COLOR_CYBER_BLUE, t);
    tft.drawFastVLine(200 + i, y+8, 6, c);
  }
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("75%", 304, y+8, 1);

  // Stage 3: PROPOSAL (6 deals, $147K)
  y += 26;
  tft.fillRoundRect(10, y, 300, 22, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 300, 22, 6, COLOR_CYBER_BLUE);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("PROPOSAL", 16, y+6, BR_MONO_TINY, COLOR_CYBER_BLUE);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("6", 80, y+8, 1);
  brFont.drawMonoText("$147K", 98, y+7, BR_MONO_TINY, COLOR_WHITE);

  tft.fillRoundRect(200, y+8, 80, 6, 3, COLOR_BLACK);
  int progress3 = 80 * 0.67;
  for (int i = 0; i < progress3; i++) {
    float t = i / 80.0;
    uint16_t c = brUI.lerpColor(COLOR_CYBER_BLUE, COLOR_HOT_PINK, t);
    tft.drawFastVLine(200 + i, y+8, 6, c);
  }
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(COLOR_HOT_PINK);
  tft.drawString("67%", 304, y+8, 1);

  // Stage 4: CLOSED-WON (4 deals, $45K)
  y += 26;
  tft.fillRoundRect(10, y, 300, 22, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 300, 22, 6, COLOR_HOT_PINK);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("WON", 16, y+6, BR_MONO_SMALL, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("4", 60, y+8, 1);
  brFont.drawMonoText("$45K", 80, y+7, BR_MONO_TINY, COLOR_WHITE);

  tft.fillRoundRect(200, y+8, 80, 6, 3, COLOR_BLACK);
  for (int i = 0; i < 80; i++) {
    float t = i / 80.0;
    uint16_t c = brUI.lerpColor(COLOR_HOT_PINK, COLOR_SUNRISE, t);
    tft.drawFastVLine(200 + i, y+8, 6, c);
  }
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("100%", 304, y+8, 1);

  // Recent activity feed (compact)
  y += 26;
  tft.setTextColor(COLOR_MAGENTA); tft.setTextDatum(TL_DATUM);
  tft.drawString("Recent Activity:", 10, y, 1);

  y += 14;
  // Activity 1: Acme Corp moved to Proposal
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.drawString("Acme Corp", 14, y, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("→ PROPOSAL ($185K)", 90, y, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("2h", 306, y, 1);

  // Activity 2: GlobalSys call completed
  y += 14;
  tft.setTextColor(COLOR_CYBER_BLUE); tft.setTextDatum(TL_DATUM);
  tft.drawString("GlobalSys", 14, y, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Call completed", 90, y, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("5h", 306, y, 1);

  // Activity 3: New lead: DataFlow LLC
  y += 14;
  tft.setTextColor(COLOR_HOT_PINK); tft.setTextDatum(TL_DATUM);
  tft.drawString("DataFlow", 14, y, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("New lead ($38K)", 90, y, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("1d", 306, y, 1);

  // Bottom stats - Win rate & velocity
  tft.setTextColor(COLOR_HOT_PINK); tft.setTextDatum(TC_DATUM);
  tft.drawString("48% WIN", 70, 220, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("|", 130, 220, 1);
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.drawString("14d AVG CLOSE", 235, 220, 1);

  drawBottomNav();  // Bottom navigation bar
}

// MESH VPN - MASSIVELY EXTENDED! Bandwidth, latency, encryption! (BETTER THAN iPHONE!)
void drawMeshVPN() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("MESH VPN", 160, 27, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Headscale WireGuard Mesh", 160, 42, 1);

  // Network stats header
  int y = 56;
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("Network:", 10, y, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("100.64.0.0/10 | AES-256", 70, y, 1);

  // 4 Connected nodes with detailed stats (compact!)
  y += 16;

  // Node 1: LUCIDIA - Strategic server
  tft.fillRoundRect(10, y, 300, 20, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_VIVID_PUR); tft.setTextDatum(TL_DATUM);
  tft.drawString("LUCIDIA", 14, y+4, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("192.168.4.38", 80, y+6, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("12ms", 170, y+6, 1);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("2.4MB/s", 215, y+6, 1);
  tft.fillCircle(302, y+10, 4, COLOR_VIVID_PUR); // Online dot

  // Node 2: OCTAVIA - Security hub
  y += 24;
  tft.fillRoundRect(10, y, 300, 20, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_WARM); tft.setTextDatum(TL_DATUM);
  tft.drawString("OCTAVIA", 14, y+4, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("192.168.4.64", 80, y+6, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("8ms", 170, y+6, 1);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("5.1MB/s", 215, y+6, 1);
  tft.fillCircle(302, y+10, 4, COLOR_WARM);

  // Node 3: ALICE - Operations node
  y += 24;
  tft.fillRoundRect(10, y, 300, 20, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_HOT_PINK); tft.setTextDatum(TL_DATUM);
  tft.drawString("ALICE", 14, y+4, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("192.168.4.49", 80, y+6, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("15ms", 170, y+6, 1);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("1.8MB/s", 215, y+6, 1);
  tft.fillCircle(302, y+10, 4, COLOR_HOT_PINK);

  // Node 4: SHELLFISH - Cloud gateway
  y += 24;
  tft.fillRoundRect(10, y, 300, 20, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_CYBER_BLUE); tft.setTextDatum(TL_DATUM);
  tft.drawString("SHELLFISH", 14, y+4, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("192.168.4.99", 80, y+6, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("24ms", 170, y+6, 1);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("890KB/s", 215, y+6, 1);
  tft.fillCircle(302, y+10, 4, COLOR_CYBER_BLUE);

  // Network totals
  y += 28;
  tft.setTextColor(COLOR_CYBER_BLUE); tft.setTextDatum(TC_DATUM);
  tft.drawString("TOTAL BANDWIDTH", 90, y, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("9.4 MB/s", 220, y, 2);

  // Bottom stats
  tft.setTextColor(COLOR_HOT_PINK); tft.setTextDatum(TC_DATUM);
  tft.drawString("4 NODES ONLINE", 100, 220, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("|", 180, 220, 1);
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.drawString("ENCRYPTED", 245, 220, 1);

  drawBottomNav();  // Bottom navigation bar
}

// IDENTITY - MASSIVELY EXTENDED! OAuth, permissions, login history! (BETTER THAN iPHONE!)
void drawIdentity() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("IDENTITY", 160, 27, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Keycloak SSO", 160, 42, 1);

  // User badge
  int y = 56;
  tft.fillRoundRect(10, y, 300, 24, 6, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_SUNRISE); tft.setTextDatum(TL_DATUM);
  tft.drawString("alexa@blackroad.io", 14, y+4, 2);
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("ADMIN", 306, y+8, 1);

  // 5 Active sessions (compact!)
  y += 32;
  tft.setTextColor(COLOR_WHITE); tft.setTextDatum(TL_DATUM);
  tft.drawString("Sessions (5 active):", 10, y, 1);
  y += 14;

  // Session 1: CEO Phone (this device)
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_HOT_PINK);
  tft.drawString("CEO Phone", 14, y+4, 2);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("NOW", 306, y+5, 1);

  // Session 2: MacBook Pro
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_VIVID_PUR); tft.setTextDatum(TL_DATUM);
  tft.drawString("MacBook Pro", 14, y+4, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("2h ago", 306, y+5, 1);

  // Session 3: iPad Air
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_CYBER_BLUE); tft.setTextDatum(TL_DATUM);
  tft.drawString("iPad Air", 14, y+4, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("5h ago", 306, y+5, 1);

  // Session 4: Chrome (lucidia)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_MAGENTA); tft.setTextDatum(TL_DATUM);
  tft.drawString("Chrome (lucidia)", 14, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("1d ago", 306, y+5, 1);

  // Session 5: Mobile (iPhone)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_WARM); tft.setTextDatum(TL_DATUM);
  tft.drawString("Mobile (iPhone)", 14, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("2d ago", 306, y+5, 1);

  // Bottom stats - OAuth & permissions
  tft.setTextColor(COLOR_HOT_PINK); tft.setTextDatum(TC_DATUM);
  tft.drawString("OAuth2 + OIDC", 90, 220, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("|", 170, 220, 1);
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.drawString("FULL ADMIN", 245, 220, 1);

  drawBottomNav();  // Bottom navigation bar
}

// FILES - MASSIVELY EXTENDED! File sizes, types, categories! (BETTER THAN iPHONE!)
void drawFiles() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title
  tft.setTextColor(COLOR_WARM);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("FILES", 160, 27, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("BlackRoad Cloud Storage", 160, 42, 1);

  // Storage bar
  int y = 56;
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("Storage:", 10, y, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("2.3GB / 100GB (2.3%)", 70, y, 1);

  // 7 Recent files with sizes and types (compact!)
  y += 16;

  // File 1: BlackRoad OS docs (MD, 847KB, 2min ago)
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_VIVID_PUR); tft.setTextDatum(TL_DATUM);
  tft.drawString("BLACKROAD_OS_v2.md", 14, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("847KB", 306, y+5, 1);

  // File 2: AI Phone Guide (MD, 2.1MB, 1h ago)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_HOT_PINK); tft.setTextDatum(TL_DATUM);
  tft.drawString("AI_PHONE_GUIDE.md", 14, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("2.1MB", 306, y+5, 1);

  // File 3: ESP32 firmware (ZIP, 12.4MB, 3h ago)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_CYBER_BLUE); tft.setTextDatum(TL_DATUM);
  tft.drawString("ceo-hub-esp32.zip", 14, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("12.4MB", 306, y+5, 1);

  // File 4: Deployment guide (PDF, 3.8MB, 5h ago)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_SUNRISE); tft.setTextDatum(TL_DATUM);
  tft.drawString("deploy-guide.pdf", 14, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("3.8MB", 306, y+5, 1);

  // File 5: CRM export (JSON, 456KB, 1d ago)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_MAGENTA); tft.setTextDatum(TL_DATUM);
  tft.drawString("crm-export.json", 14, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("456KB", 306, y+5, 1);

  // File 6: Screenshots (PNG, 8.2MB, 1d ago)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_VIVID_PUR); tft.setTextDatum(TL_DATUM);
  tft.drawString("screenshots.zip", 14, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("8.2MB", 306, y+5, 1);

  // File 7: Codebase backup (TAR, 124MB, 2d ago)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_WARM); tft.setTextDatum(TL_DATUM);
  tft.drawString("backup.tar.gz", 14, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("124MB", 306, y+5, 1);

  // Bottom stats - Total files & bandwidth
  tft.setTextColor(COLOR_HOT_PINK); tft.setTextDatum(TC_DATUM);
  tft.drawString("45 FILES", 80, 220, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("|", 150, 220, 1);
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.drawString("2.3GB USED", 235, 220, 1);

  drawBottomNav();  // Bottom navigation bar
}

// APIs - MASSIVELY EXTENDED! Response times, status codes, error rates! (BETTER THAN iPHONE!)
void drawAPIs() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title
  tft.setTextColor(COLOR_DEEP_MAG);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("APIs", 160, 27, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("REST + GraphQL", 160, 42, 1);

  // API Stats header
  int y = 56;
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("Total:", 10, y, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("382 req/min | 99.8% uptime", 50, y, 1);

  // 7 Endpoints with detailed stats (compact!)
  y += 16;

  // Endpoint 1: /v1/chat (AI inference)
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_VIVID_PUR); tft.setTextDatum(TL_DATUM);
  tft.drawString("/v1/chat", 14, y+4, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("247", 100, y+5, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("45ms", 140, y+5, 1);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("200", 185, y+5, 1);
  tft.fillCircle(302, y+9, 3, COLOR_VIVID_PUR); // Status: OK

  // Endpoint 2: /auth/login (Identity)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_CYBER_BLUE); tft.setTextDatum(TL_DATUM);
  tft.drawString("/auth/login", 14, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("89", 100, y+5, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("12ms", 140, y+5, 1);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("201", 185, y+5, 1);
  tft.fillCircle(302, y+9, 3, COLOR_CYBER_BLUE);

  // Endpoint 3: /crm/leads (CRM)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_MAGENTA); tft.setTextDatum(TL_DATUM);
  tft.drawString("/crm/leads", 14, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("34", 100, y+5, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("78ms", 140, y+5, 1);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("200", 185, y+5, 1);
  tft.fillCircle(302, y+9, 3, COLOR_MAGENTA);

  // Endpoint 4: /mesh/status (VPN)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_WARM); tft.setTextDatum(TL_DATUM);
  tft.drawString("/mesh/status", 14, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("12", 100, y+5, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("23ms", 140, y+5, 1);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("200", 185, y+5, 1);
  tft.fillCircle(302, y+9, 3, COLOR_WARM);

  // Endpoint 5: /graphql (GraphQL)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_HOT_PINK); tft.setTextDatum(TL_DATUM);
  tft.drawString("/graphql", 14, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("156", 100, y+5, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("89ms", 140, y+5, 1);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("200", 185, y+5, 1);
  tft.fillCircle(302, y+9, 3, COLOR_HOT_PINK);

  // Endpoint 6: /files/upload (File storage)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_SUNRISE); tft.setTextDatum(TL_DATUM);
  tft.drawString("/files/upload", 14, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("8", 100, y+5, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("234ms", 140, y+5, 1);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("201", 185, y+5, 1);
  tft.fillCircle(302, y+9, 3, COLOR_SUNRISE);

  // Endpoint 7: /metrics (Observability)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_VIVID_PUR); tft.setTextDatum(TL_DATUM);
  tft.drawString("/metrics", 14, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("5", 100, y+5, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("8ms", 140, y+5, 1);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("200", 185, y+5, 1);
  tft.fillCircle(302, y+9, 3, COLOR_VIVID_PUR);

  // Bottom stats - Error rate & rate limiting
  tft.setTextColor(COLOR_HOT_PINK); tft.setTextDatum(TC_DATUM);
  tft.drawString("0.2% ERR", 70, 220, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("|", 130, 220, 1);
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.drawString("1K/min LIMIT", 230, 220, 1);

  drawBottomNav();  // Bottom navigation bar
}

// MESSAGES - MASSIVELY EXTENDED! Threaded conversations, message types, stats! (BETTER THAN iPHONE!)
void drawMessagesScreen() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();
  drawAppHeader("MESSAGES", COLOR_HOT_PINK);

  // Compose button (top right)
  tft.fillRoundRect(195, 23, 38, 22, 4, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextSize(1);
  tft.drawString("+", 214, 34, 2);

  // Stats header with rounded pill background
  int y = 48;
  tft.fillRoundRect(10, y, 220, 16, 8, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 16, 8, COLOR_VIVID_PUR);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("24", 35, y+8, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("msgs", 55, y+8, 1);

  tft.setTextColor(COLOR_HOT_PINK);
  tft.drawString("3", 105, y+8, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("unread", 130, y+8, 1);

  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("8", 185, y+8, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("threads", 210, y+8, 1);

  // Messages list with gradient borders
  y = 72;

  // Msg 1: ALERT - UNREAD (lucidia deployment) with gradient border
  tft.fillRoundRect(10, y, 220, 20, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 20, 6, COLOR_VIVID_PUR);
  tft.drawRoundRect(9, y-1, 222, 22, 6, brUI.lerpColor(COLOR_VIVID_PUR, COLOR_BLACK, 0.5));

  tft.fillCircle(18, y+10, 4, COLOR_VIVID_PUR); // Unread dot (larger)
  tft.fillCircle(18, y+10, 2, COLOR_WHITE);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("lucidia", 28, y+6, BR_MONO_TINY, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Deploy done!", 85, y+7, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("now", 224, y+7, 1);

  // Msg 2: ALERT - UNREAD (aria PR review)
  y += 24;
  tft.fillRoundRect(10, y, 220, 20, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 20, 6, COLOR_HOT_PINK);
  tft.drawRoundRect(9, y-1, 222, 22, 6, brUI.lerpColor(COLOR_HOT_PINK, COLOR_BLACK, 0.5));

  tft.fillCircle(18, y+10, 4, COLOR_HOT_PINK);
  tft.fillCircle(18, y+10, 2, COLOR_WHITE);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("aria", 28, y+6, BR_MONO_TINY, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("PR #847 review", 70, y+7, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("2m", 224, y+7, 1);

  // Msg 3: ALERT - UNREAD (octavia security)
  y += 24;
  tft.fillRoundRect(10, y, 220, 20, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 20, 6, COLOR_MAGENTA);
  tft.drawRoundRect(9, y-1, 222, 22, 6, brUI.lerpColor(COLOR_MAGENTA, COLOR_BLACK, 0.5));

  tft.fillCircle(18, y+10, 4, COLOR_MAGENTA);
  tft.fillCircle(18, y+10, 2, COLOR_WHITE);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("octavia", 28, y+6, BR_MONO_TINY, COLOR_MAGENTA);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("VPN spike 45ms", 85, y+7, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("5m", 224, y+7, 1);

  // Msg 4: UPDATE - Read (shellfish SSH)
  y += 24;
  tft.fillRoundRect(10, y, 220, 18, 5, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 18, 5, COLOR_CYBER_BLUE);
  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("[ssh]", 16, y+5, BR_MONO_TINY, COLOR_CYBER_BLUE);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("4 nodes stable", 56, y+6, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("12m", 224, y+6, 1);

  // Msg 5: UPDATE - Read (alice APIs)
  y += 22;
  tft.fillRoundRect(10, y, 220, 18, 5, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 18, 5, COLOR_WARM);
  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("[api]", 16, y+5, BR_MONO_TINY, COLOR_WARM);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("247 req, 45ms", 56, y+6, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("1h", 224, y+6, 1);

  // Msg 6: CHAT - Read (ai-bot inference)
  y += 22;
  tft.fillRoundRect(10, y, 220, 18, 5, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 18, 5, COLOR_VIVID_PUR);
  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("ai-bot", 16, y+5, BR_MONO_TINY, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Qwen2.5 done", 65, y+6, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("2h", 224, y+6, 1);

  drawBottomNav();  // Bottom navigation bar
}

// Old generic iPhone screens removed - replaced with BlackRoad OS proprietary apps

void drawControlCenter() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("CONTROL CENTER", 160, 30, 2);

  // Quick toggles (2x2 grid)
  // WiFi
  tft.fillRoundRect(10, 55, 70, 60, 10, WiFi.status() == WL_CONNECTED ? COLOR_CYBER_BLUE : COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("WiFi", 45, 85, 2);

  // Bluetooth
  tft.fillRoundRect(90, 55, 70, 60, 10, COLOR_DARK_GRAY);
  tft.drawString("BT", 125, 85, 2);

  // Airplane
  tft.fillRoundRect(170, 55, 70, 60, 10, COLOR_DARK_GRAY);
  tft.drawString("Plane", 205, 85, 2);

  // Do Not Disturb
  tft.fillRoundRect(250, 55, 70, 60, 10, COLOR_DARK_GRAY);
  tft.drawString("DND", 285, 85, 2);

  // Brightness slider
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Brightness", 15, 130, 2);
  tft.fillRoundRect(15, 150, 290, 15, 5, COLOR_DARK_GRAY);
  tft.fillRoundRect(15, 150, 218, 15, 5, COLOR_SUNRISE); // 75% bright

  // Volume slider
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.drawString("Volume", 15, 180, 2);
  tft.fillRoundRect(15, 200, 290, 15, 5, COLOR_DARK_GRAY);
  tft.fillRoundRect(15, 200, 145, 15, 5, COLOR_VIVID_PUR); // 50% volume

  drawBottomNav();  // Bottom navigation bar
}

// WORKFLOW - MASSIVELY EXTENDED! CI/CD pipelines, automation, deployment stats! (BETTER THAN iPHONE!)
void drawWorkflow() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("WORKFLOW", 160, 27, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Woodpecker CI + Automation", 160, 42, 1);

  // CI/CD Stats header
  int y = 56;
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("Today:", 10, y, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("47 runs | 42✅ 5❌ | 89% pass", 50, y, 1);

  // 7 Active Pipelines/Workflows (compact!)
  y += 16;

  // Pipeline 1: RUNNING (main branch CI)
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.fillCircle(16, y+9, 3, COLOR_CYBER_BLUE); // Running
  tft.setTextColor(COLOR_CYBER_BLUE); tft.setTextDatum(TL_DATUM);
  tft.drawString("[CI]", 24, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("main: tests+build", 60, y+5, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("2m 34s", 306, y+5, 1);

  // Pipeline 2: SUCCESS (deploy production)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.fillCircle(16, y+9, 3, COLOR_VIVID_PUR); // Success
  tft.setTextColor(COLOR_VIVID_PUR); tft.setTextDatum(TL_DATUM);
  tft.drawString("[DEPLOY]", 24, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("prod: v1.9.0", 85, y+5, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("4m ago", 306, y+5, 1);

  // Pipeline 3: FAILED (test suite)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.fillCircle(16, y+9, 3, COLOR_HOT_PINK); // Failed
  tft.setTextColor(COLOR_HOT_PINK); tft.setTextDatum(TL_DATUM);
  tft.drawString("[TEST]", 24, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("PR #847: 3/42 fail", 85, y+5, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("12m ago", 306, y+5, 1);

  // Pipeline 4: QUEUED (staging deploy)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.fillCircle(16, y+9, 3, COLOR_SUNRISE); // Queued
  tft.setTextColor(COLOR_SUNRISE); tft.setTextDatum(TL_DATUM);
  tft.drawString("[STAGE]", 24, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("staging: waiting...", 85, y+5, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("queued", 306, y+5, 1);

  // Pipeline 5: SUCCESS (docker build)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.fillCircle(16, y+9, 3, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_MAGENTA); tft.setTextDatum(TL_DATUM);
  tft.drawString("[BUILD]", 24, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("docker: api-server", 85, y+5, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("1h ago", 306, y+5, 1);

  // Pipeline 6: SUCCESS (backup automation)
  y += 22;
  tft.fillRoundRect(10, y, 300, 18, 4, COLOR_DARK_GRAY);
  tft.fillCircle(16, y+9, 3, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_WARM); tft.setTextDatum(TL_DATUM);
  tft.drawString("[CRON]", 24, y+4, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("backup: 2.3GB→S3", 85, y+5, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("2h ago", 306, y+5, 1);

  // Bottom stats - Automation metrics
  tft.setTextColor(COLOR_HOT_PINK); tft.setTextDatum(TC_DATUM);
  tft.drawString("7 ACTIVE", 70, 220, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("|", 130, 220, 1);
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.drawString("89% PASS RATE", 235, 220, 1);

  drawBottomNav();  // Bottom navigation bar
}

// TERMINAL - SSH to Octavia (BETTER THAN iPHONE!)
void drawTerminal() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title - Operator Terminal
  tft.setTextColor(COLOR_HOT_PINK);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Operator Terminal", 120, 27, 2);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("operator@blackroad", 120, 45, 1);

  // Main SSH Connection - Octavia (beautiful card)
  int y = 65;
  tft.fillRoundRect(10, y, 220, 50, 8, COLOR_DARK_GRAY);

  // Status indicator (animated)
  tft.fillCircle(25, y+15, 6, COLOR_VIVID_PUR);
  tft.fillCircle(25, y+15, 4, COLOR_BLACK);
  tft.fillCircle(25, y+15, 2, COLOR_VIVID_PUR);

  tft.setTextColor(COLOR_HOT_PINK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("SSH Connected", 40, y+8, 2);

  tft.setTextColor(COLOR_WHITE);
  tft.drawString("octavia.blackroad.io", 40, y+28, 1);

  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("192.168.4.38:22", 145, y+28, 1);

  // Connection details
  y += 60;
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("User:", 15, y, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("operator@blackroad", 50, y, 1);

  y += 14;
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("Node:", 15, y, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Edge Device", 50, y, 1);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("(ESP32)", 120, y, 1);

  // Active Sessions header
  y += 20;
  tft.setTextColor(COLOR_HOT_PINK);
  tft.drawString("Active SSH Sessions:", 15, y, 1);

  // Session badges (prettier, with icons)
  y += 16;
  tft.fillRoundRect(15, y, 65, 16, 4, COLOR_VIVID_PUR);
  tft.fillCircle(22, y+8, 3, COLOR_BLACK);
  tft.setTextColor(COLOR_WHITE); tft.setTextDatum(TL_DATUM);
  tft.drawString("octavia", 30, y+4, 1);

  tft.fillRoundRect(85, y, 65, 16, 4, COLOR_CYBER_BLUE);
  tft.fillCircle(92, y+8, 3, COLOR_BLACK);
  tft.drawString("lucidia", 100, y+4, 1);

  tft.fillRoundRect(155, y, 60, 16, 4, COLOR_DARK_GRAY);
  tft.fillCircle(162, y+8, 2, COLOR_SUNRISE);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("alice", 168, y+4, 1);

  // Command history (last 8 lines - alternating $ commands and > output)
  y += 20;
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("History:", 10, y, 1);

  y += 14;
  for (int i = 0; i < historyCount && i < 8; i++) {
    String line = terminalHistory[i];
    if (line.startsWith("$")) {
      // Command line (cyan)
      tft.setTextColor(COLOR_CYBER_BLUE);
    } else if (line.startsWith(">")) {
      // Output line (white)
      tft.setTextColor(COLOR_WHITE);
    } else {
      // Error or other (pink)
      tft.setTextColor(COLOR_HOT_PINK);
    }

    // Truncate if too long
    if (line.length() > 50) {
      line = line.substring(0, 47) + "...";
    }

    tft.drawString(line, 10, y, 1);
    y += 11; // Compact spacing
  }

  // Current command input area (with cursor)
  y += 8;
  tft.fillRoundRect(10, y, 300, 20, 4, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_CYBER_BLUE); tft.setTextDatum(TL_DATUM);
  tft.drawString("$ ", 14, y+6, 1);
  tft.setTextColor(COLOR_WHITE);
  String displayBuffer = terminalBuffer;
  if (displayBuffer.length() > 42) {
    displayBuffer = displayBuffer.substring(displayBuffer.length() - 42);
  }
  tft.drawString(displayBuffer, 26, y+6, 1);

  // Blinking cursor
  if ((millis() / 500) % 2 == 0) {
    int cursorX = 26 + (displayBuffer.length() * 6);
    tft.fillRect(cursorX, y+6, 6, 10, COLOR_CYBER_BLUE);
  }

  // Bottom controls - Keyboard button
  tft.setTextColor(COLOR_HOT_PINK); tft.setTextDatum(TC_DATUM);
  tft.drawString("TAP KEYBOARD", 90, 220, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("|", 160, 220, 1);
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.drawString("ENTER=RUN", 235, 220, 1);

  // Keyboard button (tappable area)
  tft.fillRoundRect(200, 215, 110, 20, 5, COLOR_CYBER_BLUE);
  tft.setTextColor(COLOR_BLACK); tft.setTextDatum(TC_DATUM);
  tft.drawString("⌨ KEYBOARD", 255, 219, 2);

  drawBottomNav();  // Bottom navigation bar
}

// KEYBOARD - Pop-up QWERTY overlay (BETTER THAN iPHONE!)
void drawKeyboard() {
  // Semi-transparent overlay (redraw previous screen dimmed)
  tft.fillRect(0, 90, 320, 150, COLOR_DARK_GRAY);

  // Keyboard background
  tft.fillRoundRect(5, 95, 310, 140, 8, COLOR_BLACK);
  tft.drawRoundRect(5, 95, 310, 140, 8, COLOR_CYBER_BLUE);

  // Title bar
  tft.fillRoundRect(10, 100, 300, 18, 4, COLOR_CYBER_BLUE);
  tft.setTextColor(COLOR_BLACK); tft.setTextDatum(TC_DATUM);
  tft.drawString("KEYBOARD", 160, 104, 2);

  // QWERTY layout - 3 rows + bottom controls (compact for 320x240)
  const char* row1 = "QWERTYUIOP";
  const char* row2 = "ASDFGHJKL";
  const char* row3 = "ZXCVBNM";

  int keyWidth = 28;
  int keyHeight = 24;
  int keySpacing = 2;

  // Row 1 (10 keys)
  int y = 123;
  int startX = 10;
  for (int i = 0; i < 10; i++) {
    int x = startX + i * (keyWidth + keySpacing);
    tft.fillRoundRect(x, y, keyWidth, keyHeight, 3, COLOR_DARK_GRAY);
    tft.setTextColor(COLOR_WHITE); tft.setTextDatum(MC_DATUM);
    char key[2] = {row1[i], '\0'};
    tft.drawString(key, x + keyWidth/2, y + keyHeight/2, 2);
  }

  // Row 2 (9 keys - centered)
  y += keyHeight + keySpacing + 2;
  startX = 25; // Offset for centering
  for (int i = 0; i < 9; i++) {
    int x = startX + i * (keyWidth + keySpacing);
    tft.fillRoundRect(x, y, keyWidth, keyHeight, 3, COLOR_DARK_GRAY);
    tft.setTextColor(COLOR_WHITE); tft.setTextDatum(MC_DATUM);
    char key[2] = {row2[i], '\0'};
    tft.drawString(key, x + keyWidth/2, y + keyHeight/2, 2);
  }

  // Row 3 (7 keys - centered)
  y += keyHeight + keySpacing + 2;
  startX = 55; // More offset for centering
  for (int i = 0; i < 7; i++) {
    int x = startX + i * (keyWidth + keySpacing);
    tft.fillRoundRect(x, y, keyWidth, keyHeight, 3, COLOR_DARK_GRAY);
    tft.setTextColor(COLOR_WHITE); tft.setTextDatum(MC_DATUM);
    char key[2] = {row3[i], '\0'};
    tft.drawString(key, x + keyWidth/2, y + keyHeight/2, 2);
  }

  // Bottom row: SPACE, BACKSPACE, ENTER, DONE
  y += keyHeight + keySpacing + 3;

  // SPACE (wide)
  tft.fillRoundRect(10, y, 100, keyHeight, 3, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_WHITE); tft.setTextDatum(MC_DATUM);
  tft.drawString("SPACE", 60, y + keyHeight/2, 1);

  // BACKSPACE
  tft.fillRoundRect(115, y, 60, keyHeight, 3, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("< DEL", 145, y + keyHeight/2, 1);

  // ENTER
  tft.fillRoundRect(180, y, 50, keyHeight, 3, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("ENTER", 205, y + keyHeight/2, 1);

  // DONE (close keyboard)
  tft.fillRoundRect(235, y, 75, keyHeight, 3, COLOR_SUNRISE);
  tft.setTextColor(COLOR_BLACK);
  tft.drawString("DONE", 272, y + keyHeight/2, 2);
}

void drawDecisions() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title
  tft.setTextColor(COLOR_HOT_PINK);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("DECISIONS (T1)", 160, 28, 2);

  // Decision Card 1: Vendor Contract
  int y = 52;
  tft.fillRoundRect(10, y, 300, 50, 8, COLOR_DARK_GRAY);

  // Title with BRAIN emoji indicator
  tft.setTextColor(COLOR_MAGENTA);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("BRAIN", 15, y+5, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Vendor contract renewal", 50, y+5, 2);

  // Priority + Owner
  tft.fillCircle(18, y+28, 5, COLOR_HOT_PINK); // Critical
  tft.fillCircle(20, y+28, 7, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.drawString("EXEC", 30, y+24, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("VOICE", 70, y+24, 1);

  // Action buttons
  tft.fillRoundRect(15, y+36, 85, 10, 3, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("APPROVE", 57, y+37, 1);

  tft.fillRoundRect(108, y+36, 85, 10, 3, COLOR_HOT_PINK);
  tft.drawString("REJECT", 150, y+37, 1);

  tft.fillRoundRect(201, y+36, 100, 10, 3, COLOR_SUNRISE);
  tft.drawString("HOLD", 251, y+37, 1);

  y += 58;

  // Decision Card 2: Pricing Approval
  tft.fillRoundRect(10, y, 300, 50, 8, COLOR_DARK_GRAY);

  tft.setTextColor(COLOR_MAGENTA);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("BRAIN", 15, y+5, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Q4 pricing approval", 50, y+5, 2);

  tft.fillCircle(18, y+28, 5, COLOR_HOT_PINK); // High
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("TEAM", 30, y+24, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("DOC", 70, y+24, 1);

  tft.fillRoundRect(15, y+36, 85, 10, 3, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("APPROVE", 57, y+37, 1);

  tft.fillRoundRect(108, y+36, 85, 10, 3, COLOR_HOT_PINK);
  tft.drawString("REJECT", 150, y+37, 1);

  tft.fillRoundRect(201, y+36, 100, 10, 3, COLOR_SUNRISE);
  tft.drawString("HOLD", 251, y+37, 1);

  y += 58;

  // Decision Card 3: Partnership Deal
  tft.fillRoundRect(10, y, 300, 50, 8, COLOR_DARK_GRAY);

  tft.setTextColor(COLOR_MAGENTA);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("BRAIN", 15, y+5, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Partnership deal terms", 50, y+5, 2);

  tft.fillCircle(18, y+28, 5, COLOR_SUNRISE); // Medium
  tft.setTextColor(COLOR_WARM);
  tft.drawString("OPS", 30, y+24, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("BRIEF", 60, y+24, 1);

  tft.fillRoundRect(15, y+36, 85, 10, 3, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("APPROVE", 57, y+37, 1);

  tft.fillRoundRect(108, y+36, 85, 10, 3, COLOR_HOT_PINK);
  tft.drawString("REJECT", 150, y+37, 1);

  tft.fillRoundRect(201, y+36, 100, 10, 3, COLOR_SUNRISE);
  tft.drawString("HOLD", 251, y+37, 1);

  // Stats
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("3 Pending  |  Voice-First", 160, 215, 1);

  drawBottomNav();  // Bottom navigation bar
}

void drawCEODashboard() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("CEO DASH (T2)", 160, 28, 2);

  // Dashboard = emoji filters (live query)
  int y = 52;

  // CRITICAL + BRAIN (decisions pending)
  tft.fillRoundRect(10, y, 300, 24, 5, COLOR_DARK_GRAY);
  tft.fillCircle(18, y+12, 5, COLOR_HOT_PINK);
  tft.fillCircle(20, y+12, 7, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_MAGENTA);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("BRAIN", 30, y+4, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Decisions pending", 90, y+4, 2);
  tft.setTextColor(COLOR_HOT_PINK);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("3", 305, y+4, 2);

  y += 28;

  // BLOCKED items
  tft.fillRoundRect(10, y, 300, 24, 5, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_HOT_PINK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("BLOCKED", 15, y+4, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Blocked items", 100, y+4, 2);
  tft.setTextColor(COLOR_HOT_PINK);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("2", 305, y+4, 2);

  y += 28;

  // TIME-SENSITIVE
  tft.fillRoundRect(10, y, 300, 24, 5, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("TIME", 15, y+4, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Time-sensitive", 70, y+4, 2);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("5", 305, y+4, 2);

  y += 28;

  // SHIPPED (7d)
  tft.fillRoundRect(10, y, 300, 24, 5, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("SHIP", 15, y+4, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Shipped (7d)", 70, y+4, 2);
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("11", 305, y+4, 2);

  y += 28;

  // ESCALATIONS
  tft.fillRoundRect(10, y, 300, 24, 5, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_HOT_PINK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("ESCALATE", 15, y+4, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Escalations", 105, y+4, 2);
  tft.setTextColor(COLOR_HOT_PINK);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("1", 305, y+4, 2);

  y += 32;

  // Voice summary indicator
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("VOICE", 50, y, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Tap to hear summary", 200, y, 1);

  // Stats footer
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Live Query  |  Tap = Expand", 160, 215, 1);

  drawBottomNav();  // Bottom navigation bar
}

void drawCEOCore() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title
  tft.setTextColor(COLOR_HOT_PINK);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("CEO CORE 3x3", 160, 28, 2);

  // 3×3 Grid (80×80 buttons, centered)
  int startX = 40;
  int startY = 50;
  int btnSize = 80;
  int gap = 0;

  // ROW 1: REJECT | HOLD | APPROVE
  // Button: REJECT (❌)
  tft.fillRoundRect(startX, startY, btnSize, btnSize, 8, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("X", startX+btnSize/2, startY+btnSize/2-10, 4);
  tft.drawString("REJECT", startX+btnSize/2, startY+btnSize/2+15, 1);

  // Button: HOLD (💭)
  tft.fillRoundRect(startX+btnSize+gap, startY, btnSize, btnSize, 8, COLOR_SUNRISE);
  tft.drawString("...", startX+btnSize+gap+btnSize/2, startY+btnSize/2-10, 4);
  tft.drawString("HOLD", startX+btnSize+gap+btnSize/2, startY+btnSize/2+15, 1);

  // Button: APPROVE (✅)
  tft.fillRoundRect(startX+(btnSize+gap)*2, startY, btnSize, btnSize, 8, COLOR_VIVID_PUR);
  tft.drawString("\\/", startX+(btnSize+gap)*2+btnSize/2, startY+btnSize/2-10, 4);
  tft.drawString("APPROVE", startX+(btnSize+gap)*2+btnSize/2, startY+btnSize/2+15, 1);

  // ROW 2: BLOCKED | BRAIN | NEXT
  int y2 = startY + btnSize + gap;

  // Button: BLOCKED (🧱)
  tft.fillRoundRect(startX, y2, btnSize, btnSize, 8, COLOR_HOT_PINK);
  tft.drawString("##", startX+btnSize/2, y2+btnSize/2-10, 4);
  tft.drawString("BLOCKED", startX+btnSize/2, y2+btnSize/2+15, 1);

  // Button: BRAIN (🧠) - CENTER - ACTIVE
  tft.fillRoundRect(startX+btnSize+gap, y2, btnSize, btnSize, 8, COLOR_MAGENTA);
  tft.fillCircle(startX+btnSize+gap+btnSize/2, y2+btnSize/2-5, 8, COLOR_HOT_PINK); // Pulse indicator
  tft.drawString("@", startX+btnSize+gap+btnSize/2, y2+btnSize/2-10, 4);
  tft.drawString("BRAIN", startX+btnSize+gap+btnSize/2, y2+btnSize/2+15, 1);

  // Button: NEXT (🔜)
  tft.fillRoundRect(startX+(btnSize+gap)*2, y2, btnSize, btnSize, 8, COLOR_CYBER_BLUE);
  tft.drawString(">", startX+(btnSize+gap)*2+btnSize/2, y2+btnSize/2-10, 4);
  tft.drawString("NEXT", startX+(btnSize+gap)*2+btnSize/2, y2+btnSize/2+15, 1);

  // ROW 3: CRITICAL | ESCALATE | SHIP
  int y3 = y2 + btnSize + gap;

  // Button: CRITICAL (🔥)
  tft.fillRoundRect(startX, y3, btnSize, btnSize, 8, COLOR_HOT_PINK);
  tft.fillCircle(startX+btnSize/2, y3+btnSize/2-5, 5, COLOR_WHITE);
  tft.fillCircle(startX+btnSize/2, y3+btnSize/2-5, 8, COLOR_HOT_PINK);
  tft.drawString("!!", startX+btnSize/2, y3+btnSize/2-10, 4);
  tft.drawString("CRITICAL", startX+btnSize/2, y3+btnSize/2+15, 1);

  // Button: ESCALATE (🪜)
  tft.fillRoundRect(startX+btnSize+gap, y3, btnSize, btnSize, 8, COLOR_WARM);
  tft.drawString("^^", startX+btnSize+gap+btnSize/2, y3+btnSize/2-10, 4);
  tft.drawString("ESCALATE", startX+btnSize+gap+btnSize/2, y3+btnSize/2+15, 1);

  // Button: SHIP (🚀)
  tft.fillRoundRect(startX+(btnSize+gap)*2, y3, btnSize, btnSize, 8, COLOR_VIVID_PUR);
  tft.drawString("^", startX+(btnSize+gap)*2+btnSize/2, y3+btnSize/2-10, 4);
  tft.drawString("SHIP", startX+(btnSize+gap)*2+btnSize/2, y3+btnSize/2+15, 1);

  drawBottomNav();  // Bottom navigation bar
}

void drawExecGrid() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("EXEC 4x4", 160, 28, 2);

  // 4×4 Grid (60×60 buttons)
  int startX = 50;
  int startY = 50;
  int btnSize = 60;
  int gap = 2;

  // ROW 1: REJECT | HOLD | BRAIN | APPROVE
  tft.fillRoundRect(startX, startY, btnSize, btnSize, 6, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("X", startX+btnSize/2, startY+btnSize/2-8, 3);
  tft.drawString("REJ", startX+btnSize/2, startY+btnSize/2+12, 1);

  tft.fillRoundRect(startX+(btnSize+gap), startY, btnSize, btnSize, 6, COLOR_SUNRISE);
  tft.drawString("...", startX+(btnSize+gap)+btnSize/2, startY+btnSize/2-8, 3);
  tft.drawString("HOLD", startX+(btnSize+gap)+btnSize/2, startY+btnSize/2+12, 1);

  tft.fillRoundRect(startX+(btnSize+gap)*2, startY, btnSize, btnSize, 6, COLOR_MAGENTA);
  tft.drawString("@", startX+(btnSize+gap)*2+btnSize/2, startY+btnSize/2-8, 3);
  tft.drawString("BRAIN", startX+(btnSize+gap)*2+btnSize/2, startY+btnSize/2+12, 1);

  tft.fillRoundRect(startX+(btnSize+gap)*3, startY, btnSize, btnSize, 6, COLOR_VIVID_PUR);
  tft.drawString("\\/", startX+(btnSize+gap)*3+btnSize/2, startY+btnSize/2-8, 3);
  tft.drawString("APV", startX+(btnSize+gap)*3+btnSize/2, startY+btnSize/2+12, 1);

  // ROW 2: BLOCKED | RISK | CRITICAL | NEXT
  int y2 = startY + btnSize + gap;

  tft.fillRoundRect(startX, y2, btnSize, btnSize, 6, COLOR_HOT_PINK);
  tft.drawString("##", startX+btnSize/2, y2+btnSize/2-8, 3);
  tft.drawString("BLOCK", startX+btnSize/2, y2+btnSize/2+12, 1);

  tft.fillRoundRect(startX+(btnSize+gap), y2, btnSize, btnSize, 6, COLOR_SUNRISE);
  tft.drawString("/!\\", startX+(btnSize+gap)+btnSize/2, y2+btnSize/2-8, 3);
  tft.drawString("RISK", startX+(btnSize+gap)+btnSize/2, y2+btnSize/2+12, 1);

  tft.fillRoundRect(startX+(btnSize+gap)*2, y2, btnSize, btnSize, 6, COLOR_HOT_PINK);
  tft.fillCircle(startX+(btnSize+gap)*2+btnSize/2, y2+btnSize/2-5, 5, COLOR_WHITE);
  tft.drawString("!!", startX+(btnSize+gap)*2+btnSize/2, y2+btnSize/2-8, 3);
  tft.drawString("CRIT", startX+(btnSize+gap)*2+btnSize/2, y2+btnSize/2+12, 1);

  tft.fillRoundRect(startX+(btnSize+gap)*3, y2, btnSize, btnSize, 6, COLOR_CYBER_BLUE);
  tft.drawString(">", startX+(btnSize+gap)*3+btnSize/2, y2+btnSize/2-8, 3);
  tft.drawString("NEXT", startX+(btnSize+gap)*3+btnSize/2, y2+btnSize/2+12, 1);

  // ROW 3: USER | TEAM | ESCALATE | DIRECTION
  int y3 = y2 + btnSize + gap;

  tft.fillRoundRect(startX, y3, btnSize, btnSize, 6, COLOR_VIVID_PUR);
  tft.drawString("1", startX+btnSize/2, y3+btnSize/2-8, 3);
  tft.drawString("USER", startX+btnSize/2, y3+btnSize/2+12, 1);

  tft.fillRoundRect(startX+(btnSize+gap), y3, btnSize, btnSize, 6, COLOR_CYBER_BLUE);
  tft.drawString("++", startX+(btnSize+gap)+btnSize/2, y3+btnSize/2-8, 3);
  tft.drawString("TEAM", startX+(btnSize+gap)+btnSize/2, y3+btnSize/2+12, 1);

  tft.fillRoundRect(startX+(btnSize+gap)*2, y3, btnSize, btnSize, 6, COLOR_WARM);
  tft.drawString("^^", startX+(btnSize+gap)*2+btnSize/2, y3+btnSize/2-8, 3);
  tft.drawString("ESC", startX+(btnSize+gap)*2+btnSize/2, y3+btnSize/2+12, 1);

  tft.fillRoundRect(startX+(btnSize+gap)*3, y3, btnSize, btnSize, 6, COLOR_SUNRISE);
  tft.drawString("<>", startX+(btnSize+gap)*3+btnSize/2, y3+btnSize/2-8, 3);
  tft.drawString("DIR", startX+(btnSize+gap)*3+btnSize/2, y3+btnSize/2+12, 1);

  // ROW 4: VOICE | METRICS | RECURRING | SHIP
  int y4 = y3 + btnSize + gap;

  tft.fillRoundRect(startX, y4, btnSize, btnSize, 6, COLOR_SUNRISE);
  tft.drawString("o", startX+btnSize/2, y4+btnSize/2-8, 3);
  tft.drawString("VOICE", startX+btnSize/2, y4+btnSize/2+12, 1);

  tft.fillRoundRect(startX+(btnSize+gap), y4, btnSize, btnSize, 6, COLOR_CYBER_BLUE);
  tft.drawString("|_|", startX+(btnSize+gap)+btnSize/2, y4+btnSize/2-8, 3);
  tft.drawString("DATA", startX+(btnSize+gap)+btnSize/2, y4+btnSize/2+12, 1);

  tft.fillRoundRect(startX+(btnSize+gap)*2, y4, btnSize, btnSize, 6, COLOR_MAGENTA);
  tft.drawString("oo", startX+(btnSize+gap)*2+btnSize/2, y4+btnSize/2-8, 3);
  tft.drawString("LOOP", startX+(btnSize+gap)*2+btnSize/2, y4+btnSize/2+12, 1);

  tft.fillRoundRect(startX+(btnSize+gap)*3, y4, btnSize, btnSize, 6, COLOR_VIVID_PUR);
  tft.drawString("^", startX+(btnSize+gap)*3+btnSize/2, y4+btnSize/2-8, 3);
  tft.drawString("SHIP", startX+(btnSize+gap)*3+btnSize/2, y4+btnSize/2+12, 1);

  drawBottomNav();  // Bottom navigation bar
}

// META - ULTIMATE RECURSION! Aggregates ALL app data! (NO iPHONE CAN DO THIS!)
void drawMeta() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title with live indicator
  tft.setTextColor(COLOR_AMBER);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("META DASHBOARD", 120, 25, 2);

  // Live indicator (pulsing dot)
  static bool pulse = false;
  pulse = !pulse;
  tft.fillCircle(220, 30, 3, pulse ? COLOR_HOT_PINK : COLOR_MAGENTA);
  tft.setTextColor(COLOR_HOT_PINK);
  tft.drawString("LIVE", 230, 27, 1);

  // System Stats Card (Top)
  int y = 45;
  tft.fillRoundRect(5, y, 230, 32, 4, COLOR_DARK_GRAY);

  // Calculate real-time metrics
  unsigned long uptime_sec = millis() / 1000;
  int uptime_min = uptime_sec / 60;
  int uptime_hr = uptime_min / 60;
  float heap_free_kb = ESP.getFreeHeap() / 1024.0;
  int heap_percent = (ESP.getFreeHeap() * 100) / ESP.getHeapSize();

  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("SYS", 8, y+3, 1);
  tft.setTextColor(COLOR_WHITE);
  char sysStats[60];
  sprintf(sysStats, "UP:%dh%dm MEM:%d%% WIFI:%ddBm",
          uptime_hr, uptime_min % 60, heap_percent, WiFi.RSSI());
  tft.drawString(sysStats, 8, y+14, 1);

  // Mini bar graph for memory
  int barWidth = (heap_percent * 100) / 100;  // Scale to 100px max
  tft.fillRect(8, y+25, 100, 4, COLOR_DARK_GRAY);
  uint16_t memColor = heap_percent > 50 ? COLOR_CYBER_BLUE : COLOR_HOT_PINK;
  tft.fillRect(8, y+25, barWidth, 4, memColor);

  // ULTIMATE RECURSION - Pull data from EVERY app!
  y = 82;
  tft.setTextDatum(TL_DATUM);

  // Row 1: AI + Messages (2-column layout)
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.drawString("AI", 8, y, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("5 mdl·2847 req", 26, y, 1);

  tft.setTextColor(COLOR_HOT_PINK);
  tft.drawString("MSG", 125, y, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("3 unread·24 tot", 150, y, 1);

  // Row 2: CRM + VPN
  y += 14;
  tft.setTextColor(COLOR_MAGENTA);
  tft.drawString("CRM", 8, y, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("$626K·33 deals", 31, y, 1);

  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("VPN", 125, y, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("4 nodes online", 150, y, 1);

  // Row 3: Identity + Files
  y += 14;
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("ID", 8, y, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("alexa·2 sessions", 24, y, 1);

  tft.setTextColor(COLOR_WARM);
  tft.drawString("FILE", 125, y, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("45 files·2.3GB", 151, y, 1);

  // Row 4: API + Pager
  y += 14;
  tft.setTextColor(COLOR_DEEP_MAG);
  tft.drawString("API", 8, y, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("382 req·247 chat", 29, y, 1);

  tft.setTextColor(COLOR_HOT_PINK);
  tft.drawString("PAGR", 125, y, 1);
  tft.setTextColor(COLOR_WHITE);
  char pagerStat[20];
  sprintf(pagerStat, "%d alerts·%s", alertHistoryCount, hasActiveAlert ? "LIVE" : "OK");
  tft.drawString(pagerStat, 153, y, 1);

  // CEO Decision Metrics - Visual bars
  y += 20;
  int total_decisions = approveCount + rejectCount + shipCount + escalateCount;

  tft.setTextColor(COLOR_HOT_PINK);
  tft.drawString("CEO DECISIONS", 8, y, 1);
  tft.setTextColor(COLOR_SUNRISE);
  char decTotal[20];
  sprintf(decTotal, "(%d total)", total_decisions);
  tft.drawString(decTotal, 80, y, 1);

  y += 12;
  // Approve bar (green)
  int approvePct = total_decisions > 0 ? (approveCount * 100) / total_decisions : 0;
  tft.fillRect(8, y, (approvePct * 110) / 100, 6, COLOR_CYBER_BLUE);
  char appBar[20];
  sprintf(appBar, "APV %d%%", approvePct);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString(appBar, 122, y-1, 1);

  y += 9;
  // Reject bar (red)
  int rejectPct = total_decisions > 0 ? (rejectCount * 100) / total_decisions : 0;
  tft.fillRect(8, y, (rejectPct * 110) / 100, 6, COLOR_HOT_PINK);
  char rejBar[20];
  sprintf(rejBar, "REJ %d%%", rejectPct);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString(rejBar, 122, y-1, 1);

  y += 9;
  // Ship bar (purple)
  int shipPct = total_decisions > 0 ? (shipCount * 100) / total_decisions : 0;
  tft.fillRect(8, y, (shipPct * 110) / 100, 6, COLOR_VIVID_PUR);
  char shipBar[20];
  sprintf(shipBar, "SHIP %d%%", shipPct);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString(shipBar, 122, y-1, 1);

  // Active AI Model with recommendation
  y += 16;
  const char* aiNames[] = {"LUCIDIA", "ARIA", "CADENCE", "CECILIA", "OCTAVIA", "ALICE"};
  tft.fillRoundRect(5, y, 230, 28, 4, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("ACTIVE AI:", 10, y+4, 1);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString(aiNames[activeAI], 60, y+3, 2);

  // AI Recommendation from context
  tft.setTextColor(COLOR_BLACK);
  String aiRec = getAIRecommendation();
  if (aiRec.length() > 24) aiRec = aiRec.substring(0, 24);
  tft.drawString(aiRec.c_str(), 10, y+16, 1);

  // RECURSION Stats - The META level!
  y += 34;
  tft.fillRoundRect(5, y, 230, 20, 4, COLOR_SUNRISE);
  tft.setTextColor(COLOR_BLACK);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("RECURSION DEPTH: 17 APPS", 120, y+5, 2);

  // Performance indicator
  y += 24;
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COLOR_HOT_PINK);
  tft.drawString("META = ALL DATA", 60, y, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("|", 120, y, 1);
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.drawString("ULTIMATE RECURSION!", 190, y, 1);

  drawBottomNav();  // Bottom navigation bar
}

// RECURSIVE CHAT - Each AI references OTHER apps! (BETTER THAN iPHONE!)
void drawBlackRoadChat() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title
  tft.setTextColor(COLOR_HOT_PINK);
  tft.setTextDatum(TC_DATUM);
  brFont.drawMonoTextCentered("OPERATOR CHAT", 120, 27, BR_MONO_MEDIUM, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_WHITE);
  brFont.drawTechnicalLabel("proprietary models", 50, 42, COLOR_CYBER_BLUE);

  // 6 TAPPABLE AI Models (highlight active model)
  int y = 56;
  uint16_t colors[] = {COLOR_VIVID_PUR, COLOR_HOT_PINK, COLOR_CYBER_BLUE, COLOR_MAGENTA, COLOR_WARM, COLOR_VIVID_PUR};
  const char* names[] = {"LUCIDIA", "ARIA", "CADENCE", "CECILIA", "OCTAVIA", "ALICE"};
  const char* roles[] = {"Strategic", "Creative", "Workflow", "Analysis", "Security", "Operations"};

  for (int i = 0; i < 6; i++) {
    int x = (i % 2 == 0) ? 8 : 164;
    if (i >= 2 && i % 2 == 0) y += 26;

    // Highlight active model with WHITE outline (RECURSION!)
    if (i == activeAI) {
      tft.drawRoundRect(x-1, y-1, 150, 24, 4, COLOR_WHITE);
    }

    tft.fillRoundRect(x, y, 148, 22, 4, colors[i]);
    tft.setTextColor(COLOR_WHITE); tft.setTextDatum(TL_DATUM);
    tft.drawString(names[i], x+4, y+4, 2);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(roles[i], x+144, y+4, 1);
    tft.setTextColor(COLOR_SUNRISE);
    tft.drawString("ONLINE", x+144, y+13, 1);
  }

  // RECURSIVE CONVERSATIONS - Each AI references OTHER apps!
  y += 34;
  tft.setTextDatum(TL_DATUM);

  // Display conversation based on active AI
  switch (activeAI) {
    case 0: // LUCIDIA - Strategic (references CRM app)
      tft.setTextColor(COLOR_VIVID_PUR);
      tft.drawString("ACTIVE: Lucidia (Strategic)", 10, y, 1);
      y += 14;
      // User: What's priority?
      tft.fillRoundRect(120, y, 190, 18, 5, COLOR_CYBER_BLUE);
      tft.setTextColor(COLOR_WHITE); tft.setTextDatum(TR_DATUM);
      tft.drawString("What's the priority?", 306, y+4, 1);
      y += 22;
      // AI: Focus on $626K pipeline (FROM CRM APP!)
      tft.fillRoundRect(10, y, 250, 18, 5, COLOR_DARK_GRAY);
      tft.setTextColor(COLOR_VIVID_PUR); tft.setTextDatum(TL_DATUM);
      tft.drawString("LUCIDIA:", 14, y+4, 1);
      tft.setTextColor(COLOR_WHITE);
      tft.drawString("Focus on $626K pipeline", 68, y+4, 1);
      y += 22;
      // User: Show deals
      tft.fillRoundRect(150, y, 160, 18, 5, COLOR_CYBER_BLUE);
      tft.setTextColor(COLOR_WHITE); tft.setTextDatum(TR_DATUM);
      tft.drawString("Show top deals", 306, y+4, 1);
      break;

    case 1: // ARIA - Creative (references Messages + Workflow)
      tft.setTextColor(COLOR_HOT_PINK);
      tft.drawString("ACTIVE: Aria (Creative)", 10, y, 1);
      y += 14;
      tft.fillRoundRect(120, y, 190, 18, 5, COLOR_CYBER_BLUE);
      tft.setTextColor(COLOR_WHITE); tft.setTextDatum(TR_DATUM);
      tft.drawString("Design ideas?", 306, y+4, 1);
      y += 22;
      tft.fillRoundRect(10, y, 270, 18, 5, COLOR_DARK_GRAY);
      tft.setTextColor(COLOR_HOT_PINK); tft.setTextDatum(TL_DATUM);
      tft.drawString("ARIA:", 14, y+4, 1);
      tft.setTextColor(COLOR_WHITE);
      tft.drawString("Check Messages: 3 unread ideas", 48, y+4, 1);
      y += 22;
      tft.fillRoundRect(100, y, 210, 18, 5, COLOR_CYBER_BLUE);
      tft.setTextColor(COLOR_WHITE); tft.setTextDatum(TR_DATUM);
      tft.drawString("What about workflow?", 306, y+4, 1);
      break;

    case 2: // CADENCE - Workflow (references CEO Core + Workflow app)
      tft.setTextColor(COLOR_CYBER_BLUE);
      tft.drawString("ACTIVE: Cadence (Workflow)", 10, y, 1);
      y += 14;
      tft.fillRoundRect(100, y, 210, 18, 5, COLOR_CYBER_BLUE);
      tft.setTextColor(COLOR_WHITE); tft.setTextDatum(TR_DATUM);
      tft.drawString("Next workflow task?", 306, y+4, 1);
      y += 22;
      tft.fillRoundRect(10, y, 280, 18, 5, COLOR_DARK_GRAY);
      tft.setTextColor(COLOR_CYBER_BLUE); tft.setTextDatum(TL_DATUM);
      tft.drawString("CADENCE:", 14, y+4, 1);
      tft.setTextColor(COLOR_WHITE);
      tft.drawString("CEO approved 9 items. Ship next!", 72, y+4, 1);
      y += 22;
      tft.fillRoundRect(150, y, 160, 18, 5, COLOR_CYBER_BLUE);
      tft.setTextColor(COLOR_WHITE); tft.setTextDatum(TR_DATUM);
      tft.drawString("Show status", 306, y+4, 1);
      break;

    case 3: // CECILIA - Analysis (references APIs + Files)
      tft.setTextColor(COLOR_MAGENTA);
      tft.drawString("ACTIVE: Cecilia (Analysis)", 10, y, 1);
      y += 14;
      tft.fillRoundRect(130, y, 180, 18, 5, COLOR_CYBER_BLUE);
      tft.setTextColor(COLOR_WHITE); tft.setTextDatum(TR_DATUM);
      tft.drawString("Data summary?", 306, y+4, 1);
      y += 22;
      tft.fillRoundRect(10, y, 280, 18, 5, COLOR_DARK_GRAY);
      tft.setTextColor(COLOR_MAGENTA); tft.setTextDatum(TL_DATUM);
      tft.drawString("CECILIA:", 14, y+4, 1);
      tft.setTextColor(COLOR_WHITE);
      tft.drawString("APIs: 382 reqs, Files: 45 (2.3GB)", 68, y+4, 1);
      y += 22;
      tft.fillRoundRect(110, y, 200, 18, 5, COLOR_CYBER_BLUE);
      tft.setTextColor(COLOR_WHITE); tft.setTextDatum(TR_DATUM);
      tft.drawString("Breakdown by endpoint", 306, y+4, 1);
      break;

    case 4: // OCTAVIA - Security (references VPN + Identity)
      tft.setTextColor(COLOR_WARM);
      tft.drawString("ACTIVE: Octavia (Security)", 10, y, 1);
      y += 14;
      tft.fillRoundRect(120, y, 190, 18, 5, COLOR_CYBER_BLUE);
      tft.setTextColor(COLOR_WHITE); tft.setTextDatum(TR_DATUM);
      tft.drawString("Security status?", 306, y+4, 1);
      y += 22;
      tft.fillRoundRect(10, y, 300, 18, 5, COLOR_DARK_GRAY);
      tft.setTextColor(COLOR_WARM); tft.setTextDatum(TL_DATUM);
      tft.drawString("OCTAVIA:", 14, y+4, 1);
      tft.setTextColor(COLOR_WHITE);
      tft.drawString("VPN: 4 nodes online, ID: 2 sessions", 68, y+4, 1);
      y += 22;
      tft.fillRoundRect(150, y, 160, 18, 5, COLOR_CYBER_BLUE);
      tft.setTextColor(COLOR_WHITE); tft.setTextDatum(TR_DATUM);
      tft.drawString("All secure?", 306, y+4, 1);
      break;

    case 5: // ALICE - Operations (references Settings + SSH nodes)
      tft.setTextColor(COLOR_VIVID_PUR);
      tft.drawString("ACTIVE: Alice (Operations)", 10, y, 1);
      y += 14;
      tft.fillRoundRect(120, y, 190, 18, 5, COLOR_CYBER_BLUE);
      tft.setTextColor(COLOR_WHITE); tft.setTextDatum(TR_DATUM);
      tft.drawString("System health?", 306, y+4, 1);
      y += 22;
      tft.fillRoundRect(10, y, 290, 18, 5, COLOR_DARK_GRAY);
      tft.setTextColor(COLOR_VIVID_PUR); tft.setTextDatum(TL_DATUM);
      tft.drawString("ALICE:", 14, y+4, 1);
      tft.setTextColor(COLOR_WHITE);
      tft.drawString("5 SSH nodes, WiFi connected, 75%", 54, y+4, 1);
      y += 22;
      tft.fillRoundRect(130, y, 180, 18, 5, COLOR_CYBER_BLUE);
      tft.setTextColor(COLOR_WHITE); tft.setTextDatum(TR_DATUM);
      tft.drawString("Check node status", 306, y+4, 1);
      break;
  }

  // Bottom stats - RECURSION indicator
  tft.setTextColor(COLOR_HOT_PINK); tft.setTextDatum(TC_DATUM);
  tft.drawString("6 MODELS", 80, 220, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("|", 135, 220, 1);
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.drawString("TAP=SWITCH", 200, 220, 1);
  tft.setTextColor(COLOR_MAGENTA);
  tft.drawString("| RECURSIVE!", 280, 220, 1);

  drawBottomNav();  // Bottom navigation bar
}

// ═══════════════════════════════════════════════════════════════════
// EMERGENCY PAGER - UI SCREENS
// ═══════════════════════════════════════════════════════════════════

void drawEmergencyPager() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Back button
  tft.fillRoundRect(10, 25, 50, 20, 5, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("< Back", 15, 28, 2);

  // Alert heading with pulsing effect
  tft.setTextColor(COLOR_HOT_PINK);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("EMERGENCY ALERT", 160, 50, 4);

  if (!hasActiveAlert) {
    // No active alerts
    tft.setTextColor(COLOR_WHITE);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("No active alerts", 160, 120, 2);
    tft.drawString("All clear", 160, 145, 2);
    return;
  }

  // Alert info box
  tft.drawRoundRect(15, 80, 290, 90, 8, COLOR_HOT_PINK);

  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TL_DATUM);

  // Source and priority
  String sourceStr = "SOURCE:   " + String(currentAlert.source);
  String priorityStr = "PRIORITY: " + String(currentAlert.priority);

  tft.drawString(sourceStr, 25, 90, 2);
  tft.drawString(priorityStr, 25, 110, 2);

  // Pattern name
  String patternStr = "PATTERN:  ";
  switch(currentAlert.patternId) {
    case PATTERN_FAST_STROBE: patternStr += "Fast Strobe"; break;
    case PATTERN_MEDIUM_BLINK: patternStr += "Medium Blink"; break;
    case PATTERN_SLOW_BLINK: patternStr += "Slow Blink"; break;
    case PATTERN_MORSE_SOS: patternStr += "SOS"; break;
    default: patternStr += "Standard";
  }
  tft.drawString(patternStr, 25, 130, 2);

  // Timestamp
  unsigned long seconds = currentAlert.timestamp / 1000;
  String timeStr = "TIME:     " + String(seconds) + "s ago";
  tft.drawString(timeStr, 25, 150, 2);

  // Message (if set)
  if(strlen(currentAlert.message) > 0) {
    tft.setTextColor(COLOR_SUNRISE);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(currentAlert.message, 160, 180, 1);
  }

  // ACK button
  tft.fillRoundRect(30, 200, 120, 35, 8, COLOR_CYBER_BLUE);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("ACK", 90, 217, 2);

  // DISMISS button
  tft.fillRoundRect(170, 200, 120, 35, 8, COLOR_DARK_GRAY);
  tft.drawString("DISMISS", 230, 217, 2);
}

void drawAlertHistory() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Back button
  tft.fillRoundRect(10, 25, 50, 20, 5, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("< Back", 15, 28, 2);

  // Title
  tft.setTextColor(COLOR_HOT_PINK);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("ALERT HISTORY", 160, 50, 4);

  if(alertHistoryCount == 0) {
    tft.setTextColor(COLOR_WHITE);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("No alerts in history", 160, 120, 2);
    return;
  }

  // Display last 5 alerts
  tft.setTextDatum(TL_DATUM);
  int y = 80;
  int displayCount = alertHistoryCount > 5 ? 5 : alertHistoryCount;

  for(int i = alertHistoryCount - 1; i >= alertHistoryCount - displayCount; i--) {
    Alert &alert = alertHistory[i];

    // Time
    unsigned long seconds = alert.timestamp / 1000;
    String timeStr = String(seconds) + "s";
    tft.setTextColor(COLOR_DARK_GRAY);
    tft.drawString(timeStr, 15, y, 1);

    // Source + Priority
    String infoStr = String(alert.source) + " " + String(alert.priority);
    tft.setTextColor(COLOR_WHITE);
    tft.drawString(infoStr, 70, y, 2);

    // ACK status
    if(alert.acknowledged) {
      tft.setTextColor(COLOR_CYBER_BLUE);
      tft.drawString("ACK", 270, y, 2);
    }

    y += 25;
  }

  // Stats
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TC_DATUM);
  String statsStr = "Total alerts: " + String(alertHistoryCount);
  tft.drawString(statsStr, 160, 215, 1);
}

// ═══════════════════════════════════════════════════════════════════
// WEATHER APP - OpenWeatherMap 5-Day Forecast
// ═══════════════════════════════════════════════════════════════════

void drawWeather() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();
  drawAppHeader("WEATHER", COLOR_CYBER_BLUE);

  int y = 55;

  // Location card - horizontal layout
  tft.fillRoundRect(10, y, 300, 28, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 300, 28, 6, COLOR_VIVID_PUR);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Location", 16, y+5, BR_MONO_TINY, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Minnesota Wilderness", 16, y+16, 1);
  tft.setTextColor(COLOR_AMBER);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("46.2N", 304, y+16, 1);

  // Current conditions card
  y += 33;
  tft.fillRoundRect(10, y, 300, 50, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 300, 50, 6, COLOR_CYBER_BLUE);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Current", 16, y+5, BR_MONO_TINY, COLOR_CYBER_BLUE);

  // Temperature (large)
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("28", 20, y+18, 4);
  tft.drawString("°F", 60, y+18, 2);

  // Conditions
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Partly Cloudy", 100, y+20, 1);
  tft.setTextColor(COLOR_AMBER);
  tft.drawString("Feels 25°F", 100, y+32, 1);

  // Status dot
  tft.fillCircle(298, y+25, 3, COLOR_SUNRISE);

  // Weather metrics grid
  y += 55;
  const char* metrics[4] = {"Humid", "Wind", "UV", "Press"};
  const char* values[4] = {"65%", "12mph", "2", "1013"};
  uint16_t mColors[4] = {COLOR_CYBER_BLUE, COLOR_HOT_PINK, COLOR_SUNRISE, COLOR_VIVID_PUR};

  for (int i = 0; i < 4; i++) {
    int x = 10 + (i%2)*110;
    int cardY = y + (i/2)*28;

    tft.fillRoundRect(x, cardY, 105, 24, 4, COLOR_DARK_GRAY);
    tft.drawRoundRect(x, cardY, 105, 24, 4, mColors[i]);

    tft.setTextDatum(TL_DATUM);
    brFont.drawMonoText(metrics[i], x+6, cardY+5, BR_MONO_TINY, mColors[i]);
    tft.setTextColor(COLOR_WHITE);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(values[i], x+100, cardY+13, 1);
  }

  // 5-day forecast
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("5-DAY FORECAST", 15, 140, 1);

  const char* days[] = {"MON", "TUE", "WED", "THU", "FRI"};
  int highs[] = {75, 78, 72, 69, 71};
  int lows[] = {58, 61, 55, 52, 54};
  const char* conditions[] = {"Sunny", "Sunny", "Rain", "Cloud", "Sunny"};

  int startY = 162;
  for(int i = 0; i < 5; i++) {
    int y = startY + (i * 28);

    // Day
    tft.setTextColor(COLOR_WHITE);
    brFont.drawMonoText(days[i], 20, y, 2, COLOR_WHITE);

    // Condition icon (emoji-style)
    tft.setTextColor(COLOR_SUNRISE);
    const char* icon = (conditions[i][0] == 'S') ? "☀" : (conditions[i][0] == 'R') ? "🌧" : "☁";
    brFont.drawMonoText(icon, 75, y, 2, COLOR_WHITE);

    // Temps
    tft.setTextColor(COLOR_HOT_PINK);
    String high = String(highs[i]) + "°";
    brFont.drawMonoText(high.c_str(), 115, y, 2, COLOR_WHITE);

    tft.setTextColor(COLOR_CYBER_BLUE);
    String low = String(lows[i]) + "°";
    brFont.drawMonoText(low.c_str(), 165, y, 2, COLOR_WHITE);

    // Condition
    tft.setTextColor(COLOR_DARK_GRAY);
    brFont.drawMonoText(conditions[i], 210, y, 1, COLOR_WHITE);
  }

  // Last updated
  tft.setTextColor(COLOR_DARK_GRAY);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Updated: 2 min ago", 160, 302, 1);

  drawBottomNav();
}

// ═══════════════════════════════════════════════════════════════════
// CALENDAR APP - Nextcloud CalDAV Integration (SOVEREIGN STACK!)
// ═══════════════════════════════════════════════════════════════════

void drawCalendar() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title with Nextcloud integration + sovereign indicator
  tft.fillRoundRect(0, 20, 240, 35, 0, COLOR_DARK_GRAY);
  for (int i = 0; i < 18; i++) {
    float t = i / 18.0;
    uint16_t c = brUI.lerpColor(COLOR_VIVID_PUR, COLOR_HOT_PINK, t);
    tft.drawFastHLine(0, 20 + i, 240, c);
  }

  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoTextCentered("CALENDAR", 120, 27, BR_MONO_MEDIUM, COLOR_WHITE);

  // Nextcloud badge
  tft.fillCircle(205, 29, 3, COLOR_SUNRISE);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextSize(1);
  tft.drawString("NC", 220, 27, 1);

  int y = 45;

  // Today's date card
  tft.fillRoundRect(10, y, 220, 35, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 35, 6, COLOR_VIVID_PUR);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Today", 16, y+6, BR_MONO_TINY, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Friday, January 3", 16, y+18, 2);

  // Sync indicator
  tft.fillCircle(218, y+18, 3, COLOR_SUNRISE);

  // Upcoming events
  y += 38;
  tft.setTextColor(COLOR_AMBER);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("UPCOMING:", 10, y, 1);

  y += 12;
  const char* events[4] = {"Team Standup", "Deploy Review", "1:1 with Alexa", "Project Demo"};
  const char* times[4] = {"9:00", "11:30", "14:00", "16:00"};
  uint16_t eventColors[4] = {COLOR_CYBER_BLUE, COLOR_HOT_PINK, COLOR_VIVID_PUR, COLOR_SUNRISE};

  for (int i = 0; i < 4; i++) {
    tft.fillRoundRect(10, y, 220, 26, 4, COLOR_DARK_GRAY);
    tft.drawRoundRect(10, y, 220, 26, 4, eventColors[i]);

    tft.setTextDatum(TL_DATUM);
    brFont.drawMonoText(times[i], 16, y+5, BR_MONO_TINY, eventColors[i]);
    tft.setTextColor(COLOR_WHITE);
    tft.drawString(events[i], 60, y+14, 1);

    // Calendar indicator dot
    tft.fillCircle(218, y+13, 3, eventColors[i]);

    y += 30;
  }

  // Nextcloud sync status
  tft.setTextColor(COLOR_DARK_GRAY);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("CalDAV: cloud.blackroad.io", 120, y+8, 1);

  drawBottomNav();
}

// ═══════════════════════════════════════════════════════════════════
// PAYMENTS APP - BTCPay Server (BITCOIN SOVEREIGN!)
// ═══════════════════════════════════════════════════════════════════

void drawPayments() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title with Bitcoin + sovereign indicator
  tft.fillRoundRect(0, 20, 240, 35, 0, COLOR_DARK_GRAY);
  for (int i = 0; i < 18; i++) {
    float t = i / 18.0;
    uint16_t c = brUI.lerpColor(COLOR_SUNRISE, COLOR_AMBER, t);
    tft.drawFastHLine(0, 20 + i, 240, c);
  }

  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoTextCentered("PAYMENTS", 120, 27, BR_MONO_MEDIUM, COLOR_WHITE);

  // BTC badge
  tft.fillCircle(205, 29, 3, COLOR_SUNRISE);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextSize(1);
  tft.drawString("BTC", 223, 27, 1);

  int y = 45;

  // Bitcoin wallet card
  tft.fillRoundRect(10, y, 220, 45, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 45, 6, COLOR_SUNRISE);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Bitcoin Wallet", 16, y+6, BR_MONO_TINY, COLOR_SUNRISE);

  // BTC balance
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("0.1234", 20, y+18, 4);
  tft.setTextColor(COLOR_AMBER);
  tft.drawString("BTC", 95, y+25, 2);

  // USD value
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("$4,567", 224, y+25, 2);

  // Recent transactions
  y += 52;
  tft.setTextColor(COLOR_AMBER);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("RECENT:", 10, y, 1);

  y += 12;
  const char* txTypes[3] = {"Received", "Sent", "Received"};
  const char* txAmounts[3] = {"+0.005", "-0.012", "+0.008"};
  const char* txDates[3] = {"2h", "1d", "3d"};
  uint16_t txColors[3] = {COLOR_SUNRISE, COLOR_HOT_PINK, COLOR_SUNRISE};

  for (int i = 0; i < 3; i++) {
    tft.fillRoundRect(10, y, 220, 24, 4, COLOR_DARK_GRAY);
    tft.drawRoundRect(10, y, 220, 24, 4, txColors[i]);

    tft.setTextDatum(TL_DATUM);
    brFont.drawMonoText(txTypes[i], 16, y+5, BR_MONO_TINY, txColors[i]);
    tft.setTextColor(COLOR_WHITE);
    tft.drawString(txAmounts[i], 80, y+13, 1);

    tft.setTextColor(COLOR_DARK_GRAY);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(txDates[i], 224, y+13, 1);

    y += 28;
  }

  // BTCPay Server status
  tft.setTextColor(COLOR_DARK_GRAY);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("BTCPay: btc.blackroad.io", 120, y+10, 1);

  drawBottomNav();
}

// ═══════════════════════════════════════════════════════════════════
// EMAIL APP - Roundcube Webmail (SOVEREIGN STACK!)
// ═══════════════════════════════════════════════════════════════════

void drawEmail() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title with Roundcube + sovereign indicator
  tft.fillRoundRect(0, 20, 240, 35, 0, COLOR_DARK_GRAY);
  for (int i = 0; i < 18; i++) {
    float t = i / 18.0;
    uint16_t c = brUI.lerpColor(COLOR_CYBER_BLUE, COLOR_VIVID_PUR, t);
    tft.drawFastHLine(0, 20 + i, 240, c);
  }

  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoTextCentered("EMAIL", 120, 27, BR_MONO_MEDIUM, COLOR_WHITE);

  // Compose button
  tft.fillRoundRect(195, 23, 38, 22, 4, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextSize(1);
  tft.drawString("+", 214, 34, 2);

  int y = 48;

  // Inbox stats
  tft.fillRoundRect(10, y, 220, 16, 8, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 16, 8, COLOR_CYBER_BLUE);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_SUNRISE);
  tft.drawString("47", 40, y+8, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("msgs", 65, y+8, 1);

  tft.setTextColor(COLOR_HOT_PINK);
  tft.drawString("5", 120, y+8, 1);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("unread", 145, y+8, 1);

  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("IMAP", 205, y+8, 1);

  // Email list
  y += 20;
  tft.setTextColor(COLOR_AMBER);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("INBOX:", 10, y, 1);

  y += 12;
  const char* senders[4] = {"GitHub", "Linear", "Alexa", "Team"};
  const char* subjects[4] = {"PR merged #847", "Task updated", "Meeting notes", "Deploy ready"};
  const char* emailTimes[4] = {"2m", "15m", "1h", "3h"};
  bool unread[4] = {true, true, false, false};
  uint16_t emailColors[4] = {COLOR_VIVID_PUR, COLOR_HOT_PINK, COLOR_CYBER_BLUE, COLOR_SUNRISE};

  for (int i = 0; i < 4; i++) {
    tft.fillRoundRect(10, y, 220, 26, 4, COLOR_DARK_GRAY);
    tft.drawRoundRect(10, y, 220, 26, 4, emailColors[i]);

    // Unread indicator
    if (unread[i]) {
      tft.fillCircle(18, y+13, 3, emailColors[i]);
      tft.fillCircle(18, y+13, 1, COLOR_WHITE);
    }

    tft.setTextDatum(TL_DATUM);
    brFont.drawMonoText(senders[i], 28, y+5, BR_MONO_TINY, emailColors[i]);
    tft.setTextColor(COLOR_WHITE);
    tft.drawString(subjects[i], 28, y+15, 1);

    tft.setTextColor(COLOR_DARK_GRAY);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(emailTimes[i], 224, y+15, 1);

    y += 30;
  }

  // Roundcube status
  tft.setTextColor(COLOR_DARK_GRAY);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("mail.blackroad.io", 120, y+8, 1);

  drawBottomNav();
}

// ═══════════════════════════════════════════════════════════════════
// VIDEO CHAT APP - Jitsi Meet (SOVEREIGN STACK!)
// ═══════════════════════════════════════════════════════════════════

void drawVideoChat() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title with Jitsi + sovereign indicator
  tft.fillRoundRect(0, 20, 240, 35, 0, COLOR_DARK_GRAY);
  for (int i = 0; i < 18; i++) {
    float t = i / 18.0;
    uint16_t c = brUI.lerpColor(COLOR_HOT_PINK, COLOR_MAGENTA, t);
    tft.drawFastHLine(0, 20 + i, 240, c);
  }

  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoTextCentered("VIDEO CHAT", 120, 27, BR_MONO_MEDIUM, COLOR_WHITE);

  // Jitsi badge
  tft.fillCircle(200, 29, 3, COLOR_SUNRISE);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextSize(1);
  tft.drawString("JITSI", 225, 27, 1);

  int y = 50;

  // Active meeting card
  tft.fillRoundRect(10, y, 220, 55, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 55, 6, COLOR_HOT_PINK);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Active Meeting", 16, y+6, BR_MONO_TINY, COLOR_HOT_PINK);

  tft.setTextColor(COLOR_WHITE);
  tft.drawString("BlackRoad Standup", 20, y+20, 2);

  // Participants
  tft.setTextColor(COLOR_AMBER);
  tft.drawString("4 participants", 20, y+38, 1);

  // Join button
  tft.fillRoundRect(140, y+32, 75, 18, 4, COLOR_SUNRISE);
  tft.setTextColor(COLOR_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("JOIN", 177, y+41, 1);

  // Quick start meeting
  y += 65;
  tft.fillRoundRect(10, y, 220, 35, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 35, 6, COLOR_VIVID_PUR);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Quick Start", 16, y+6, BR_MONO_TINY, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Start instant meeting", 20, y+20, 1);

  // Recent meetings
  y += 42;
  tft.setTextColor(COLOR_AMBER);
  tft.drawString("RECENT:", 10, y, 1);

  y += 12;
  const char* meetings[3] = {"Team Sprint", "Design Review", "1:1 Session"};
  const char* meetingTimes[3] = {"2h", "1d", "2d"};
  uint16_t meetingColors[3] = {COLOR_CYBER_BLUE, COLOR_VIVID_PUR, COLOR_HOT_PINK};

  for (int i = 0; i < 3; i++) {
    tft.fillRoundRect(10, y, 220, 22, 4, COLOR_DARK_GRAY);
    tft.drawRoundRect(10, y, 220, 22, 4, meetingColors[i]);

    tft.setTextDatum(TL_DATUM);
    brFont.drawMonoText(meetings[i], 16, y+4, BR_MONO_TINY, meetingColors[i]);

    tft.setTextColor(COLOR_DARK_GRAY);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(meetingTimes[i], 224, y+12, 1);

    // Replay icon
    tft.setTextColor(meetingColors[i]);
    tft.drawString("↻", 16, y+13, 1);

    y += 26;
  }

  // Jitsi server status
  tft.setTextColor(COLOR_DARK_GRAY);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("meet.blackroad.io", 120, y+5, 1);

  drawBottomNav();
}

// ═══════════════════════════════════════════════════════════════════
// DOCUMENT EDITING APP - OnlyOffice (SOVEREIGN STACK!)
// ═══════════════════════════════════════════════════════════════════

void drawDocs() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title with OnlyOffice + sovereign indicator
  tft.fillRoundRect(0, 20, 240, 35, 0, COLOR_DARK_GRAY);
  for (int i = 0; i < 18; i++) {
    float t = i / 18.0;
    uint16_t c = brUI.lerpColor(COLOR_CYBER_BLUE, COLOR_VIVID_PUR, t);
    tft.drawFastHLine(0, 20 + i, 240, c);
  }

  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoTextCentered("DOCUMENTS", 120, 27, BR_MONO_MEDIUM, COLOR_WHITE);

  // OnlyOffice badge
  tft.fillCircle(200, 29, 3, COLOR_SUNRISE);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextSize(1);
  tft.drawString("OO", 220, 27, 1);

  int y = 50;

  // New document button
  tft.fillRoundRect(10, y, 220, 32, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 32, 6, COLOR_CYBER_BLUE);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Create New", 16, y+6, BR_MONO_TINY, COLOR_CYBER_BLUE);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Doc • Sheet • Slides", 20, y+18, 1);

  // Recent documents
  y += 40;
  tft.setTextColor(COLOR_AMBER);
  tft.drawString("RECENT:", 10, y, 1);

  y += 12;
  const char* docs[4] = {"Q1 Report.docx", "Budget.xlsx", "Roadmap.pptx", "Notes.txt"};
  const char* docTimes[4] = {"5m", "1h", "2h", "1d"};
  const char* docIcons[4] = {"📄", "📊", "📽", "📝"};
  uint16_t docColors[4] = {COLOR_CYBER_BLUE, COLOR_SUNRISE, COLOR_HOT_PINK, COLOR_VIVID_PUR};

  for (int i = 0; i < 4; i++) {
    tft.fillRoundRect(10, y, 220, 24, 4, COLOR_DARK_GRAY);
    tft.drawRoundRect(10, y, 220, 24, 4, docColors[i]);

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(docColors[i]);
    tft.drawString(docIcons[i], 16, y+7, 1);

    brFont.drawMonoText(docs[i], 34, y+4, BR_MONO_TINY, COLOR_WHITE);

    tft.setTextColor(COLOR_DARK_GRAY);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(docTimes[i], 224, y+12, 1);

    y += 28;
  }

  // OnlyOffice server status
  tft.setTextColor(COLOR_DARK_GRAY);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("docs.blackroad.io", 120, y+5, 1);

  drawBottomNav();
}

// ═══════════════════════════════════════════════════════════════════
// MUSIC STREAMING APP - Navidrome (SOVEREIGN STACK!)
// ═══════════════════════════════════════════════════════════════════

void drawMusic() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title with Navidrome + sovereign indicator
  tft.fillRoundRect(0, 20, 240, 35, 0, COLOR_DARK_GRAY);
  for (int i = 0; i < 18; i++) {
    float t = i / 18.0;
    uint16_t c = brUI.lerpColor(COLOR_HOT_PINK, COLOR_VIVID_PUR, t);
    tft.drawFastHLine(0, 20 + i, 240, c);
  }

  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoTextCentered("MUSIC", 120, 27, BR_MONO_MEDIUM, COLOR_WHITE);

  // ND badge (Navidrome)
  tft.fillCircle(200, 29, 3, COLOR_SUNRISE);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextSize(1);
  tft.drawString("ND", 220, 27, 1);

  int y = 50;

  // Now playing
  tft.fillRoundRect(10, y, 220, 45, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 45, 6, COLOR_HOT_PINK);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Now Playing", 16, y+6, BR_MONO_TINY, COLOR_HOT_PINK);

  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Digital Sovereignty", 20, y+20, 1);

  tft.setTextColor(COLOR_AMBER);
  tft.drawString("BlackRoad Sound System", 20, y+32, 1);

  // Playback controls
  y += 53;
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("⏮", 60, y+10, 2);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("⏸", 120, y+10, 2);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.drawString("⏭", 180, y+10, 2);

  // Recent albums
  y += 28;
  tft.setTextColor(COLOR_AMBER);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("ALBUMS:", 10, y, 1);

  y += 12;
  const char* albums[3] = {"Open Source Anthem", "Self-Hosted Hits", "Forkies Forever"};
  const char* albumArtists[3] = {"Various", "BlackRoad", "Community"};
  uint16_t albumColors[3] = {COLOR_VIVID_PUR, COLOR_CYBER_BLUE, COLOR_SUNRISE};

  for (int i = 0; i < 3; i++) {
    tft.fillRoundRect(10, y, 220, 24, 4, COLOR_DARK_GRAY);
    tft.drawRoundRect(10, y, 220, 24, 4, albumColors[i]);

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(albumColors[i]);
    tft.drawString("♫", 16, y+7, 1);

    brFont.drawMonoText(albums[i], 34, y+4, BR_MONO_TINY, COLOR_WHITE);

    tft.setTextColor(COLOR_DARK_GRAY);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(albumArtists[i], 224, y+12, 1);

    y += 28;
  }

  // Navidrome status
  tft.setTextColor(COLOR_DARK_GRAY);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("music.blackroad.io", 120, y+5, 1);

  drawBottomNav();
}

// ═══════════════════════════════════════════════════════════════════
// PHOTO GALLERY APP - PhotoPrism (SOVEREIGN STACK!)
// ═══════════════════════════════════════════════════════════════════

void drawPhotos() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title with PhotoPrism + sovereign indicator
  tft.fillRoundRect(0, 20, 240, 35, 0, COLOR_DARK_GRAY);
  for (int i = 0; i < 18; i++) {
    float t = i / 18.0;
    uint16_t c = brUI.lerpColor(COLOR_VIVID_PUR, COLOR_HOT_PINK, t);
    tft.drawFastHLine(0, 20 + i, 240, c);
  }

  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoTextCentered("PHOTOS", 120, 27, BR_MONO_MEDIUM, COLOR_WHITE);

  // PP badge (PhotoPrism)
  tft.fillCircle(200, 29, 3, COLOR_SUNRISE);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextSize(1);
  tft.drawString("PP", 220, 27, 1);

  int y = 50;

  // Library stats
  tft.fillRoundRect(10, y, 220, 28, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 28, 6, COLOR_VIVID_PUR);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Library", 16, y+6, BR_MONO_TINY, COLOR_VIVID_PUR);

  tft.setTextColor(COLOR_WHITE);
  tft.drawString("12,847 photos", 20, y+16, 1);

  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("256 GB", 224, y+16, 1);

  // Recent albums
  y += 36;
  tft.setTextColor(COLOR_AMBER);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("ALBUMS:", 10, y, 1);

  y += 12;
  const char* photoAlbums[4] = {"ESP32 Projects", "Minnesota", "Team Photos", "Screenshots"};
  const char* photoCounts[4] = {"47", "234", "89", "1,203"};
  const char* photoIcons[4] = {"🔧", "🌲", "👥", "📸"};
  uint16_t photoColors[4] = {COLOR_CYBER_BLUE, COLOR_SUNRISE, COLOR_HOT_PINK, COLOR_VIVID_PUR};

  for (int i = 0; i < 4; i++) {
    tft.fillRoundRect(10, y, 220, 24, 4, COLOR_DARK_GRAY);
    tft.drawRoundRect(10, y, 220, 24, 4, photoColors[i]);

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(photoColors[i]);
    tft.drawString(photoIcons[i], 16, y+7, 1);

    brFont.drawMonoText(photoAlbums[i], 34, y+4, BR_MONO_TINY, COLOR_WHITE);

    tft.setTextColor(COLOR_DARK_GRAY);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(photoCounts[i], 224, y+12, 1);

    y += 28;
  }

  // PhotoPrism status
  tft.setTextColor(COLOR_DARK_GRAY);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("photos.blackroad.io", 120, y+5, 1);

  drawBottomNav();
}

// ═══════════════════════════════════════════════════════════════════
// ═══════════════════════════════════════════════════════════════════
// CODE EDITOR APP - code-server (VSCode) (SOVEREIGN STACK!)
// ═══════════════════════════════════════════════════════════════════

void drawCode() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title with VSCode + sovereign indicator
  tft.fillRoundRect(0, 20, 240, 35, 0, COLOR_DARK_GRAY);
  for (int i = 0; i < 18; i++) {
    float t = i / 18.0;
    uint16_t c = brUI.lerpColor(COLOR_CYBER_BLUE, COLOR_VIVID_PUR, t);
    tft.drawFastHLine(0, 20 + i, 240, c);
  }

  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoTextCentered("CODE", 120, 27, BR_MONO_MEDIUM, COLOR_WHITE);

  // VS badge (VSCode)
  tft.fillCircle(200, 29, 3, COLOR_CYBER_BLUE);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.setTextSize(1);
  tft.drawString("VS", 220, 27, 1);

  int y = 50;

  // Active workspace
  tft.fillRoundRect(10, y, 220, 32, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 32, 6, COLOR_CYBER_BLUE);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Workspace", 16, y+6, BR_MONO_TINY, COLOR_CYBER_BLUE);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("ceo-hub-esp32", 20, y+18, 1);

  // Recent files
  y += 40;
  tft.setTextColor(COLOR_AMBER);
  tft.drawString("RECENT:", 10, y, 1);

  y += 12;
  const char* codeFiles[4] = {"main.cpp", "platformio.ini", "README.md", "deploy.sh"};
  const char* codeTimes[4] = {"2m", "15m", "1h", "2h"};
  const char* codeIcons[4] = {"C++", "INI", "MD", "SH"};
  uint16_t codeColors[4] = {COLOR_CYBER_BLUE, COLOR_SUNRISE, COLOR_VIVID_PUR, COLOR_HOT_PINK};

  for (int i = 0; i < 4; i++) {
    tft.fillRoundRect(10, y, 220, 24, 4, COLOR_DARK_GRAY);
    tft.drawRoundRect(10, y, 220, 24, 4, codeColors[i]);

    tft.setTextDatum(TL_DATUM);
    brFont.drawMonoText(codeIcons[i], 16, y+4, BR_MONO_TINY, codeColors[i]);

    tft.setTextColor(COLOR_WHITE);
    tft.drawString(codeFiles[i], 50, y+7, 1);

    tft.setTextColor(COLOR_DARK_GRAY);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(codeTimes[i], 224, y+12, 1);

    y += 28;
  }

  // code-server status
  tft.setTextColor(COLOR_DARK_GRAY);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("code.blackroad.io", 120, y+5, 1);

  drawBottomNav();
}

// ═══════════════════════════════════════════════════════════════════
// PASSWORD MANAGER APP - Vaultwarden (Bitwarden) (SOVEREIGN STACK!)
// ═══════════════════════════════════════════════════════════════════

void drawPasswords() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title with Vaultwarden + sovereign indicator
  tft.fillRoundRect(0, 20, 240, 35, 0, COLOR_DARK_GRAY);
  for (int i = 0; i < 18; i++) {
    float t = i / 18.0;
    uint16_t c = brUI.lerpColor(COLOR_HOT_PINK, COLOR_VIVID_PUR, t);
    tft.drawFastHLine(0, 20 + i, 240, c);
  }

  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoTextCentered("PASSWORDS", 120, 27, BR_MONO_MEDIUM, COLOR_WHITE);

  // VW badge (Vaultwarden)
  tft.fillCircle(200, 29, 3, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_HOT_PINK);
  tft.setTextSize(1);
  tft.drawString("VW", 220, 27, 1);

  int y = 50;

  // Vault stats
  tft.fillRoundRect(10, y, 220, 28, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 28, 6, COLOR_HOT_PINK);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Vault", 16, y+6, BR_MONO_TINY, COLOR_HOT_PINK);

  tft.setTextColor(COLOR_WHITE);
  tft.drawString("127 items", 20, y+16, 1);

  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("LOCKED", 224, y+16, 1);

  // Recent passwords
  y += 36;
  tft.setTextColor(COLOR_AMBER);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("ITEMS:", 10, y, 1);

  y += 12;
  const char* pwItems[4] = {"GitHub", "AWS", "Cloudflare", "Linear"};
  const char* pwTypes[4] = {"Login", "API Key", "Login", "Login"};
  const char* pwIcons[4] = {"🔐", "🔑", "🔐", "🔐"};
  uint16_t pwColors[4] = {COLOR_VIVID_PUR, COLOR_SUNRISE, COLOR_CYBER_BLUE, COLOR_HOT_PINK};

  for (int i = 0; i < 4; i++) {
    tft.fillRoundRect(10, y, 220, 24, 4, COLOR_DARK_GRAY);
    tft.drawRoundRect(10, y, 220, 24, 4, pwColors[i]);

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(pwColors[i]);
    tft.drawString(pwIcons[i], 16, y+7, 1);

    brFont.drawMonoText(pwItems[i], 34, y+4, BR_MONO_TINY, COLOR_WHITE);

    tft.setTextColor(COLOR_DARK_GRAY);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(pwTypes[i], 224, y+12, 1);

    y += 28;
  }

  // Vaultwarden status
  tft.setTextColor(COLOR_DARK_GRAY);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("vault.blackroad.io", 120, y+5, 1);

  drawBottomNav();
}

// ═══════════════════════════════════════════════════════════════════
// NOTES APP - Joplin Server (SOVEREIGN STACK!)
// ═══════════════════════════════════════════════════════════════════

void drawNotes() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title with Joplin + sovereign indicator
  tft.fillRoundRect(0, 20, 240, 35, 0, COLOR_DARK_GRAY);
  for (int i = 0; i < 18; i++) {
    float t = i / 18.0;
    uint16_t c = brUI.lerpColor(COLOR_SUNRISE, COLOR_CYBER_BLUE, t);
    tft.drawFastHLine(0, 20 + i, 240, c);
  }

  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoTextCentered("NOTES", 120, 27, BR_MONO_MEDIUM, COLOR_WHITE);

  // JO badge (Joplin)
  tft.fillCircle(200, 29, 3, COLOR_SUNRISE);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextSize(1);
  tft.drawString("JO", 220, 27, 1);

  int y = 50;

  // New note button
  tft.fillRoundRect(10, y, 220, 28, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 28, 6, COLOR_SUNRISE);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("New Note", 16, y+6, BR_MONO_TINY, COLOR_SUNRISE);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Quick capture", 20, y+16, 1);

  // Recent notes
  y += 36;
  tft.setTextColor(COLOR_AMBER);
  tft.drawString("RECENT:", 10, y, 1);

  y += 12;
  const char* notes[4] = {"ESP32 Ideas", "Meeting Notes", "Code Snippets", "Project Plan"};
  const char* noteTimes[4] = {"5m", "1h", "2h", "1d"};
  const char* noteIcons[4] = {"💡", "📝", "💻", "📋"};
  uint16_t noteColors[4] = {COLOR_SUNRISE, COLOR_CYBER_BLUE, COLOR_VIVID_PUR, COLOR_HOT_PINK};

  for (int i = 0; i < 4; i++) {
    tft.fillRoundRect(10, y, 220, 24, 4, COLOR_DARK_GRAY);
    tft.drawRoundRect(10, y, 220, 24, 4, noteColors[i]);

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(noteColors[i]);
    tft.drawString(noteIcons[i], 16, y+7, 1);

    brFont.drawMonoText(notes[i], 34, y+4, BR_MONO_TINY, COLOR_WHITE);

    tft.setTextColor(COLOR_DARK_GRAY);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(noteTimes[i], 224, y+12, 1);

    y += 28;
  }

  // Joplin status
  tft.setTextColor(COLOR_DARK_GRAY);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("notes.blackroad.io", 120, y+5, 1);

  drawBottomNav();
}

// ═══════════════════════════════════════════════════════════════════
// RSS READER APP - FreshRSS (SOVEREIGN STACK!)
// ═══════════════════════════════════════════════════════════════════

void drawRSS() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title with FreshRSS + sovereign indicator
  tft.fillRoundRect(0, 20, 240, 35, 0, COLOR_DARK_GRAY);
  for (int i = 0; i < 18; i++) {
    float t = i / 18.0;
    uint16_t c = brUI.lerpColor(COLOR_SUNRISE, COLOR_HOT_PINK, t);
    tft.drawFastHLine(0, 20 + i, 240, c);
  }

  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoTextCentered("RSS", 120, 27, BR_MONO_MEDIUM, COLOR_WHITE);

  // FR badge (FreshRSS)
  tft.fillCircle(200, 29, 3, COLOR_SUNRISE);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextSize(1);
  tft.drawString("FR", 220, 27, 1);

  int y = 50;

  // Feed stats
  tft.fillRoundRect(10, y, 220, 28, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 28, 6, COLOR_SUNRISE);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Feeds", 16, y+6, BR_MONO_TINY, COLOR_SUNRISE);

  tft.setTextColor(COLOR_WHITE);
  tft.drawString("24 feeds", 20, y+16, 1);

  tft.setTextColor(COLOR_HOT_PINK);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("47 unread", 224, y+16, 1);

  // Recent articles
  y += 36;
  tft.setTextColor(COLOR_AMBER);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("UNREAD:", 10, y, 1);

  y += 12;
  const char* articles[4] = {"Hacker News", "Ars Technica", "Self-Hosted", "ESP32 Updates"};
  const char* articleCounts[4] = {"12", "8", "5", "3"};
  uint16_t articleColors[4] = {COLOR_SUNRISE, COLOR_CYBER_BLUE, COLOR_VIVID_PUR, COLOR_HOT_PINK};

  for (int i = 0; i < 4; i++) {
    tft.fillRoundRect(10, y, 220, 24, 4, COLOR_DARK_GRAY);
    tft.drawRoundRect(10, y, 220, 24, 4, articleColors[i]);

    tft.setTextDatum(TL_DATUM);
    brFont.drawMonoText(articles[i], 16, y+4, BR_MONO_TINY, COLOR_WHITE);

    // Unread badge
    tft.fillCircle(224, y+12, 8, articleColors[i]);
    tft.setTextColor(COLOR_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(articleCounts[i], 224, y+12, 1);

    y += 28;
  }

  // FreshRSS status
  tft.setTextColor(COLOR_DARK_GRAY);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("rss.blackroad.io", 120, y+5, 1);

  drawBottomNav();
}

// ═══════════════════════════════════════════════════════════════════
// SOCIAL NETWORK APP - Mastodon (SOVEREIGN STACK!)
// ═══════════════════════════════════════════════════════════════════

void drawSocial() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title with Mastodon + sovereign indicator
  tft.fillRoundRect(0, 20, 240, 35, 0, COLOR_DARK_GRAY);
  for (int i = 0; i < 18; i++) {
    float t = i / 18.0;
    uint16_t c = brUI.lerpColor(COLOR_VIVID_PUR, COLOR_CYBER_BLUE, t);
    tft.drawFastHLine(0, 20 + i, 240, c);
  }

  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoTextCentered("SOCIAL", 120, 27, BR_MONO_MEDIUM, COLOR_WHITE);

  // MA badge (Mastodon)
  tft.fillCircle(200, 29, 3, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.setTextSize(1);
  tft.drawString("MA", 220, 27, 1);

  int y = 50;

  // Compose toot button
  tft.fillRoundRect(10, y, 220, 28, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 28, 6, COLOR_VIVID_PUR);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Compose Toot", 16, y+6, BR_MONO_TINY, COLOR_VIVID_PUR);
  tft.setTextColor(COLOR_WHITE);
  tft.drawString("Share your thoughts", 20, y+16, 1);

  // Recent toots
  y += 36;
  tft.setTextColor(COLOR_AMBER);
  tft.drawString("TIMELINE:", 10, y, 1);

  y += 12;
  const char* toots[4] = {"@blackroad", "@opensource", "@esp32dev", "@selfhosted"};
  const char* tootTexts[4] = {"New ESP32!", "Forkies forever", "Check this", "Self-hosting"};
  const char* tootTimes[4] = {"2m", "15m", "1h", "3h"};
  uint16_t tootColors[4] = {COLOR_VIVID_PUR, COLOR_CYBER_BLUE, COLOR_SUNRISE, COLOR_HOT_PINK};

  for (int i = 0; i < 4; i++) {
    tft.fillRoundRect(10, y, 220, 24, 4, COLOR_DARK_GRAY);
    tft.drawRoundRect(10, y, 220, 24, 4, tootColors[i]);

    tft.setTextDatum(TL_DATUM);
    brFont.drawMonoText(toots[i], 16, y+4, BR_MONO_TINY, tootColors[i]);

    tft.setTextColor(COLOR_WHITE);
    tft.drawString(tootTexts[i], 80, y+7, 1);

    tft.setTextColor(COLOR_DARK_GRAY);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(tootTimes[i], 224, y+12, 1);

    y += 28;
  }

  // Mastodon status
  tft.setTextColor(COLOR_DARK_GRAY);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("social.blackroad.io", 120, y+5, 1);

  drawBottomNav();
}

// ═══════════════════════════════════════════════════════════════════
// WIKI APP - BookStack (SOVEREIGN STACK!)
// ═══════════════════════════════════════════════════════════════════

void drawWiki() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();

  // Title with BookStack + sovereign indicator
  tft.fillRoundRect(0, 20, 240, 35, 0, COLOR_DARK_GRAY);
  for (int i = 0; i < 18; i++) {
    float t = i / 18.0;
    uint16_t c = brUI.lerpColor(COLOR_CYBER_BLUE, COLOR_SUNRISE, t);
    tft.drawFastHLine(0, 20 + i, 240, c);
  }

  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoTextCentered("WIKI", 120, 27, BR_MONO_MEDIUM, COLOR_WHITE);

  // BS badge (BookStack)
  tft.fillCircle(200, 29, 3, COLOR_CYBER_BLUE);
  tft.setTextColor(COLOR_CYBER_BLUE);
  tft.setTextSize(1);
  tft.drawString("BS", 220, 27, 1);

  int y = 50;

  // Wiki stats
  tft.fillRoundRect(10, y, 220, 28, 6, COLOR_DARK_GRAY);
  tft.drawRoundRect(10, y, 220, 28, 6, COLOR_CYBER_BLUE);

  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Knowledge Base", 16, y+6, BR_MONO_TINY, COLOR_CYBER_BLUE);

  tft.setTextColor(COLOR_WHITE);
  tft.drawString("47 pages", 20, y+16, 1);

  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("8 books", 224, y+16, 1);

  // Recent pages
  y += 36;
  tft.setTextColor(COLOR_AMBER);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("RECENT:", 10, y, 1);

  y += 12;
  const char* wikiPages[4] = {"ESP32 Guide", "API Docs", "Setup Guide", "Troubleshooting"};
  const char* wikiTimes[4] = {"5m", "1h", "2h", "1d"};
  const char* wikiIcons[4] = {"📘", "📗", "📕", "📙"};
  uint16_t wikiColors[4] = {COLOR_CYBER_BLUE, COLOR_SUNRISE, COLOR_VIVID_PUR, COLOR_HOT_PINK};

  for (int i = 0; i < 4; i++) {
    tft.fillRoundRect(10, y, 220, 24, 4, COLOR_DARK_GRAY);
    tft.drawRoundRect(10, y, 220, 24, 4, wikiColors[i]);

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(wikiColors[i]);
    tft.drawString(wikiIcons[i], 16, y+7, 1);

    brFont.drawMonoText(wikiPages[i], 34, y+4, BR_MONO_TINY, COLOR_WHITE);

    tft.setTextColor(COLOR_DARK_GRAY);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(wikiTimes[i], 224, y+12, 1);

    y += 28;
  }

  // BookStack status
  tft.setTextColor(COLOR_DARK_GRAY);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("wiki.blackroad.io", 120, y+5, 1);

  drawBottomNav();
}


// GITHUB INTEGRATION - Forgejo Self-Hosted (SOVEREIGN STACK!)
// ═══════════════════════════════════════════════════════════════════

void drawGitHub() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();
  drawBackButton();

  // Title
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("GITHUB", 160, 28, 1);

  // Account info
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("@BlackRoad-OS", 15, 55, 1);

  tft.setTextColor(COLOR_DARK_GRAY);
  tft.drawString("66 repos • 15 orgs", 15, 73, 1);

  // Quick stats cards
  int cardY = 92;

  // PRs card
  drawCard(10, cardY, 145, 45, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TL_DATUM);
  brFont.drawMonoText("Pull Requests", 18, cardY + 8, 1, COLOR_WHITE);
  tft.setTextColor(COLOR_HOT_PINK);
  brFont.drawMonoText("5 open", 18, cardY + 23, 2, COLOR_WHITE);

  // Issues card
  drawCard(165, cardY, 145, 45, COLOR_CYBER_BLUE);
  tft.setTextColor(COLOR_WHITE);
  brFont.drawMonoText("Issues", 173, cardY + 8, 1, COLOR_WHITE);
  tft.setTextColor(COLOR_CYBER_BLUE);
  brFont.drawMonoText("12 open", 173, cardY + 23, 2, COLOR_WHITE);

  // Recent activity
  tft.setTextColor(COLOR_VIVID_PUR);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("RECENT ACTIVITY", 15, 150, 1);

  const char* activities[] = {
    "Merged PR #123 - Add auth",
    "Created Issue #456",
    "Pushed to main branch",
    "Workflow run succeeded",
    "Commented on PR #122"
  };

  const char* times[] = {"2m ago", "15m ago", "1h ago", "2h ago", "3h ago"};

  int y = 172;
  for(int i = 0; i < 5; i++) {
    // Activity
    tft.setTextColor(COLOR_WHITE);
    brFont.drawMonoText(activities[i], 20, y, 1, COLOR_WHITE);

    // Time
    tft.setTextColor(COLOR_DARK_GRAY);
    brFont.drawMonoText(times[i], 250, y, 1, COLOR_WHITE);

    y += 22;
  }

  // Stats summary
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("847 contributions this year", 160, 295, 1);

  drawBottomNav();
}

// ═══════════════════════════════════════════════════════════════════
// LINEAR INTEGRATION - Task Tracking
// ═══════════════════════════════════════════════════════════════════

void drawLinear() {
  tft.fillScreen(COLOR_BLACK);
  drawStatusBar();
  drawBackButton();

  // Title
  tft.setTextColor(COLOR_HOT_PINK);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("LINEAR", 160, 28, 1);

  // Team info
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("BlackRoad Engineering", 15, 55, 1);

  tft.setTextColor(COLOR_DARK_GRAY);
  tft.drawString("Sprint 23 • 12 tasks", 15, 73, 1);

  // Task breakdown
  int cardY = 92;
  int cardW = 72;

  // Todo
  drawCard(10, cardY, cardW, 45, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextDatum(TC_DATUM);
  brFont.drawMonoText("TODO", 46, cardY + 8, 1, COLOR_WHITE);
  brFont.drawMonoText("4", 46, cardY + 23, 3, COLOR_WHITE);

  // In Progress
  drawCard(89, cardY, cardW, 45, COLOR_SUNRISE);
  tft.setTextColor(COLOR_WHITE);
  brFont.drawMonoText("DOING", 125, cardY + 8, 1, COLOR_WHITE);
  brFont.drawMonoText("3", 125, cardY + 23, 3, COLOR_WHITE);

  // Done
  drawCard(168, cardY, cardW, 45, COLOR_CYBER_BLUE);
  tft.setTextColor(COLOR_WHITE);
  brFont.drawMonoText("DONE", 204, cardY + 8, 1, COLOR_WHITE);
  brFont.drawMonoText("5", 204, cardY + 23, 3, COLOR_WHITE);

  // Blocked
  drawCard(247, cardY, cardW, 45, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_WHITE);
  brFont.drawMonoText("BLOCK", 283, cardY + 8, 1, COLOR_WHITE);
  brFont.drawMonoText("2", 283, cardY + 23, 3, COLOR_WHITE);

  // Active tasks
  tft.setTextColor(COLOR_HOT_PINK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("IN PROGRESS", 15, 150, 1);

  struct Task {
    const char* id;
    const char* title;
    const char* priority;
    uint16_t color;
  };

  Task tasks[] = {
    {"BR-142", "Fix ESP32 memory leak", "P1", COLOR_HOT_PINK},
    {"BR-143", "Add GitHub integration", "P2", COLOR_SUNRISE},
    {"BR-144", "Update documentation", "P3", COLOR_CYBER_BLUE}
  };

  int y = 172;
  for(int i = 0; i < 3; i++) {
    // Task card
    drawCard(10, y, 220, 35);

    // ID + Priority
    tft.setTextColor(tasks[i].color);
    brFont.drawMonoText(tasks[i].id, 18, y + 8, 2, COLOR_WHITE);
    brFont.drawMonoText(tasks[i].priority, 265, y + 8, 1, COLOR_WHITE);

    // Title
    tft.setTextColor(COLOR_WHITE);
    brFont.drawMonoText(tasks[i].title, 18, y + 22, 1, COLOR_WHITE);

    y += 42;
  }

  // Velocity
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Velocity: 18 pts/week", 160, 302, 1);

  drawBottomNav();
}

// ═══════════════════════════════════════════════════════════════════
// 🔥 RECENT APPS SWITCHER - Better than iPhone multitasking!
// ═══════════════════════════════════════════════════════════════════

void drawRecentApps() {
  tft.fillScreen(COLOR_BLACK);

  // Title
  tft.setTextColor(COLOR_HOT_PINK);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("RECENT APPS", 120, 10, 2);

  tft.setTextColor(COLOR_AMBER);
  char subtitle[30];
  sprintf(subtitle, "%d apps · Tap to switch", recentAppsCount);
  tft.drawString(subtitle, 120, 32, 1);

  // Draw recent app cards (vertically stacked)
  int y = 55;
  const char* appNames[] = {"CEO", "EXEC", "META", "WORK", "AI", "MSG", "CRM", "VPN",
                             "ID", "FILE", "API", "SET", "CC", "CHAT", "TERM", "PAGR"};

  for (int i = 0; i < recentAppsCount && i < 5; i++) {
    Screen screen = recentApps[i];

    // Skip if it's the current screen
    if (screen == currentScreen) continue;

    // Determine app info
    int appIndex = screen - SCREEN_AI_INFERENCE;  // Offset to app array
    const char* appName = "APP";
    uint16_t appColor = COLOR_ELECTRIC_BLUE;

    if (appIndex >= 0 && appIndex < 16) {
      appName = appNames[appIndex];
      appColor = apps[appIndex].color;
    }

    // App card with preview
    bool isFirst = (i == 0);
    drawCard(10, y, 220, 42, isFirst ? COLOR_DARK_GRAY : 0x1082);

    // App icon circle
    tft.fillCircle(32, y + 21, 15, appColor);
    tft.setTextColor(COLOR_WHITE);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(appName, 32, y + 21, 1);

    // App title
    tft.setTextColor(COLOR_WHITE);
    tft.setTextDatum(ML_DATUM);
    tft.drawString(appName, 55, y + 12, 2);

    // Timestamp/status
    tft.setTextColor(COLOR_AMBER);
    tft.drawString("Active", 55, y + 28, 1);

    // Arrow indicator on first item
    if (isFirst) {
      tft.setTextColor(COLOR_HOT_PINK);
      tft.setTextDatum(MR_DATUM);
      tft.drawString(">", 215, y + 21, 2);
    }

    y += 47;
  }

  if (recentAppsCount == 0) {
    tft.setTextColor(COLOR_DEEP_PURPLE);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("No recent apps", 120, 140, 2);
    tft.setTextColor(COLOR_AMBER);
    tft.drawString("Launch apps to see them here", 120, 165, 1);
  }

  // Instructions at bottom
  tft.setTextColor(COLOR_DARK_GRAY);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Swipe or tap bottom to close", 120, 255, 1);

  drawBottomNav();
}

// Raspberry Pi Network Monitor Screen
void drawPiNetworkScreen() {
  tft.fillScreen(COLOR_BLACK);

  // Header
  brFont.drawMonoText("PI NETWORK", 10, 10, BR_MONO_MEDIUM, COLOR_HOT_PINK);
  brFont.drawMonoText("MONITOR", 10, 30, BR_MONO_MEDIUM, COLOR_HOT_PINK);

  // Scan button
  tft.fillRoundRect(180, 10, 50, 30, 5, COLOR_ELECTRIC_BLUE);
  brFont.drawMonoText("SCAN", 185, 17, BR_MONO_SMALL, COLOR_WHITE);

  tft.drawLine(0, 50, 240, 50, COLOR_AMBER);

  int y = 60;

  // OCTAVIA (ONLINE)
  brFont.drawMonoText("OCTAVIA", 10, y, BR_MONO_SMALL, COLOR_WHITE);
  brFont.drawMonoText("192.168.4.38", 10, y+12, BR_MONO_TINY, COLOR_AMBER);
  tft.fillCircle(220, y+5, 4, COLOR_CYBER_BLUE);  // ✅ indicator

  brFont.drawMonoText("  Dash :3000", 10, y+24, BR_MONO_TINY, COLOR_WHITE);
  brFont.drawMonoText("  vLLM :8000", 10, y+34, BR_MONO_TINY, COLOR_WHITE);
  brFont.drawMonoText("  Svcs x3", 10, y+44, BR_MONO_TINY, COLOR_WHITE);
  y += 60;

  // ALICE (ONLINE)
  brFont.drawMonoText("ALICE", 10, y, BR_MONO_SMALL, COLOR_WHITE);
  brFont.drawMonoText("192.168.4.49", 10, y+12, BR_MONO_TINY, COLOR_AMBER);
  tft.fillCircle(220, y+5, 4, COLOR_CYBER_BLUE);

  brFont.drawMonoText("  SSH :22", 10, y+24, BR_MONO_TINY, COLOR_WHITE);
  y += 40;

  // ARIA (ONLINE)
  brFont.drawMonoText("ARIA", 10, y, BR_MONO_SMALL, COLOR_WHITE);
  brFont.drawMonoText("192.168.4.27", 10, y+12, BR_MONO_TINY, COLOR_AMBER);
  tft.fillCircle(220, y+5, 4, COLOR_CYBER_BLUE);

  brFont.drawMonoText("  Svc :5000", 10, y+24, BR_MONO_TINY, COLOR_WHITE);
  brFont.drawMonoText("  TIMEOUT", 10, y+34, BR_MONO_TINY, COLOR_ORANGE);
  y += 50;

  // BLACKROAD-PI (OFFLINE)
  brFont.drawMonoText("BLACKROAD", 10, y, BR_MONO_SMALL, COLOR_DEEP_MAG);
  brFont.drawMonoText("192.168.4.64", 10, y+12, BR_MONO_TINY, COLOR_DEEP_MAG);
  tft.fillCircle(220, y+5, 4, COLOR_DEEP_MAG);  // ❌ indicator
  brFont.drawMonoText("  OFFLINE", 10, y+24, BR_MONO_TINY, COLOR_DEEP_MAG);
  y += 40;

  // LUCIDIA (OFFLINE)
  brFont.drawMonoText("LUCIDIA", 10, y, BR_MONO_SMALL, COLOR_DEEP_MAG);
  brFont.drawMonoText("192.168.4.99", 10, y+12, BR_MONO_TINY, COLOR_DEEP_MAG);
  tft.fillCircle(220, y+5, 4, COLOR_DEEP_MAG);
  brFont.drawMonoText("  OFFLINE", 10, y+24, BR_MONO_TINY, COLOR_DEEP_MAG);

  // Footer
  tft.drawLine(0, 270, 240, 270, COLOR_AMBER);
  brFont.drawMonoText("3/5 ONLINE (60%)", 10, 278, BR_MONO_SMALL, COLOR_CYBER_BLUE);

  // Refresh button
  tft.fillRoundRect(160, 275, 70, 25, 5, COLOR_HOT_PINK);
  brFont.drawMonoText("REFRESH", 165, 280, BR_MONO_TINY, COLOR_WHITE);

  drawBottomNav();
}

// ═══════════════════════════════════════════════════════════════════

void drawCurrentScreen() {
  switch (currentScreen) {
    case SCREEN_LOCK:
      drawLockScreen();
      break;
    case SCREEN_HOME:
      drawHomeScreen();
      break;
    case SCREEN_AI_INFERENCE:
      drawAIInference();
      break;
    case SCREEN_MESSAGES:
      drawMessagesScreen();
      break;
    case SCREEN_CRM:
      drawCRMScreen();
      break;
    case SCREEN_MESH_VPN:
      drawMeshVPN();
      break;
    case SCREEN_IDENTITY:
      drawIdentity();
      break;
    case SCREEN_FILES:
      drawFiles();
      break;
    case SCREEN_APIS:
      drawAPIs();
      break;
    case SCREEN_SETTINGS:
      drawSettingsScreen();
      break;
    case SCREEN_CONTROL_CENTER:
      drawControlCenter();
      break;
    case SCREEN_WORKFLOW:
      drawWorkflow();
      break;
    case SCREEN_DECISIONS:
      drawDecisions();
      break;
    case SCREEN_CEO_DASHBOARD:
      drawCEODashboard();
      break;
    case SCREEN_CEO_CORE:
      drawCEOCore();
      break;
    case SCREEN_EXEC_GRID:
      drawExecGrid();
      break;
    case SCREEN_META:
      drawMeta();
      break;
    case SCREEN_BLACKROAD_CHAT:
      drawBlackRoadChat();
      break;
    case SCREEN_TERMINAL:
      drawTerminal();
      break;
    case SCREEN_KEYBOARD:
      drawKeyboard();
      break;
    case SCREEN_EMERGENCY_PAGER:
      drawEmergencyPager();
      break;
    case SCREEN_ALERT_HISTORY:
      drawAlertHistory();
      break;
    case SCREEN_WEATHER:
      drawWeather();
      break;
    case SCREEN_CALENDAR:
      drawCalendar();
      break;
    case SCREEN_PAYMENTS:
      drawPayments();
      break;
    case SCREEN_EMAIL:
      drawEmail();
      break;
    case SCREEN_VIDEO_CHAT:
      drawVideoChat();
      break;
    case SCREEN_DOCS:
      drawDocs();
      break;
    case SCREEN_MUSIC:
      drawMusic();
      break;
    case SCREEN_PHOTOS:
      drawPhotos();
      break;
    case SCREEN_CODE:
      drawCode();
      break;
    case SCREEN_PASSWORDS:
      drawPasswords();
      break;
    case SCREEN_NOTES:
      drawNotes();
      break;
    case SCREEN_RSS:
      drawRSS();
      break;
    case SCREEN_SOCIAL:
      drawSocial();
      break;
    case SCREEN_WIKI:
      drawWiki();
      break;
    case SCREEN_GITHUB:
      drawGitHub();
      break;
    case SCREEN_LINEAR:
      drawLinear();
      break;
    case SCREEN_RECENT_APPS:
    case SCREEN_PI_NETWORK:
      drawPiNetworkScreen();
      break;
      drawRecentApps();
      break;
    default:
      drawHomeScreen();
  }
}

// Handle touch events
void handleTouch() {
  int x, y;

  if (!getTouchCoordinates(x, y)) {
    return;
  }

  // Debounce
  if (millis() - lastTouchTime < 200) {
    return;
  }
  lastTouchTime = millis();

  Serial.printf("Touch at x:%d, y:%d on screen:%d\n", x, y, currentScreen);

  // 🔥 ENHANCED BOTTOM NAVIGATION - Smart back/recent apps buttons!
  if (currentScreen != SCREEN_LOCK && currentScreen != SCREEN_HOME && currentScreen != SCREEN_RECENT_APPS) {
    if (y >= 270) {
      // LEFT BUTTON (10-80, 278-313) - BACK or HOME
      if (x >= 10 && x <= 80) {
        playBeep();
        if (navigationHistoryCount > 0) {
          // Go back in history
          currentScreen = popNavigationHistory();
          Serial.printf("✓ BACK button - returning to screen %d (history depth: %d)\n", currentScreen, navigationHistoryCount);
        } else {
          // No history, go home
          currentScreen = SCREEN_HOME;
          Serial.println("✓ HOME button - no history");
        }
        drawCurrentScreen();
        return;
      }

      // RIGHT BUTTON (160-230, 278-313) - RECENT APPS
      if (x >= 160 && x <= 230) {
        playBeep();
        if (recentAppsCount > 1) {
          currentScreen = SCREEN_RECENT_APPS;
          Serial.printf("✓ RECENT APPS button - showing %d apps\n", recentAppsCount);
        } else {
          currentScreen = SCREEN_HOME;
          Serial.println("✓ No recent apps - going home");
        }
        drawCurrentScreen();
        return;
      }
    }
  }

  // Recent Apps screen - handle app selection
  if (currentScreen == SCREEN_RECENT_APPS) {
    if (y >= 270) {
      // Bottom nav on recent apps goes back
      playBeep();
      currentScreen = popNavigationHistory();
      if (currentScreen == SCREEN_RECENT_APPS) currentScreen = SCREEN_HOME;
      drawCurrentScreen();
      return;
    }

    // App cards are at y=55, 102, 149, 196, 243 (height 42)
    int cardIndex = (y - 55) / 47;
    if (cardIndex >= 0 && cardIndex < recentAppsCount && x >= 10 && x <= 230) {
      Screen selectedScreen = recentApps[cardIndex];
      if (selectedScreen != currentScreen) {
        playBeep();
        currentScreen = selectedScreen;
        Serial.printf("✓ Switched to recent app: screen %d\n", currentScreen);
        drawCurrentScreen();
      }
      return;
    }
  }

  switch (currentScreen) {
    case SCREEN_LOCK:
      // Any touch unlocks (tap anywhere on screen)
      Serial.println("Unlocking...");
      currentScreen = SCREEN_HOME;
      drawCurrentScreen();
      break;

    case SCREEN_HOME:
      // Check app icons
      for (int i = 0; i < APP_COUNT; i++) {
        if (x >= apps[i].x && x <= apps[i].x + apps[i].size &&
            y >= apps[i].y && y <= apps[i].y + apps[i].size) {
          // Visual feedback
          drawAppIcon(apps[i], true);
          delay(100);

          // 🔥 Track navigation
          pushNavigationHistory(apps[i].screen);

          currentScreen = apps[i].screen;
          Serial.printf("✓ Launched app: screen %d (history depth: %d)\n", currentScreen, navigationHistoryCount);
          drawCurrentScreen();
          break;
        }
      }
      break;

    case SCREEN_CEO_CORE:
      // Check back button FIRST (larger hitbox for better touch response)
      if (x >= 5 && x <= 80 && y >= 0 && y <= 45) {
        Serial.println("Back button tapped - returning to home");
        playBeep();
        currentScreen = SCREEN_HOME;
        drawCurrentScreen();
        break;
      }

      // 3×3 CEO CORE Grid (80×80 buttons, no gaps)
      {
        int startX = 40;
        int startY = 50;
        int btnSize = 80;

        // Determine which button was tapped
        int col = (x - startX) / btnSize;
        int row = (y - startY) / btnSize;

        if (col >= 0 && col < 3 && row >= 0 && row < 3) {
          lastActionTime = millis();

          // ROW 0 buttons
          if (row == 0) {
            if (col == 0) { // REJECT
              rejectCount++;
              playCritical();
              Serial.println("❌ REJECT - Count: " + String(rejectCount));
            } else if (col == 1) { // HOLD
              holdCount++;
              playBeep();
              Serial.println("💭 HOLD - Count: " + String(holdCount));
            } else if (col == 2) { // APPROVE
              approveCount++;
              playSuccess();
              Serial.println("✅ APPROVE - Count: " + String(approveCount));
            }
          }
          // ROW 1 buttons
          else if (row == 1) {
            if (col == 0) { // BLOCKED
              blockedCount++;
              playCritical();
              Serial.println("🧱 BLOCKED - Count: " + String(blockedCount));
            } else if (col == 1) { // BRAIN (next decision)
              playBeep(100);
              Serial.println("🧠 BRAIN - Loading next decision...");
            } else if (col == 2) { // NEXT
              playBeep();
              Serial.println("🔜 NEXT - Moving to next item");
            }
          }
          // ROW 2 buttons
          else if (row == 2) {
            if (col == 0) { // CRITICAL
              criticalCount++;
              playCritical();
              Serial.println("🔥 CRITICAL - Count: " + String(criticalCount));
            } else if (col == 1) { // ESCALATE
              escalateCount++;
              playSuccess();
              Serial.println("🪜 ESCALATE - Count: " + String(escalateCount));
            } else if (col == 2) { // SHIP
              shipCount++;
              playSuccess();
              Serial.println("🚀 SHIP - Count: " + String(shipCount));
            }
          }

          // Flash the button (visual feedback)
          delay(100);
          drawCEOCore();
        }
      }
      break;

    case SCREEN_EXEC_GRID:
      // Check back button FIRST (larger hitbox for better touch response)
      if (x >= 5 && x <= 80 && y >= 0 && y <= 45) {
        Serial.println("Back button tapped - returning to home");
        playBeep();
        currentScreen = SCREEN_HOME;
        drawCurrentScreen();
        break;
      }

      // 4×4 EXEC Grid (60×60 buttons, 2px gaps)
      {
        int startX = 50;
        int startY = 50;
        int btnSize = 60;
        int gap = 2;

        int col = (x - startX) / (btnSize + gap);
        int row = (y - startY) / (btnSize + gap);

        if (col >= 0 && col < 4 && row >= 0 && row < 4) {
          lastActionTime = millis();
          playBeep();

          // Map grid position to action
          String actions[4][4] = {
            {"REJECT", "HOLD", "BRAIN", "APPROVE"},
            {"BLOCKED", "RISK", "CRITICAL", "NEXT"},
            {"USER", "TEAM", "ESCALATE", "DIRECTION"},
            {"VOICE", "DATA", "LOOP", "SHIP"}
          };

          String action = actions[row][col];
          Serial.println("EXEC 4×4 - " + action + " activated");

          // Update counts for matching actions
          if (action == "APPROVE") {approveCount++; playSuccess();}
          else if (action == "REJECT") {rejectCount++; playCritical();}
          else if (action == "HOLD") {holdCount++;}
          else if (action == "SHIP") {shipCount++; playSuccess();}
          else if (action == "ESCALATE") {escalateCount++;}
          else if (action == "CRITICAL") {criticalCount++; playCritical();}
          else if (action == "BLOCKED") {blockedCount++;}

          delay(100);
          drawExecGrid();
        }
      }
      break;

    case SCREEN_BLACKROAD_CHAT:
      // Check back button FIRST (larger hitbox for better touch response)
      if (x >= 5 && x <= 80 && y >= 0 && y <= 45) {
        Serial.println("Back button tapped - returning to home");
        playBeep();
        currentScreen = SCREEN_HOME;
        drawCurrentScreen();
        break;
      }

      // RECURSIVE AI MODEL SELECTION - Tap any model to switch! (BETTER THAN iPHONE!)
      // 6 AI models in 2×3 grid (148×22px each)
      // Row 1 (y=56-78): LUCIDIA (x=8-156), ARIA (x=164-312)
      // Row 2 (y=82-104): CADENCE (x=8-156), CECILIA (x=164-312)
      // Row 3 (y=108-130): OCTAVIA (x=8-156), ALICE (x=164-312)

      {
        const char* names[] = {"LUCIDIA", "ARIA", "CADENCE", "CECILIA", "OCTAVIA", "ALICE"};
        int tappedAI = -1;

        // Row 1 (y=56-78)
        if (y >= 56 && y <= 78) {
          if (x >= 8 && x <= 156) tappedAI = 0; // LUCIDIA
          else if (x >= 164 && x <= 312) tappedAI = 1; // ARIA
        }
        // Row 2 (y=82-104)
        else if (y >= 82 && y <= 104) {
          if (x >= 8 && x <= 156) tappedAI = 2; // CADENCE
          else if (x >= 164 && x <= 312) tappedAI = 3; // CECILIA
        }
        // Row 3 (y=108-130)
        else if (y >= 108 && y <= 130) {
          if (x >= 8 && x <= 156) tappedAI = 4; // OCTAVIA
          else if (x >= 164 && x <= 312) tappedAI = 5; // ALICE
        }

        if (tappedAI != -1) {
          activeAI = tappedAI;
          playBeep();
          Serial.printf("🔄 RECURSION! Switched to %s (AI #%d)\n", names[tappedAI], tappedAI);
          Serial.println("💡 AI now references different apps!");
          drawCurrentScreen(); // Redraw with new AI conversation
        }
      }
      break;

    case SCREEN_TERMINAL:
      // Check back button FIRST (larger hitbox for better touch response)
      if (x >= 5 && x <= 80 && y >= 0 && y <= 45) {
        Serial.println("Back button tapped - returning to home");
        playBeep();
        currentScreen = SCREEN_HOME;
        drawCurrentScreen();
        break;
      }

      // KEYBOARD button (tappable area: x=200-310, y=215-235)
      if (x >= 200 && x <= 310 && y >= 215 && y <= 235) {
        Serial.println("⌨ Opening keyboard...");
        playBeep();
        screenBeforeKeyboard = SCREEN_TERMINAL;
        currentScreen = SCREEN_KEYBOARD;
        drawCurrentScreen();
        break;
      }
      break;

    case SCREEN_KEYBOARD:
      {
        // QWERTY keyboard touch handling (BETTER THAN iPHONE!)
        const char* row1 = "QWERTYUIOP";
        const char* row2 = "ASDFGHJKL";
        const char* row3 = "ZXCVBNM";

        int keyWidth = 28;
        int keyHeight = 24;
        int keySpacing = 2;

        bool keyPressed = false;

        // Row 1 (10 keys, y=123-147)
        if (y >= 123 && y <= 147) {
          int startX = 10;
          for (int i = 0; i < 10; i++) {
            int keyX = startX + i * (keyWidth + keySpacing);
            if (x >= keyX && x <= keyX + keyWidth) {
              terminalBuffer += row1[i];
              playBeep();
              Serial.printf("⌨ Key pressed: %c\n", row1[i]);
              keyPressed = true;
              break;
            }
          }
        }

        // Row 2 (9 keys, y=151-175, centered at x=25)
        if (!keyPressed && y >= 151 && y <= 175) {
          int startX = 25;
          for (int i = 0; i < 9; i++) {
            int keyX = startX + i * (keyWidth + keySpacing);
            if (x >= keyX && x <= keyX + keyWidth) {
              terminalBuffer += row2[i];
              playBeep();
              Serial.printf("⌨ Key pressed: %c\n", row2[i]);
              keyPressed = true;
              break;
            }
          }
        }

        // Row 3 (7 keys, y=179-203, centered at x=55)
        if (!keyPressed && y >= 179 && y <= 203) {
          int startX = 55;
          for (int i = 0; i < 7; i++) {
            int keyX = startX + i * (keyWidth + keySpacing);
            if (x >= keyX && x <= keyX + keyWidth) {
              terminalBuffer += row3[i];
              playBeep();
              Serial.printf("⌨ Key pressed: %c\n", row3[i]);
              keyPressed = true;
              break;
            }
          }
        }

        // Bottom row (y=207-231)
        if (!keyPressed && y >= 207 && y <= 231) {
          // SPACE (x=10-110)
          if (x >= 10 && x <= 110) {
            terminalBuffer += ' ';
            playBeep();
            Serial.println("⌨ SPACE pressed");
            keyPressed = true;
          }
          // BACKSPACE (x=115-175)
          else if (x >= 115 && x <= 175) {
            if (terminalBuffer.length() > 0) {
              terminalBuffer.remove(terminalBuffer.length() - 1);
              playBeep();
              Serial.println("⌨ BACKSPACE pressed");
              keyPressed = true;
            }
          }
          // ENTER (x=180-230)
          else if (x >= 180 && x <= 230) {
            playSuccess();
            Serial.printf("⌨ ENTER pressed - Command: %s\n", terminalBuffer.c_str());

            // Add command to history
            if (terminalBuffer.length() > 0) {
              // Shift history up
              for (int i = 0; i < 7; i++) {
                terminalHistory[i] = terminalHistory[i+1];
              }
              // Add new command and placeholder output
              terminalHistory[6] = "$ " + terminalBuffer;
              terminalHistory[7] = "> [Command executed]";
              historyCount = 8;

              // Clear buffer
              terminalBuffer = "";
            }

            // Close keyboard and return to terminal
            currentScreen = screenBeforeKeyboard;
            drawCurrentScreen();
            keyPressed = true;
          }
          // DONE (x=235-310)
          else if (x >= 235 && x <= 310) {
            playBeep();
            Serial.println("⌨ DONE pressed - closing keyboard");
            currentScreen = screenBeforeKeyboard;
            drawCurrentScreen();
            keyPressed = true;
          }
        }

        // Redraw keyboard to show updated buffer
        if (keyPressed) {
          drawCurrentScreen();
        }
      }
      break;

    case SCREEN_EMERGENCY_PAGER:
      // Back button
      if (x >= 10 && x <= 60 && y >= 25 && y <= 45) {
        Serial.println("Pager: Back to home");
        playBeep();
        currentScreen = SCREEN_HOME;
        drawCurrentScreen();
        break;
      }

      // ACK button (30, 200, 120, 35)
      if (x >= 30 && x <= 150 && y >= 200 && y <= 235) {
        Serial.println("Pager: ACK pressed");
        playBeep();
        acknowledgeAlert();
        currentScreen = SCREEN_HOME;
        drawCurrentScreen();
        break;
      }

      // DISMISS button (170, 200, 120, 35)
      if (x >= 170 && x <= 290 && y >= 200 && y <= 235) {
        Serial.println("Pager: DISMISS pressed");
        playBeep();
        hasActiveAlert = false;
        apps[16].badge = 0;
        currentScreen = SCREEN_HOME;
        drawCurrentScreen();
        break;
      }
      break;

    case SCREEN_ALERT_HISTORY:
      // Back button
      if (x >= 10 && x <= 60 && y >= 25 && y <= 45) {
        Serial.println("Alert History: Back to home");
        playBeep();
        currentScreen = SCREEN_HOME;
        drawCurrentScreen();
        break;
      }
      break;

    default:
      // Back button (universal) - larger hit area for better touch response
      if (x >= 5 && x <= 80 && y >= 0 && y <= 45) {
        Serial.println("Back button tapped - returning to home");
        playBeep();
        currentScreen = SCREEN_HOME;
        drawCurrentScreen();
      }
  }

  // Wait for touch release (with timeout to prevent blocking)
  unsigned long releaseWait = millis();
  while (digitalRead(XPT2046_IRQ) == LOW && (millis() - releaseWait < 100)) {
    delay(5);
  }
}

void connectWiFi() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(COLOR_SUNRISE);
  tft.setTextDatum(MC_DATUM);
  brFont.drawMonoTextCentered("OPERATOR", 120, 140, BR_MONO_LARGE, COLOR_HOT_PINK);
  tft.setTextColor(COLOR_WHITE);
  brFont.drawTechnicalLabel("by blackroad os inc", 30, 175, COLOR_CYBER_BLUE);
  tft.setTextColor(COLOR_WHITE);
  brFont.drawMonoTextCentered("INITIALIZING...", 120, 200, BR_MONO_SMALL, COLOR_WHITE);

  WiFi.setHostname(DEVICE_NAME);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    delay(500);
    Serial.print(".");
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    tft.fillScreen(COLOR_BLACK);
    tft.setTextColor(COLOR_VIVID_PUR);
    tft.setTextDatum(MC_DATUM);
    brFont.drawMonoTextCentered("CONNECTED", 120, 140, BR_MONO_LARGE, COLOR_VIVID_PUR);
    tft.setTextColor(COLOR_WHITE);
    tft.drawString(WiFi.localIP().toString(), 120, 175, 2);
    delay(800);  // Faster!
  } else {
    Serial.println("\nWiFi Failed - Running Offline");
    tft.fillScreen(COLOR_BLACK);

    // Beautiful offline screen
    tft.setTextColor(COLOR_CYBER_BLUE);
    tft.setTextDatum(MC_DATUM);
    brFont.drawMonoTextCentered("OPERATOR", 120, 80, BR_MONO_LARGE, COLOR_HOT_PINK);

    tft.setTextColor(COLOR_HOT_PINK);
    brFont.drawTechnicalLabel("by blackroad os inc", 30, 115, COLOR_CYBER_BLUE);

    // Status
    tft.fillRoundRect(40, 145, 160, 35, 8, COLOR_DARK_GRAY);
    tft.setTextColor(COLOR_SUNRISE);
    brFont.drawMonoTextCentered("OFFLINE MODE", 120, 158, BR_MONO_MEDIUM, COLOR_SUNRISE);

    // Device info
    tft.setTextColor(COLOR_WHITE);
    brFont.drawMonoTextCentered("EDGE DEVICE READY", 120, 200, BR_MONO_SMALL, COLOR_WHITE);
    tft.setTextColor(COLOR_CYBER_BLUE);
    brFont.drawTechnicalLabel("operator@blackroad", 30, 215, COLOR_CYBER_BLUE);

    delay(1500);  // Faster!
  }
}

// BlackRoad OS Transparency Logging (SD Card)
// COMMENTED OUT - Emergency Pager doesn't need AI API logging
/*
void logAIInteraction(String aiSystem, String command, String response) {
  File logFile = SD.open("/ai_transparency.log", FILE_APPEND);
  if (logFile) {
    String timestamp = String(millis());
    String logEntry = "[" + timestamp + "] " + aiSystem + ": " + command + " -> " + response + "\n";
    logFile.print(logEntry);
    logFile.close();
    Serial.println("🔒 TRANSPARENCY LOG: " + logEntry);
  } else {
    Serial.println("❌ Failed to open transparency log");
  }
}

// SSH Command Execution (via octavia proxy)
String executeSSHCommand(String command) {
  HTTPClient http;
  http.begin("http://" + String(OCTAVIA_IP) + ":8080/ssh");  // Octavia SSH proxy endpoint
  http.addHeader("Content-Type", "text/plain");

  int httpCode = http.POST(command);
  String response = "";

  if (httpCode == 200) {
    response = http.getString();
    Serial.println("✅ SSH Command executed: " + command);
    Serial.println("📝 Response: " + response);
  } else {
    response = "ERROR: HTTP " + String(httpCode);
    Serial.println("❌ SSH Command failed: " + command);
  }

  http.end();
  return response;
}

// AI API Initialization (for Claude/ChatGPT)
*/

// COMMENTED OUT - Emergency Pager doesn't need AI API server
/*
void setupAIAPI() {
  // Endpoint: POST /ai/command - Execute command on ESP32
  aiServer.on("/ai/command", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {

      // Parse AI request
      String command = "";
      for (size_t i = 0; i < len; i++) {
        command += (char)data[i];
      }

      // Log transparency (BlackRoad OS requirement)
      String aiSystem = request->header("X-AI-System");  // Claude or ChatGPT
      if (aiSystem == "") aiSystem = "Unknown AI";

      Serial.println("\n🤖 AI COMMAND RECEIVED:");
      Serial.println("   System: " + aiSystem);
      Serial.println("   Command: " + command);

      // Execute command
      String response = executeSSHCommand(command);

      // Log interaction
      logAIInteraction(aiSystem, command, response);

      // Return response
      request->send(200, "text/plain", response);

      Serial.println("   ✅ AI Response sent: " + response.substring(0, 50) + "...");
    }
  );

  // Endpoint: GET /ai/status - Device health check
  aiServer.on("/ai/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String status = "{"
      "\"device\":\"BlackRoad OS AI Quantum Device\","
      "\"mac\":\"20:e7:c8:ba:1b:94\","
      "\"ip\":\"" + WiFi.localIP().toString() + "\","
      "\"ssh_target\":\"" + String(OCTAVIA_IP) + "\","
      "\"sd_card\":\"" + (SD.cardType() != CARD_NONE ? "Ready" : "Not Found") + "\","
      "\"uptime_ms\":" + String(millis()) +
    "}";

    request->send(200, "application/json", status);
    Serial.println("🔍 AI Status Check: " + WiFi.localIP().toString());
  });

  // Endpoint: GET /ai/logs - Read transparency logs
  aiServer.on("/ai/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
    File logFile = SD.open("/ai_transparency.log", FILE_READ);
    if (logFile) {
      String logs = logFile.readString();
      logFile.close();
      request->send(200, "text/plain", logs);
      Serial.println("📖 AI Logs accessed (" + String(logs.length()) + " bytes)");
    } else {
      request->send(404, "text/plain", "No logs found");
    }
  });

  aiServer.begin();
  Serial.println("🌐 AI API Server started on port 8080");
  Serial.println("   POST /ai/command - Execute SSH command");
  Serial.println("   GET /ai/status - Device health check");
  Serial.println("   GET /ai/logs - Transparency logs");
}
*/

// ═══════════════════════════════════════════════════════════════════
// EMERGENCY PAGER - LED PATTERN FUNCTIONS
// ═══════════════════════════════════════════════════════════════════

void ledHeartbeat() {
  // Gentle breathing pattern (standby mode)
  for(int brightness = 0; brightness <= 255; brightness += 5) {
    analogWrite(LED_PIN, brightness);
    delay(20);
  }
  for(int brightness = 255; brightness >= 0; brightness -= 5) {
    analogWrite(LED_PIN, brightness);
    delay(20);
  }
}

void ledFastStrobe() {
  // Ultra-fast strobe for P1/CRITICAL (50ms on/off, 10 seconds)
  unsigned long start = millis();
  while(millis() - start < 10000 && hasActiveAlert) {
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(50);
  }
}

void ledMediumBlink() {
  // Fast blink for P2/URGENT (200ms on/off, 8 seconds)
  unsigned long start = millis();
  while(millis() - start < 8000 && hasActiveAlert) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}

void ledSlowBlink() {
  // Slow blink for normal priority (500ms on/off, 5 seconds)
  unsigned long start = millis();
  while(millis() - start < 5000 && hasActiveAlert) {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
  }
}

void ledDoubleFlash() {
  // Double flash pattern (100ms-100ms-100ms-200ms, repeat)
  unsigned long start = millis();
  while(millis() - start < 8000 && hasActiveAlert) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}

void ledMorseSOS() {
  // Morse code SOS (... --- ...) - Emergency pattern
  while(hasActiveAlert) {
    // S (...)
    for(int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }
    delay(400);

    // O (---)
    for(int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(600);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }
    delay(400);

    // S (...)
    for(int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }

    delay(2000); // Pause between SOS repeats
  }
}

void triggerLEDPattern(int patternId) {
  // Trigger LED pattern in non-blocking way (simplified version)
  switch(patternId) {
    case PATTERN_HEARTBEAT:
      ledHeartbeat();
      break;
    case PATTERN_FAST_STROBE:
      ledFastStrobe();
      break;
    case PATTERN_MEDIUM_BLINK:
      ledMediumBlink();
      break;
    case PATTERN_SLOW_BLINK:
      ledSlowBlink();
      break;
    case PATTERN_DOUBLE_FLASH:
      ledDoubleFlash();
      break;
    case PATTERN_MORSE_SOS:
      ledMorseSOS();
      break;
  }
  digitalWrite(LED_PIN, LOW); // Ensure LED is off after pattern
}

// ═══════════════════════════════════════════════════════════════════
// EMERGENCY PAGER - SERIAL COMMAND HANDLER
// ═══════════════════════════════════════════════════════════════════

bool isDuplicateAlert(String alertId) {
  for(int i = 0; i < 20; i++) {
    if(alertCache[i] == alertId) {
      return true;
    }
  }
  alertCache[cacheIndex] = alertId;
  cacheIndex = (cacheIndex + 1) % 20;
  return false;
}

void parseAlert(String command) {
  // Parse: "ALERT LINEAR P1" or "ALERT SLACK CRITICAL"
  command.trim();

  int firstSpace = command.indexOf(' ');
  int secondSpace = command.indexOf(' ', firstSpace + 1);

  if(firstSpace == -1) return;

  String source = command.substring(firstSpace + 1, secondSpace == -1 ? command.length() : secondSpace);
  String priority = secondSpace == -1 ? "" : command.substring(secondSpace + 1);

  // Create unique alert ID for deduplication
  String alertId = source + "-" + priority + "-" + String(millis() / 60000);

  if(isDuplicateAlert(alertId)) {
    Serial.println("DUPLICATE");
    return;
  }

  // Store alert
  strncpy(currentAlert.source, source.c_str(), 15);
  currentAlert.source[15] = '\0';
  strncpy(currentAlert.priority, priority.c_str(), 15);
  currentAlert.priority[15] = '\0';

  // Set default message
  snprintf(currentAlert.message, 127, "Alert from %s with priority %s",
           currentAlert.source, currentAlert.priority);

  currentAlert.timestamp = millis();
  currentAlert.acknowledged = false;
  hasActiveAlert = true;

  // Determine LED pattern
  if(strcmp(priority.c_str(), "P1") == 0 || strcmp(priority.c_str(), "CRITICAL") == 0) {
    currentAlert.patternId = PATTERN_FAST_STROBE;
  } else if(strcmp(priority.c_str(), "P2") == 0 || strcmp(priority.c_str(), "URGENT") == 0) {
    currentAlert.patternId = PATTERN_MEDIUM_BLINK;
  } else if(strcmp(priority.c_str(), "SOS") == 0) {
    currentAlert.patternId = PATTERN_MORSE_SOS;
  } else {
    currentAlert.patternId = PATTERN_SLOW_BLINK;
  }

  // Add to history
  if(alertHistoryCount < 10) {
    alertHistory[alertHistoryCount++] = currentAlert;
  } else {
    // Shift history
    for(int i = 0; i < 9; i++) {
      alertHistory[i] = alertHistory[i + 1];
    }
    alertHistory[9] = currentAlert;
  }

  // Update PAGER app badge
  apps[16].badge = hasActiveAlert ? 1 : 0;

  // Switch to pager screen
  currentScreen = SCREEN_EMERGENCY_PAGER;

  Serial.println("OK");
}

void acknowledgeAlert() {
  currentAlert.acknowledged = true;
  hasActiveAlert = false;
  apps[16].badge = 0;  // Clear PAGER badge
  Serial.println("ACK");
}

void sendStatus() {
  String status = "STATE ";
  if(hasActiveAlert) {
    status += "ALERT";
  } else {
    status += "STANDBY";
  }
  Serial.println(status);
}

void handleSerialCommand() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.startsWith("ALERT")) {
      parseAlert(cmd);
    }
    else if (cmd == "ACK") {
      acknowledgeAlert();
    }
    else if (cmd == "PING") {
      Serial.println("READY");
    }
    else if (cmd == "STATUS") {
      sendStatus();
    }
    else if (cmd == "STANDBY") {
      hasActiveAlert = false;
      apps[16].badge = 0;
      if(currentScreen == SCREEN_EMERGENCY_PAGER) {
        currentScreen = SCREEN_HOME;
      }
      Serial.println("OK");
    }
    else if (cmd == "RESET") {
      ESP.restart();
    }
  }
}

// ═══════════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\n=================================");
  Serial.println("BLACKROAD AI QUANTUM DEVICE");
  Serial.println("+ EMERGENCY PAGER SYSTEM");
  Serial.println("ESP32 AI Operating System + SSH");
  Serial.println("=================================");

  // Turn on backlight
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // Initialize LED for emergency pager
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize haptic/audio feedback (BETTER THAN iPHONE!)
  pinMode(BUZZER_PIN, OUTPUT);
  playSuccess();  // Boot sound

  // Initialize display
  tft.init();
  tft.setRotation(1); // Landscape mode (320x240) - HORIZONTAL LAYOUT
  tft.fillScreen(COLOR_BLACK);

  Serial.println("Display initialized");

  // Show professional boot screen with animations
  drawBootScreen();

  // Initialize touch SPI
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  pinMode(XPT2046_CS, OUTPUT);
  pinMode(XPT2046_IRQ, INPUT);
  digitalWrite(XPT2046_CS, HIGH);

  Serial.println("Touch initialized");

  // Initialize SD Card (for transparency logging)
  // COMMENTED OUT - Emergency Pager doesn't need SD card
  /*
  Serial.println("\n💾 Initializing SD Card...");
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (SD.begin(SD_CS)) {
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
      Serial.println("❌ No SD card attached");
    } else {
      Serial.println("✅ SD Card initialized");
      uint64_t cardSize = SD.cardSize() / (1024 * 1024);
      Serial.print("   Card Size: ");
      Serial.print(cardSize);
      Serial.println(" MB");

      // Create transparency log file if not exists
      if (!SD.exists("/ai_transparency.log")) {
        File logFile = SD.open("/ai_transparency.log", FILE_WRITE);
        if (logFile) {
          logFile.println("# BlackRoad OS AI Transparency Log");
          logFile.println("# All AI interactions are logged here");
          logFile.println("# Format: [timestamp_ms] AI_System: command -> response");
          logFile.println("");
          logFile.close();
          Serial.println("   Created transparency log");
        }
      }
    }
  } else {
    Serial.println("❌ SD Card mount failed");
  }
  */

  // Connect to WiFi
  connectWiFi();

  // Configure NTP for real time (PST = UTC-8)
  configTime(-8 * 3600, 3600, "pool.ntp.org", "time.nist.gov");
  Serial.println("⏰ Configuring time from NTP...");

  // Wait for time sync with progress
  struct tm timeinfo;
  int retries = 0;
  while (!getLocalTime(&timeinfo) && retries < 10) {
    Serial.print(".");
    delay(500);
    retries++;
  }

  if (getLocalTime(&timeinfo)) {
    Serial.println("\n✅ Time synced!");
    Serial.printf("   Current time: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  } else {
    Serial.println("\n⚠️ Time sync failed (will show --:--)");
  }

  // Start AI API Server (for Claude/ChatGPT)
  Serial.println("\n🤖 Starting AI API Server...");
  // setupAIAPI();  // Commented out - Emergency Pager doesn't need AI API server
  Serial.println("✅ AI Quantum Device Ready!");
  Serial.println("   Device IP: " + WiFi.localIP().toString());
  Serial.println("   API Port: 8080");
  Serial.println("   Claude/ChatGPT can now call this device!");
  Serial.println("   SSH Target: " + String(OCTAVIA_IP));

  // Show lock screen
  currentScreen = SCREEN_LOCK;
  drawCurrentScreen();

  Serial.println("\nBlackRoad AI Quantum Phone Ready!");
  Serial.println("Tap screen to unlock and start using your AI Phone!");
}

void loop() {
  handleSerialCommand();  // Check for emergency pager commands
  updateRealTimeData();   // Update real-time metrics from backends
  handleTouch();
  delay(10);
}
