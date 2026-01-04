#ifndef API_FUNCTIONS_H
#define API_FUNCTIONS_H

#include <HTTPClient.h>
#include "api_config.h"

/*
 * API Health Check & Connection Functions
 * Real HTTP requests to production endpoints
 */

struct APIStatus {
  String name;
  String url;
  bool online;
  int responseTime;  // milliseconds
  int httpCode;
  String lastError;
};

// Global API status array
APIStatus apiStatuses[HEALTH_CHECK_COUNT];
unsigned long lastHealthCheck = 0;
#define HEALTH_CHECK_INTERVAL 60000  // Check every 60 seconds

// Initialize API status tracking
void initAPIStatus() {
  for (int i = 0; i < HEALTH_CHECK_COUNT; i++) {
    apiStatuses[i].name = String(HEALTH_CHECKS[i].name);
    apiStatuses[i].url = String(HEALTH_CHECKS[i].url);
    apiStatuses[i].online = false;
    apiStatuses[i].responseTime = 0;
    apiStatuses[i].httpCode = 0;
    apiStatuses[i].lastError = "";
  }
}

// Check single API endpoint
APIStatus checkAPIEndpoint(const char* name, const char* url, const char* method) {
  APIStatus status;
  status.name = String(name);
  status.url = String(url);
  status.online = false;
  status.responseTime = 0;
  status.httpCode = 0;
  status.lastError = "";

  HTTPClient http;
  unsigned long startTime = millis();

  Serial.printf("🔍 Checking API: %s (%s)\n", name, url);

  http.begin(url);
  http.setTimeout(5000);  // 5 second timeout

  int httpCode = -1;
  if (strcmp(method, "GET") == 0) {
    httpCode = http.GET();
  } else if (strcmp(method, "POST") == 0) {
    httpCode = http.POST("");
  }

  unsigned long responseTime = millis() - startTime;

  status.responseTime = responseTime;
  status.httpCode = httpCode;

  if (httpCode > 0) {
    status.online = (httpCode >= 200 && httpCode < 400);
    Serial.printf("   ✅ %s: %d (%dms)\n", name, httpCode, responseTime);
  } else {
    status.online = false;
    status.lastError = http.errorToString(httpCode);
    Serial.printf("   ❌ %s: %s\n", name, status.lastError.c_str());
  }

  http.end();
  return status;
}

// Check all API endpoints (health check)
void checkAllAPIs() {
  Serial.println("\n🔍 Running API Health Checks...");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

  for (int i = 0; i < HEALTH_CHECK_COUNT; i++) {
    // Skip APIs that require auth for now (will add auth tokens later)
    if (HEALTH_CHECKS[i].requiresAuth) {
      Serial.printf("⏭  Skipping %s (requires auth)\n", HEALTH_CHECKS[i].name);
      apiStatuses[i].name = String(HEALTH_CHECKS[i].name);
      apiStatuses[i].url = String(HEALTH_CHECKS[i].url);
      apiStatuses[i].online = false;
      apiStatuses[i].responseTime = 0;
      apiStatuses[i].httpCode = 0;
      apiStatuses[i].lastError = "Auth required";
      continue;
    }

    apiStatuses[i] = checkAPIEndpoint(
      HEALTH_CHECKS[i].name,
      HEALTH_CHECKS[i].url,
      HEALTH_CHECKS[i].method
    );

    delay(500);  // Rate limiting - don't hammer APIs
  }

  // Summary
  int onlineCount = 0;
  for (int i = 0; i < HEALTH_CHECK_COUNT; i++) {
    if (apiStatuses[i].online) onlineCount++;
  }

  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.printf("✅ Health Check Complete: %d/%d online\n", onlineCount, HEALTH_CHECK_COUNT);

  lastHealthCheck = millis();
}

// Get API status for display
String getAPIStatusSummary() {
  int total = HEALTH_CHECK_COUNT;
  int online = 0;
  int offline = 0;
  int auth_required = 0;

  for (int i = 0; i < total; i++) {
    if (apiStatuses[i].lastError == "Auth required") {
      auth_required++;
    } else if (apiStatuses[i].online) {
      online++;
    } else {
      offline++;
    }
  }

  return String(online) + "/" + String(total) + " online";
}

// Check if it's time for health check
bool shouldRunHealthCheck() {
  return (millis() - lastHealthCheck > HEALTH_CHECK_INTERVAL);
}

// Quick ping test to local Raspberry Pis
bool pingLocalServer(const char* ip, int port) {
  HTTPClient http;
  String url = "http://" + String(ip) + ":" + String(port) + "/health";

  http.begin(url);
  http.setTimeout(2000);  // 2 second timeout

  int httpCode = http.GET();
  http.end();

  return (httpCode == 200 || httpCode == 404);  // 404 means server is up but no health endpoint
}

// Test all local infrastructure
void testLocalInfrastructure() {
  Serial.println("\n🏠 Testing Local Infrastructure...");

  Serial.print("Octavia (192.168.4.38): ");
  if (pingLocalServer(OCTAVIA_IP, OCTAVIA_API_PORT)) {
    Serial.println("✅ ONLINE");
  } else {
    Serial.println("❌ OFFLINE");
  }

  Serial.print("Lucidia (192.168.4.99): ");
  if (pingLocalServer(LUCIDIA_IP, LUCIDIA_API_PORT)) {
    Serial.println("✅ ONLINE");
  } else {
    Serial.println("❌ OFFLINE");
  }

  Serial.print("BlackRoad Pi (192.168.4.64): ");
  if (pingLocalServer(BLACKROAD_PI_IP, 3000)) {
    Serial.println("✅ ONLINE");
  } else {
    Serial.println("❌ OFFLINE");
  }

  Serial.print("iPhone Koder (192.168.4.68): ");
  if (pingLocalServer(IPHONE_KODER_IP, IPHONE_API_PORT)) {
    Serial.println("✅ ONLINE");
  } else {
    Serial.println("❌ OFFLINE");
  }
}

#endif // API_FUNCTIONS_H
