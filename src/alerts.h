#ifndef ALERTS_H
#define ALERTS_H

#include <HTTPClient.h>
#include <WiFiClient.h>
#include "secrets.h"

/*
 * ═══════════════════════════════════════════════════════════════════════
 * BLACKROAD ALERT SYSTEM
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Real-time alerting for critical infrastructure events:
 * - Hot leads threshold exceeded
 * - Mesh nodes offline
 * - Sovereignty score drop
 * - AI system failure
 * - CRM pipeline milestones
 *
 * Sends alerts to:
 * - Discord webhook
 * - Slack webhook
 * - Email (via Cloudflare worker)
 * - Local buzzer/LED
 */

// ─────────────────────────────────────────────────────────────────────
// ALERT CONFIGURATION
// ─────────────────────────────────────────────────────────────────────

#define ALERT_HOT_LEADS_THRESHOLD 15        // Alert when hot leads > 15
#define ALERT_SOVEREIGNTY_THRESHOLD 60.0    // Alert when sovereignty < 60%
#define ALERT_MESH_NODES_THRESHOLD 2        // Alert when active nodes < 2
#define ALERT_COOLDOWN_MS 300000            // 5 minutes between same alert

// ─────────────────────────────────────────────────────────────────────
// ALERT TYPES
// ─────────────────────────────────────────────────────────────────────

enum AlertLevel {
  ALERT_INFO,      // Informational (green)
  ALERT_WARNING,   // Warning (amber)
  ALERT_CRITICAL   // Critical (red)
};

enum AlertType {
  ALERT_HOT_LEADS,
  ALERT_SOVEREIGNTY,
  ALERT_MESH_DOWN,
  ALERT_AI_DOWN,
  ALERT_CRM_DOWN,
  ALERT_PIPELINE_MILESTONE
};

struct InfraAlert {
  AlertType type;
  AlertLevel level;
  char title[64];
  char message[128];
  unsigned long timestamp;
  bool sent;
};

// ─────────────────────────────────────────────────────────────────────
// ALERT HISTORY
// ─────────────────────────────────────────────────────────────────────

InfraAlert infraAlertHistory[10];
int infraAlertHistoryCount = 0;
unsigned long lastAlertTime[10] = {0};  // Cooldown tracker

// ─────────────────────────────────────────────────────────────────────
// DISCORD WEBHOOK INTEGRATION
// ─────────────────────────────────────────────────────────────────────

bool sendDiscordAlert(String title, String message, AlertLevel level) {
  #ifdef DISCORD_WEBHOOK_URL
  HTTPClient http;
  WiFiClient client;

  http.begin(client, String(DISCORD_WEBHOOK_URL));
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);

  // Color based on alert level
  int color = 0x00FF00;  // Green
  if (level == ALERT_WARNING) color = 0xFFA500;   // Amber
  if (level == ALERT_CRITICAL) color = 0xFF0000;  // Red

  // Build Discord embed
  String payload = "{\"embeds\":[{";
  payload += "\"title\":\"" + title + "\",";
  payload += "\"description\":\"" + message + "\",";
  payload += "\"color\":" + String(color) + ",";
  payload += "\"footer\":{\"text\":\"BlackRoad CEO Hub\"},";
  payload += "\"timestamp\":\"" + String(millis()) + "\"";
  payload += "}]}";

  int httpCode = http.POST(payload);
  bool success = (httpCode == 200 || httpCode == 204);

  http.end();
  return success;
  #else
    return false;  // Not configured
  #endif
}

// ─────────────────────────────────────────────────────────────────────
// SLACK WEBHOOK INTEGRATION
// ─────────────────────────────────────────────────────────────────────

bool sendSlackAlert(String title, String message, AlertLevel level) {
  #ifdef SLACK_WEBHOOK_URL
  HTTPClient http;
  WiFiClient client;

  http.begin(client, String(SLACK_WEBHOOK_URL));
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);

  // Emoji based on level
  String emoji = ":information_source:";
  if (level == ALERT_WARNING) emoji = ":warning:";
  if (level == ALERT_CRITICAL) emoji = ":rotating_light:";

  String payload = "{";
  payload += "\"text\":\"" + emoji + " *" + title + "*\",";
  payload += "\"blocks\":[{";
  payload += "\"type\":\"section\",";
  payload += "\"text\":{\"type\":\"mrkdwn\",\"text\":\"" + message + "\"}";
  payload += "}]";
  payload += "}";

  int httpCode = http.POST(payload);
  bool success = (httpCode == 200);

  http.end();
  return success;
  #else
    return false;  // Not configured
  #endif
}

// ─────────────────────────────────────────────────────────────────────
// LOCAL ALERT (BUZZER + LED)
// ─────────────────────────────────────────────────────────────────────

void triggerLocalAlert(AlertLevel level) {
  #ifdef BUZZER_PIN
    if (level == ALERT_CRITICAL) {
      // Critical: 3 long beeps
      for (int i = 0; i < 3; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(500);
        digitalWrite(BUZZER_PIN, LOW);
        delay(200);
      }
    } else if (level == ALERT_WARNING) {
      // Warning: 2 short beeps
      for (int i = 0; i < 2; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
        delay(100);
      }
    }
  #endif

  #ifdef LED_PIN
    // Flash LED
    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
  #endif
}

// ─────────────────────────────────────────────────────────────────────
// ALERT DISPATCHER
// ─────────────────────────────────────────────────────────────────────

void sendAlert(AlertType type, AlertLevel level, String title, String message) {
  // Check cooldown
  if (millis() - lastAlertTime[type] < ALERT_COOLDOWN_MS) {
    return;  // Too soon since last alert of this type
  }

  Serial.println("\n🚨 ALERT: " + title);
  Serial.println("   " + message);

  // Store in history
  if (infraAlertHistoryCount < 10) {
    infraAlertHistory[infraAlertHistoryCount].type = type;
    infraAlertHistory[infraAlertHistoryCount].level = level;
    strncpy(infraAlertHistory[infraAlertHistoryCount].title, title.c_str(), 63);
    infraAlertHistory[infraAlertHistoryCount].title[63] = '\0';
    strncpy(infraAlertHistory[infraAlertHistoryCount].message, message.c_str(), 127);
    infraAlertHistory[infraAlertHistoryCount].message[127] = '\0';
    infraAlertHistory[infraAlertHistoryCount].timestamp = millis();
    infraAlertHistory[infraAlertHistoryCount].sent = false;
    infraAlertHistoryCount++;
  }

  // Send to external services
  bool discordSent = sendDiscordAlert(title, message, level);
  bool slackSent = sendSlackAlert(title, message, level);

  if (discordSent) Serial.println("   ✓ Discord notification sent");
  if (slackSent) Serial.println("   ✓ Slack notification sent");

  // Trigger local alert
  triggerLocalAlert(level);

  // Update cooldown
  lastAlertTime[type] = millis();

  // Mark as sent
  if (infraAlertHistoryCount > 0) {
    infraAlertHistory[infraAlertHistoryCount - 1].sent = true;
  }
}

// ─────────────────────────────────────────────────────────────────────
// ALERT CONDITION CHECKERS
// ─────────────────────────────────────────────────────────────────────

void checkHotLeadsAlert() {
  if (navState.crmHealthy && crmMetrics.hotLeads > ALERT_HOT_LEADS_THRESHOLD) {
    String message = String(crmMetrics.hotLeads) + " hot leads detected! ";
    message += "Pipeline value: $" + String(crmMetrics.pipelineValue / 1000) + "K. ";
    message += "Action required: Follow up on top leads.";

    sendAlert(
      ALERT_HOT_LEADS,
      ALERT_WARNING,
      "🔥 Hot Leads Threshold Exceeded",
      message
    );
  }
}

void checkSovereigntyAlert() {
  SovereigntyMetrics metrics = getSovereigntyMetrics();

  if (metrics.sovereigntyScore < ALERT_SOVEREIGNTY_THRESHOLD) {
    String message = "Sovereignty score dropped to " + String((int)metrics.sovereigntyScore) + "%. ";
    message += "Status: " + getSovereigntyStatus() + ". ";
    message += String(metrics.totalComponents - metrics.sovereignComponents) + " external dependencies detected.";

    sendAlert(
      ALERT_SOVEREIGNTY,
      ALERT_CRITICAL,
      "⚠️ Sovereignty Score Critical",
      message
    );
  }
}

void checkMeshAlert() {
  if (navState.meshHealthy && navState.activeNodes < ALERT_MESH_NODES_THRESHOLD) {
    String message = "Only " + String(navState.activeNodes) + " mesh nodes active. ";
    message += "Expected: 4 nodes (lucidia, octavia, alice, shellfish). ";
    message += "Network redundancy compromised.";

    sendAlert(
      ALERT_MESH_DOWN,
      ALERT_CRITICAL,
      "🌐 Mesh Network Degraded",
      message
    );
  }
}

void checkSystemHealthAlert() {
  int downCount = 0;
  String downSystems = "";

  if (!navState.meshHealthy) {
    downCount++;
    downSystems += "Mesh Network, ";
  }
  if (!navState.crmHealthy) {
    downCount++;
    downSystems += "CRM API, ";
  }
  if (!navState.aiHealthy) {
    downCount++;
    downSystems += "AI Infrastructure, ";
  }

  if (downCount >= 2) {
    downSystems = downSystems.substring(0, downSystems.length() - 2);  // Remove trailing comma
    String message = String(downCount) + " critical systems offline: " + downSystems + ". ";
    message += "Infrastructure health degraded. Check system logs.";

    sendAlert(
      ALERT_AI_DOWN,  // Reuse alert type
      ALERT_CRITICAL,
      "🚨 Multiple Systems Offline",
      message
    );
  }
}

void checkPipelineMilestoneAlert() {
  if (navState.crmHealthy && crmMetrics.pipelineValue >= 1000000) {  // $1M
    String message = "Pipeline value: $" + String(crmMetrics.pipelineValue / 1000) + "K! ";
    message += String(crmMetrics.openDeals) + " open deals, ";
    message += String(crmMetrics.hotLeads) + " hot leads. ";
    message += "Milestone achievement unlocked! 🎉";

    sendAlert(
      ALERT_PIPELINE_MILESTONE,
      ALERT_INFO,
      "🎯 Pipeline Milestone Reached",
      message
    );
  }
}

// ─────────────────────────────────────────────────────────────────────
// MASTER ALERT CHECK
// ─────────────────────────────────────────────────────────────────────

void checkAllAlerts() {
  if (WiFi.status() != WL_CONNECTED) {
    return;  // Don't alert if offline
  }

  Serial.println("\n🔍 Checking alert conditions...");

  checkHotLeadsAlert();
  checkSovereigntyAlert();
  checkMeshAlert();
  checkSystemHealthAlert();
  checkPipelineMilestoneAlert();

  Serial.println("   ✓ Alert check complete\n");
}

// ─────────────────────────────────────────────────────────────────────
// ALERT HISTORY HELPERS
// ─────────────────────────────────────────────────────────────────────

int getInfraAlertCount() {
  return infraAlertHistoryCount;
}

InfraAlert getInfraAlert(int index) {
  if (index >= 0 && index < infraAlertHistoryCount) {
    return infraAlertHistory[index];
  }
  InfraAlert empty;
  empty.type = ALERT_HOT_LEADS;
  empty.level = ALERT_INFO;
  empty.title[0] = '\0';
  empty.message[0] = '\0';
  empty.timestamp = 0;
  empty.sent = false;
  return empty;
}

void clearInfraAlertHistory() {
  infraAlertHistoryCount = 0;
  for (int i = 0; i < 10; i++) {
    lastAlertTime[i] = 0;
  }
}

String getAlertLevelString(AlertLevel level) {
  switch (level) {
    case ALERT_INFO: return "INFO";
    case ALERT_WARNING: return "WARNING";
    case ALERT_CRITICAL: return "CRITICAL";
    default: return "UNKNOWN";
  }
}

uint16_t getAlertLevelColor(AlertLevel level) {
  switch (level) {
    case ALERT_INFO: return COLOR_CYBER_BLUE;
    case ALERT_WARNING: return COLOR_SUNRISE;
    case ALERT_CRITICAL: return COLOR_HOT_PINK;
    default: return COLOR_WHITE;
  }
}

#endif // ALERTS_H
