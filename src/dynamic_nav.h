#ifndef DYNAMIC_NAV_H
#define DYNAMIC_NAV_H

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include "secrets.h"

// Forward declaration for sovereign_stack.h function
void updateStackHealth();

/*
 * ═══════════════════════════════════════════════════════════════════════
 * BLACKROAD DYNAMIC NAVIGATION SYSTEM
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Pulls real-time data from:
 * - Tailscale mesh network (100.64.0.0/10)
 * - BlackRoad CRM API (crm.blackroad.io)
 * - HuggingFace inference endpoints
 * - Genesis hashlist verification
 *
 * Navigation becomes REACTIVE to infrastructure state!
 */

// ─────────────────────────────────────────────────────────────────────
// API ENDPOINTS
// ─────────────────────────────────────────────────────────────────────
#define TAILSCALE_API_URL "http://100.100.100.100:41641/api/v2"
#define CRM_API_URL "https://crm.blackroad.io/api"
#define HF_API_URL "https://hf.blackroad.io"
#define MESH_STATUS_URL "http://lucidia.blackroad.network:8080/mesh/status"

// ─────────────────────────────────────────────────────────────────────
// DATA STRUCTURES
// ─────────────────────────────────────────────────────────────────────

struct MeshNode {
  String name;           // "lucidia", "octavia", "alice"
  String ip;             // "100.64.0.1"
  String hostname;       // "lucidia.blackroad.network"
  bool online;
  int latency;           // milliseconds
  float bandwidth;       // MB/s
  String status;         // "active", "idle", "offline"
  unsigned long lastSeen;
};

struct CRMMetrics {
  int totalContacts;
  int hotLeads;
  int openDeals;
  float pipelineValue;
  int activity24h;
  String topLead;        // Contact name
  int topLeadScore;
};

struct AIMetrics {
  String modelName;
  String status;         // "running", "idle", "loading"
  int requestsToday;
  float avgLatency;      // milliseconds
  int tokensGenerated;
  float gpuUtil;         // percentage
  String lastInference;
};

struct HotLead {
  char name[32];          // "John Doe"
  char company[32];       // "Acme Corp"
  char email[64];         // "john@acme.com"
  int score;              // Lead score (0-100)
  char temperature[16];   // "burning", "hot", "warm"
  int opens;              // Email opens
  int clicks;             // Link clicks
  char lastActivity[32];  // "2h ago", "1d ago"
  char stage[24];         // "Ready", "Evaluating"
  bool hasReplied;
};

struct NavigationState {
  int activeNodes;
  int hotLeads;
  int aiRequests;
  bool meshHealthy;
  bool crmHealthy;
  bool aiHealthy;
  unsigned long lastUpdate;
};

// ─────────────────────────────────────────────────────────────────────
// GLOBAL STATE
// ─────────────────────────────────────────────────────────────────────

MeshNode meshNodes[4];  // lucidia, octavia, alice, shellfish
int meshNodeCount = 0;

HotLead hotLeads[5];     // Top 5 hot leads
int hotLeadCount = 0;

CRMMetrics crmMetrics;
AIMetrics aiMetrics;
NavigationState navState;

// ─────────────────────────────────────────────────────────────────────
// STATIC DATA FALLBACKS (must be declared before fetch functions)
// ─────────────────────────────────────────────────────────────────────

bool initStaticMeshData() {
  meshNodeCount = 4;

  meshNodes[0] = {"lucidia", "100.66.235.47", "lucidia.blackroad.network", true, 12, 2.4, "active", millis()};
  meshNodes[1] = {"octavia", "100.64.0.2", "octavia.blackroad.network", true, 8, 5.1, "active", millis()};
  meshNodes[2] = {"alice", "100.64.0.3", "alice.blackroad.network", true, 15, 1.8, "idle", millis()};
  meshNodes[3] = {"shellfish", "100.64.0.4", "shellfish.blackroad.network", true, 24, 0.9, "active", millis()};

  navState.activeNodes = 4;
  navState.meshHealthy = false; // Static data, not real
  return false;
}

bool initStaticCRMData() {
  crmMetrics.totalContacts = 150;
  crmMetrics.hotLeads = 12;
  crmMetrics.openDeals = 8;
  crmMetrics.pipelineValue = 250000;
  crmMetrics.activity24h = 45;
  crmMetrics.topLead = "Acme Corp";
  crmMetrics.topLeadScore = 87;

  // Initialize static hot leads data
  hotLeadCount = 5;

  strncpy(hotLeads[0].name, "Sarah Chen", 31);
  strncpy(hotLeads[0].company, "TechVentures Inc", 31);
  strncpy(hotLeads[0].email, "sarah@techventures.io", 63);
  hotLeads[0].score = 94;
  strncpy(hotLeads[0].temperature, "burning", 15);
  hotLeads[0].opens = 8;
  hotLeads[0].clicks = 5;
  strncpy(hotLeads[0].lastActivity, "2h ago", 31);
  strncpy(hotLeads[0].stage, "Ready", 23);
  hotLeads[0].hasReplied = true;

  strncpy(hotLeads[1].name, "Marcus Webb", 31);
  strncpy(hotLeads[1].company, "DataFlow Systems", 31);
  strncpy(hotLeads[1].email, "marcus@dataflow.com", 63);
  hotLeads[1].score = 89;
  strncpy(hotLeads[1].temperature, "hot", 15);
  hotLeads[1].opens = 6;
  hotLeads[1].clicks = 4;
  strncpy(hotLeads[1].lastActivity, "5h ago", 31);
  strncpy(hotLeads[1].stage, "Evaluating", 23);
  hotLeads[1].hasReplied = false;

  strncpy(hotLeads[2].name, "Jennifer Lopez", 31);
  strncpy(hotLeads[2].company, "CloudScale Ltd", 31);
  strncpy(hotLeads[2].email, "jlo@cloudscale.com", 63);
  hotLeads[2].score = 85;
  strncpy(hotLeads[2].temperature, "hot", 15);
  hotLeads[2].opens = 5;
  hotLeads[2].clicks = 3;
  strncpy(hotLeads[2].lastActivity, "1d ago", 31);
  strncpy(hotLeads[2].stage, "Ready", 23);
  hotLeads[2].hasReplied = true;

  strncpy(hotLeads[3].name, "David Kim", 31);
  strncpy(hotLeads[3].company, "FinTech Partners", 31);
  strncpy(hotLeads[3].email, "david@fintechp.io", 63);
  hotLeads[3].score = 82;
  strncpy(hotLeads[3].temperature, "hot", 15);
  hotLeads[3].opens = 7;
  hotLeads[3].clicks = 2;
  strncpy(hotLeads[3].lastActivity, "3h ago", 31);
  strncpy(hotLeads[3].stage, "Curious", 23);
  hotLeads[3].hasReplied = false;

  strncpy(hotLeads[4].name, "Amanda Foster", 31);
  strncpy(hotLeads[4].company, "StartupLabs", 31);
  strncpy(hotLeads[4].email, "afoster@startuplabs.io", 63);
  hotLeads[4].score = 78;
  strncpy(hotLeads[4].temperature, "warm", 15);
  hotLeads[4].opens = 4;
  hotLeads[4].clicks = 3;
  strncpy(hotLeads[4].lastActivity, "6h ago", 31);
  strncpy(hotLeads[4].stage, "Evaluating", 23);
  hotLeads[4].hasReplied = true;

  navState.hotLeads = 12;
  navState.crmHealthy = false; // Static data
  return false;
}

bool initStaticAIData() {
  aiMetrics.modelName = "Lucidia-7B";
  aiMetrics.status = "idle";
  aiMetrics.requestsToday = 234;
  aiMetrics.avgLatency = 145.2;
  aiMetrics.tokensGenerated = 45678;
  aiMetrics.gpuUtil = 23.5;
  aiMetrics.lastInference = "2m ago";

  navState.aiRequests = 234;
  navState.aiHealthy = false; // Static data
  return false;
}

// ─────────────────────────────────────────────────────────────────────
// TAILSCALE MESH INTEGRATION
// ─────────────────────────────────────────────────────────────────────

bool fetchMeshStatus() {
  HTTPClient http;
  WiFiClient client;

  Serial.println("\n🌐 Fetching Tailscale mesh status...");

  // Try local Tailscale API first
  http.begin(client, MESH_STATUS_URL);
  http.setTimeout(3000);

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();

    // Parse JSON response
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      JsonArray nodes = doc["nodes"];
      meshNodeCount = 0;

      for (JsonObject node : nodes) {
        if (meshNodeCount >= 4) break;

        meshNodes[meshNodeCount].name = node["name"].as<String>();
        meshNodes[meshNodeCount].ip = node["ip"].as<String>();
        meshNodes[meshNodeCount].hostname = node["hostname"].as<String>();
        meshNodes[meshNodeCount].online = node["online"].as<bool>();
        meshNodes[meshNodeCount].latency = node["latency"].as<int>();
        meshNodes[meshNodeCount].bandwidth = node["bandwidth"].as<float>();
        meshNodes[meshNodeCount].status = node["status"].as<String>();
        meshNodes[meshNodeCount].lastSeen = millis();

        Serial.printf("  ✓ %s: %s (%dms)\n",
          meshNodes[meshNodeCount].name.c_str(),
          meshNodes[meshNodeCount].online ? "online" : "offline",
          meshNodes[meshNodeCount].latency
        );

        meshNodeCount++;
      }

      navState.activeNodes = meshNodeCount;
      navState.meshHealthy = true;
      http.end();
      return true;
    }
  }

  http.end();

  // Fallback: Use static mesh data
  Serial.println("  ⚠️  Using static mesh data");
  return initStaticMeshData();
}

// ─────────────────────────────────────────────────────────────────────
// CRM API INTEGRATION
// ─────────────────────────────────────────────────────────────────────

bool fetchCRMMetrics() {
  HTTPClient http;
  WiFiClient client;

  Serial.println("\n💼 Fetching CRM metrics...");

  http.begin(client, String(CRM_API_URL) + "/stats");
  http.addHeader("Authorization", "Bearer " + String(CRM_SECRET));
  http.setTimeout(5000);

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      crmMetrics.totalContacts = doc["total_contacts"] | 0;
      crmMetrics.hotLeads = doc["hot_leads"] | 0;
      crmMetrics.openDeals = doc["open_deals"] | 0;
      crmMetrics.pipelineValue = doc["pipeline_value"] | 0.0;
      crmMetrics.activity24h = doc["activity_24h"] | 0;

      Serial.printf("  ✓ Contacts: %d | Hot Leads: %d | Pipeline: $%.0fK\n",
        crmMetrics.totalContacts,
        crmMetrics.hotLeads,
        crmMetrics.pipelineValue / 1000.0
      );

      navState.hotLeads = crmMetrics.hotLeads;
      navState.crmHealthy = true;
      http.end();
      return true;
    }
  }

  http.end();

  // Fallback: Use static CRM data
  Serial.println("  ⚠️  Using static CRM data");
  return initStaticCRMData();
}

bool fetchHotLeads() {
  HTTPClient http;
  WiFiClient client;

  Serial.println("\n🔥 Fetching hot leads from CRM...");

  http.begin(client, String(CRM_API_URL) + "/views/hot-leads");
  http.addHeader("Authorization", "Bearer " + String(CRM_SECRET));
  http.setTimeout(5000);

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();

    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error && doc.containsKey("contacts")) {
      JsonArray contacts = doc["contacts"];
      hotLeadCount = 0;

      for (JsonObject contact : contacts) {
        if (hotLeadCount >= 5) break;  // Max 5 leads

        // Parse contact data
        const char* firstName = contact["first_name"] | "";
        const char* lastName = contact["last_name"] | "";
        const char* company = contact["company"] | "Unknown";
        const char* email = contact["email"] | "";
        int score = contact["lead_score"] | 0;
        const char* temp = contact["temperature"] | "warm";
        int opens = contact["email_opens"] | 0;
        int clicks = contact["email_clicks"] | 0;
        const char* stage = contact["stage"] | "Aware";
        bool replied = contact["has_replied"] | false;

        // Build full name
        char fullName[32];
        snprintf(fullName, 31, "%s %s", firstName, lastName);
        strncpy(hotLeads[hotLeadCount].name, fullName, 31);

        // Copy data
        strncpy(hotLeads[hotLeadCount].company, company, 31);
        strncpy(hotLeads[hotLeadCount].email, email, 63);
        hotLeads[hotLeadCount].score = score;
        strncpy(hotLeads[hotLeadCount].temperature, temp, 15);
        hotLeads[hotLeadCount].opens = opens;
        hotLeads[hotLeadCount].clicks = clicks;
        strncpy(hotLeads[hotLeadCount].stage, stage, 23);
        hotLeads[hotLeadCount].hasReplied = replied;

        // Calculate last activity (simplified)
        const char* updatedAt = contact["updated_at"] | "";
        strncpy(hotLeads[hotLeadCount].lastActivity, "Recent", 31);

        Serial.printf("  ✓ %s (%s) - Score: %d\n", fullName, company, score);

        hotLeadCount++;
      }

      Serial.printf("  ✓ Loaded %d hot leads from API\n", hotLeadCount);
      navState.crmHealthy = true;
      http.end();
      return true;
    }
  }

  http.end();

  // Fallback: Use static hot leads data
  Serial.println("  ⚠️  Using static hot leads data");
  return initStaticCRMData();
}

// ─────────────────────────────────────────────────────────────────────
// HUGGINGFACE AI INTEGRATION
// ─────────────────────────────────────────────────────────────────────

bool fetchAIMetrics() {
  HTTPClient http;
  WiFiClient client;

  Serial.println("\n🤖 Fetching AI metrics...");

  http.begin(client, String(HF_API_URL) + "/health");
  http.setTimeout(3000);

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      aiMetrics.modelName = doc["model"].as<String>();
      aiMetrics.status = doc["status"].as<String>();
      aiMetrics.requestsToday = doc["requests_today"] | 0;
      aiMetrics.avgLatency = doc["avg_latency"] | 0.0;
      aiMetrics.tokensGenerated = doc["tokens_generated"] | 0;
      aiMetrics.gpuUtil = doc["gpu_util"] | 0.0;

      Serial.printf("  ✓ Model: %s | Status: %s | Requests: %d\n",
        aiMetrics.modelName.c_str(),
        aiMetrics.status.c_str(),
        aiMetrics.requestsToday
      );

      navState.aiRequests = aiMetrics.requestsToday;
      navState.aiHealthy = true;
      http.end();
      return true;
    }
  }

  http.end();

  // Fallback: Use static AI data
  Serial.println("  ⚠️  Using static AI data");
  return initStaticAIData();
}

// ─────────────────────────────────────────────────────────────────────
// MASTER UPDATE FUNCTION
// ─────────────────────────────────────────────────────────────────────

void updateDynamicNavigation() {
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("🔄 UPDATING DYNAMIC NAVIGATION");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

  unsigned long startTime = millis();

  // Fetch all data sources in parallel (sequentially for now)
  bool meshOk = fetchMeshStatus();
  bool crmOk = fetchCRMMetrics();
  bool aiOk = fetchAIMetrics();

  // Update sovereign stack health based on navigation state
  updateStackHealth();

  navState.lastUpdate = millis();

  unsigned long elapsed = millis() - startTime;

  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("📊 NAVIGATION STATE SUMMARY");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.printf("Mesh:   %s (%d nodes)\n", meshOk ? "✅ LIVE" : "⚠️  STATIC", navState.activeNodes);
  Serial.printf("CRM:    %s (%d hot leads)\n", crmOk ? "✅ LIVE" : "⚠️  STATIC", navState.hotLeads);
  Serial.printf("AI:     %s (%d requests)\n", aiOk ? "✅ LIVE" : "⚠️  STATIC", navState.aiRequests);
  Serial.printf("Update: %lu ms\n", elapsed);
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
}

// ─────────────────────────────────────────────────────────────────────
// GENESIS HASH VERIFICATION
// ─────────────────────────────────────────────────────────────────────

struct GenesisHash {
  String hash;
  String entity;
  String type;
  bool verified;
};

GenesisHash genesisHashes[10];
int genesisHashCount = 0;

bool verifyGenesisHash(String hash, String entity) {
  // Compare against known hashes from blackroad_genesis_hashlist_v1.txt

  // Example hashes from genesis list:
  const char* knownHashes[][2] = {
    {"1031f308ae9ae6d34fe87e83867c1e5869c9fca7e35fdd5d0e8deb798e9c51be", "human:alexa"},
    {"c1cba42fd51be0b76c1f47ef2eda55fbcc1646b7b0a372d9563bb5db21ed1de1", "agent:cece"},
    {"2a402097b594033b74fcc1c7666d4c35f42e578537fea8c8a5cebd7330f591ba", "agent:lucidia"},
    {"496762c13853508f5a52806717465ac0221391c55bc0ccef5817a640ec9fc948", "agent:alice"},
    {"0ac14c4c4d90ccb78b73a8b8781d2103b1e019f01963413eaf632ffa4d29a69e", "domain:blackroad.io"},
  };

  for (int i = 0; i < 5; i++) {
    if (hash == knownHashes[i][0] && entity.indexOf(knownHashes[i][1]) >= 0) {
      Serial.printf("✅ Genesis hash verified: %s\n", entity.c_str());
      return true;
    }
  }

  Serial.printf("❌ Genesis hash FAILED: %s\n", entity.c_str());
  return false;
}

void initGenesisVerification() {
  Serial.println("\n🔐 Initializing Genesis Verification...");

  // Load critical hashes
  genesisHashCount = 5;

  genesisHashes[0] = {"1031f308ae9ae6d34fe87e83867c1e5869c9fca7e35fdd5d0e8deb798e9c51be", "alexa-amundson", "principal", true};
  genesisHashes[1] = {"c1cba42fd51be0b76c1f47ef2eda55fbcc1646b7b0a372d9563bb5db21ed1de1", "agent:cece", "core_agent", true};
  genesisHashes[2] = {"2a402097b594033b74fcc1c7666d4c35f42e578537fea8c8a5cebd7330f591ba", "agent:lucidia", "core_agent", true};
  genesisHashes[3] = {"0ac14c4c4d90ccb78b73a8b8781d2103b1e019f01963413eaf632ffa4d29a69e", "blackroad.io", "domain", true};
  genesisHashes[4] = {"bc05212d9e8ccab6d369cab5736089afe6275000756d354e7e7f41a3e9326bcf", "protocol:amundson:v0.1.0", "protocol", true};

  Serial.printf("  ✓ Loaded %d genesis hashes\n", genesisHashCount);
  Serial.println("  ✓ Verification system ready\n");
}

// ─────────────────────────────────────────────────────────────────────
// DYNAMIC NAVIGATION HELPERS
// ─────────────────────────────────────────────────────────────────────

String getNodeStatusColor(String status) {
  if (status == "active") return "COLOR_VIVID_PUR";
  if (status == "idle") return "COLOR_SUNRISE";
  if (status == "offline") return "COLOR_HOT_PINK";
  return "COLOR_WHITE";
}

String getHealthIndicator(bool healthy) {
  return healthy ? "🟢" : "🟡";
}

int getLeadUrgencyScore() {
  // Calculate urgency based on hot leads + recent activity
  return (navState.hotLeads * 10) + (crmMetrics.activity24h / 5);
}

bool shouldAlertOperator() {
  // Alert if:
  // - Hot leads > 15
  // - Mesh nodes offline
  // - AI system down
  return (navState.hotLeads > 15) ||
         (navState.activeNodes < 3) ||
         (!navState.aiHealthy);
}

#endif // DYNAMIC_NAV_H
