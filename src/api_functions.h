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

/*
 * ═══════════════════════════════════════════════════════════════════════
 * REAL API INTEGRATIONS - GitHub, Linear, Weather, Stripe
 * ═══════════════════════════════════════════════════════════════════════
 */

// Include ArduinoJson for parsing API responses
#include <ArduinoJson.h>

// API Response structure for standardized handling
struct APIResponse {
  bool success;
  int statusCode;
  String data;
  String error;
  unsigned long timestamp;
};

// Rate limiting tracker
struct RateLimitTracker {
  unsigned long lastRequest;
  int requestCount;
  unsigned long windowStart;
};

RateLimitTracker rateLimits[10];  // Track up to 10 different APIs

// Check if API request is within rate limit
bool checkRateLimit(int apiIndex, int maxRequests, unsigned long windowMs) {
  unsigned long now = millis();

  // Reset window if expired
  if (now - rateLimits[apiIndex].windowStart > windowMs) {
    rateLimits[apiIndex].windowStart = now;
    rateLimits[apiIndex].requestCount = 0;
  }

  // Check if under limit
  if (rateLimits[apiIndex].requestCount < maxRequests) {
    rateLimits[apiIndex].requestCount++;
    rateLimits[apiIndex].lastRequest = now;
    return true;
  }

  return false;
}

// Generic GET request with authentication
APIResponse apiGet(const char* url, const char* authHeader = nullptr, const char* authValue = nullptr) {
  APIResponse response;
  response.success = false;
  response.statusCode = 0;
  response.timestamp = millis();

  HTTPClient http;
  http.begin(url);
  http.setTimeout(10000);  // 10 second timeout

  if (authHeader && authValue) {
    http.addHeader(authHeader, authValue);
  }

  int httpCode = http.GET();
  response.statusCode = httpCode;

  if (httpCode > 0) {
    response.success = (httpCode == 200 || httpCode == 201);
    response.data = http.getString();

    if (!response.success) {
      response.error = "HTTP " + String(httpCode);
    }
  } else {
    response.error = "Connection failed: " + http.errorToString(httpCode);
  }

  http.end();
  return response;
}

/*
 * ───────────────────────────────────────────────────────────────────────
 * GITHUB API INTEGRATION
 * ───────────────────────────────────────────────────────────────────────
 */

struct GitHubStats {
  int totalRepos;
  int openPRs;
  int openIssues;
  int starsTotal;
  int forksTotal;
  char lastCommitMsg[150];
  char lastCommitAuthor[50];
  char lastCommitRepo[50];
  unsigned long lastUpdate;
};

GitHubStats githubData;

bool fetchGitHubStats(const char* token, const char* username) {
  if (!checkRateLimit(0, 60, 3600000)) {  // 60 requests per hour
    Serial.println("⏱️  GitHub API rate limit reached");
    return false;
  }

  Serial.println("📊 Fetching GitHub stats...");

  String url = "https://api.github.com/users/" + String(username) + "/repos?per_page=100&sort=updated";
  String authValue = "Bearer " + String(token);

  APIResponse response = apiGet(url.c_str(), "Authorization", authValue.c_str());

  if (!response.success) {
    Serial.printf("❌ GitHub API error: %s\n", response.error.c_str());
    return false;
  }

  // Parse JSON response
  DynamicJsonDocument doc(16384);
  DeserializationError error = deserializeJson(doc, response.data);

  if (error) {
    Serial.printf("❌ JSON parse error: %s\n", error.c_str());
    return false;
  }

  // Reset counters
  githubData.totalRepos = 0;
  githubData.starsTotal = 0;
  githubData.forksTotal = 0;

  // Process repositories
  JsonArray repos = doc.as<JsonArray>();
  for (JsonObject repo : repos) {
    githubData.totalRepos++;
    githubData.starsTotal += repo["stargazers_count"].as<int>();
    githubData.forksTotal += repo["forks_count"].as<int>();

    // Get most recent commit from first repo
    if (githubData.totalRepos == 1) {
      const char* repoName = repo["name"];
      strncpy(githubData.lastCommitRepo, repoName, 49);

      // Fetch latest commit for this repo
      String commitUrl = "https://api.github.com/repos/" + String(username) + "/" + String(repoName) + "/commits?per_page=1";
      APIResponse commitResponse = apiGet(commitUrl.c_str(), "Authorization", authValue.c_str());

      if (commitResponse.success) {
        DynamicJsonDocument commitDoc(4096);
        deserializeJson(commitDoc, commitResponse.data);
        JsonArray commits = commitDoc.as<JsonArray>();

        if (commits.size() > 0) {
          JsonObject commit = commits[0];
          const char* message = commit["commit"]["message"];
          const char* author = commit["commit"]["author"]["name"];

          strncpy(githubData.lastCommitMsg, message, 149);
          strncpy(githubData.lastCommitAuthor, author, 49);
        }
      }
    }
  }

  // Fetch open PRs and issues count (simplified - would need to check each repo)
  githubData.openPRs = 0;  // TODO: Implement per-repo PR checks
  githubData.openIssues = 0;  // TODO: Implement per-repo issue checks

  githubData.lastUpdate = millis();

  Serial.printf("✅ GitHub: %d repos, %d stars, %d forks\n",
                githubData.totalRepos, githubData.starsTotal, githubData.forksTotal);

  return true;
}

/*
 * ───────────────────────────────────────────────────────────────────────
 * FORWARD DECLARATIONS
 * ───────────────────────────────────────────────────────────────────────
 */

// API tracking function (defined later in file)
void trackAPIFetch(int index, bool success);

/*
 * ───────────────────────────────────────────────────────────────────────
 * CRYPTO API INTEGRATION (CoinGecko)
 * ───────────────────────────────────────────────────────────────────────
 */

struct CryptoPrice {
  float btcPrice;
  float ethPrice;
  float solPrice;
  float btc24hChange;
  float eth24hChange;
  float sol24hChange;
  unsigned long lastUpdate;
};

CryptoPrice cryptoData = {0, 0, 0, 0, 0, 0, 0};

bool fetchCryptoPrice() {
  if (!checkRateLimit(2, 50, 60000)) {  // 50 requests per minute (CoinGecko free tier)
    Serial.println("⏱️  Crypto API rate limit reached");
    return false;
  }

  Serial.println("💰 Fetching crypto prices...");

  // CoinGecko free API - no key needed!
  String url = "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin,ethereum,solana&vs_currencies=usd&include_24hr_change=true";

  APIResponse response = apiGet(url.c_str(), "", "");

  if (response.statusCode == 200) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, response.data);

    if (error) {
      Serial.printf("❌ Crypto JSON parse error: %s\n", error.c_str());
      trackAPIFetch(2, false);
      return false;
    }

    // Extract prices
    cryptoData.btcPrice = doc["bitcoin"]["usd"];
    cryptoData.ethPrice = doc["ethereum"]["usd"];
    cryptoData.solPrice = doc["solana"]["usd"];

    // Extract 24h price changes
    cryptoData.btc24hChange = doc["bitcoin"]["usd_24h_change"];
    cryptoData.eth24hChange = doc["ethereum"]["usd_24h_change"];
    cryptoData.sol24hChange = doc["solana"]["usd_24h_change"];

    cryptoData.lastUpdate = millis();

    Serial.printf("✅ Crypto: BTC $%.2f (%.2f%%), ETH $%.2f (%.2f%%), SOL $%.2f (%.2f%%)\n",
                  cryptoData.btcPrice, cryptoData.btc24hChange,
                  cryptoData.ethPrice, cryptoData.eth24hChange,
                  cryptoData.solPrice, cryptoData.sol24hChange);

    trackAPIFetch(2, true);
    return true;
  }

  Serial.printf("❌ Crypto API error: HTTP %d\n", response.statusCode);
  trackAPIFetch(2, false);
  return false;
}

/*
 * ───────────────────────────────────────────────────────────────────────
 * WEATHER API INTEGRATION (OpenWeatherMap)
 * ───────────────────────────────────────────────────────────────────────
 */

struct WeatherData {
  float temp;
  float feelsLike;
  int humidity;
  float windSpeed;
  char condition[50];
  char icon[10];
  int uvIndex;
  char forecast[3][50];  // 3-day forecast
  unsigned long lastUpdate;
};

WeatherData weatherData;

bool fetchWeather(const char* apiKey, const char* city) {
  if (!checkRateLimit(1, 100, 3600000)) {  // 100 requests per hour
    Serial.println("⏱️  Weather API rate limit reached");
    return false;
  }

  Serial.println("🌤️  Fetching weather data...");

  String url = "https://api.openweathermap.org/data/2.5/weather?q=" +
               String(city) + "&appid=" + String(apiKey) + "&units=imperial";

  APIResponse response = apiGet(url.c_str());

  if (!response.success) {
    Serial.printf("❌ Weather API error: %s\n", response.error.c_str());
    return false;
  }

  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, response.data);

  if (error) {
    Serial.printf("❌ JSON parse error: %s\n", error.c_str());
    return false;
  }

  weatherData.temp = doc["main"]["temp"];
  weatherData.feelsLike = doc["main"]["feels_like"];
  weatherData.humidity = doc["main"]["humidity"];
  weatherData.windSpeed = doc["wind"]["speed"];

  const char* condition = doc["weather"][0]["main"];
  const char* icon = doc["weather"][0]["icon"];

  strncpy(weatherData.condition, condition, 49);
  strncpy(weatherData.icon, icon, 9);

  weatherData.uvIndex = 0;  // Requires separate API call
  weatherData.lastUpdate = millis();

  Serial.printf("✅ Weather: %.1f°F, %s, %d%% humidity\n",
                weatherData.temp, weatherData.condition, weatherData.humidity);

  return true;
}

/*
 * ───────────────────────────────────────────────────────────────────────
 * STRIPE API INTEGRATION
 * ───────────────────────────────────────────────────────────────────────
 */

struct StripeMetrics {
  float revenueToday;
  float revenueWeek;
  float revenueMonth;
  int chargesCount;
  int customersCount;
  char recentCharges[3][100];
  unsigned long lastUpdate;
};

StripeMetrics stripeData;

bool fetchStripeMetrics(const char* apiKey) {
  if (!checkRateLimit(2, 100, 3600000)) {  // 100 requests per hour
    Serial.println("⏱️  Stripe API rate limit reached");
    return false;
  }

  Serial.println("💳 Fetching Stripe metrics...");

  String url = "https://api.stripe.com/v1/charges?limit=10";
  String authValue = "Bearer " + String(apiKey);

  APIResponse response = apiGet(url.c_str(), "Authorization", authValue.c_str());

  if (!response.success) {
    Serial.printf("❌ Stripe API error: %s\n", response.error.c_str());
    return false;
  }

  DynamicJsonDocument doc(8192);
  DeserializationError error = deserializeJson(doc, response.data);

  if (error) {
    Serial.printf("❌ JSON parse error: %s\n", error.c_str());
    return false;
  }

  stripeData.revenueToday = 0;
  stripeData.revenueWeek = 0;
  stripeData.revenueMonth = 0;
  stripeData.chargesCount = 0;

  JsonArray charges = doc["data"];
  for (JsonObject charge : charges) {
    if (charge["status"] == "succeeded") {
      float amount = charge["amount"].as<float>() / 100.0;  // Convert cents to dollars
      stripeData.revenueMonth += amount;
      stripeData.chargesCount++;

      // Store first 3 recent charges
      if (stripeData.chargesCount <= 3) {
        String chargeInfo = "$" + String(amount, 2) + " - " + String(charge["description"].as<const char*>());
        strncpy(stripeData.recentCharges[stripeData.chargesCount - 1], chargeInfo.c_str(), 99);
      }
    }
  }

  stripeData.lastUpdate = millis();

  Serial.printf("✅ Stripe: $%.2f revenue, %d charges\n",
                stripeData.revenueMonth, stripeData.chargesCount);

  return true;
}

/*
 * ───────────────────────────────────────────────────────────────────────
 * LINEAR API INTEGRATION (GraphQL)
 * ───────────────────────────────────────────────────────────────────────
 */

struct LinearTask {
  char id[50];
  char title[100];
  char state[20];
  int priority;
};

LinearTask linearTasks[10];
int linearTaskCount = 0;

bool fetchLinearTasks(const char* apiKey, const char* userEmail) {
  if (!checkRateLimit(3, 100, 3600000)) {  // 100 requests per hour
    Serial.println("⏱️  Linear API rate limit reached");
    return false;
  }

  Serial.println("📋 Fetching Linear tasks...");

  HTTPClient http;
  http.begin("https://api.linear.app/graphql");
  http.setTimeout(10000);
  http.addHeader("Authorization", apiKey);
  http.addHeader("Content-Type", "application/json");

  String query = "{\"query\":\"{ issues(filter: { assignee: { email: { eq: \\\"" +
                 String(userEmail) + "\\\" }}}) { nodes { id title state { name } priority }}}\"}";

  int httpCode = http.POST(query);

  if (httpCode != 200) {
    Serial.printf("❌ Linear API error: HTTP %d\n", httpCode);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  DynamicJsonDocument doc(8192);
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.printf("❌ JSON parse error: %s\n", error.c_str());
    return false;
  }

  linearTaskCount = 0;
  JsonArray issues = doc["data"]["issues"]["nodes"];

  for (JsonObject issue : issues) {
    if (linearTaskCount >= 10) break;

    strncpy(linearTasks[linearTaskCount].id, issue["id"], 49);
    strncpy(linearTasks[linearTaskCount].title, issue["title"], 99);
    strncpy(linearTasks[linearTaskCount].state, issue["state"]["name"], 19);
    linearTasks[linearTaskCount].priority = issue["priority"].as<int>();

    linearTaskCount++;
  }

  Serial.printf("✅ Linear: %d tasks loaded\n", linearTaskCount);

  return true;
}

/*
 * ═══════════════════════════════════════════════════════════════════════
 * PRIORITY 2: CRM DATA PERSISTENCE WITH SPIFFS
 * ═══════════════════════════════════════════════════════════════════════
 */

#include <SPIFFS.h>

struct Customer {
  char id[20];
  char name[100];
  char email[100];
  char phone[20];
  char company[100];
  float dealValue;
  char stage[20];  // Lead, Qualified, Proposal, Won, Lost
  unsigned long lastContact;
};

Customer customers[50];  // Support up to 50 customers
int customerCount = 0;

// Initialize SPIFFS
bool initCRMStorage() {
  if (!SPIFFS.begin(true)) {
    Serial.println("❌ SPIFFS mount failed");
    return false;
  }
  Serial.println("✅ SPIFFS mounted successfully");
  return true;
}

// Save CRM data to SPIFFS
bool saveCRMData() {
  File file = SPIFFS.open("/crm_data.json", "w");
  if (!file) {
    Serial.println("❌ Failed to open CRM file for writing");
    return false;
  }

  DynamicJsonDocument doc(32768);
  JsonArray array = doc.to<JsonArray>();

  for (int i = 0; i < customerCount; i++) {
    JsonObject obj = array.createNestedObject();
    obj["id"] = customers[i].id;
    obj["name"] = customers[i].name;
    obj["email"] = customers[i].email;
    obj["phone"] = customers[i].phone;
    obj["company"] = customers[i].company;
    obj["dealValue"] = customers[i].dealValue;
    obj["stage"] = customers[i].stage;
    obj["lastContact"] = customers[i].lastContact;
  }

  serializeJson(doc, file);
  file.close();

  Serial.printf("✅ CRM: Saved %d customers to SPIFFS\n", customerCount);
  return true;
}

// Load CRM data from SPIFFS
bool loadCRMData() {
  File file = SPIFFS.open("/crm_data.json", "r");
  if (!file) {
    Serial.println("⚠️  CRM file not found, starting fresh");
    return false;
  }

  DynamicJsonDocument doc(32768);
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.printf("❌ CRM JSON parse error: %s\n", error.c_str());
    return false;
  }

  customerCount = 0;
  JsonArray array = doc.as<JsonArray>();

  for (JsonObject obj : array) {
    if (customerCount >= 50) break;

    strncpy(customers[customerCount].id, obj["id"], 19);
    strncpy(customers[customerCount].name, obj["name"], 99);
    strncpy(customers[customerCount].email, obj["email"], 99);
    strncpy(customers[customerCount].phone, obj["phone"], 19);
    strncpy(customers[customerCount].company, obj["company"], 99);
    customers[customerCount].dealValue = obj["dealValue"];
    strncpy(customers[customerCount].stage, obj["stage"], 19);
    customers[customerCount].lastContact = obj["lastContact"];

    customerCount++;
  }

  Serial.printf("✅ CRM: Loaded %d customers from SPIFFS\n", customerCount);
  return true;
}

// Add new customer
bool addCustomer(const char* name, const char* email, const char* company, float dealValue) {
  if (customerCount >= 50) {
    Serial.println("❌ CRM: Customer limit reached");
    return false;
  }

  // Generate simple ID
  sprintf(customers[customerCount].id, "CUST%04d", customerCount + 1);
  strncpy(customers[customerCount].name, name, 99);
  strncpy(customers[customerCount].email, email, 99);
  strncpy(customers[customerCount].company, company, 99);
  customers[customerCount].dealValue = dealValue;
  strcpy(customers[customerCount].stage, "Lead");
  customers[customerCount].lastContact = millis();

  customerCount++;
  saveCRMData();

  Serial.printf("✅ CRM: Added %s (%s)\n", name, company);
  return true;
}

/*
 * ═══════════════════════════════════════════════════════════════════════
 * SELF-REFERENTIAL INTEGRATION TRACKING
 * Track this ESP32's own API integration progress!
 * ═══════════════════════════════════════════════════════════════════════
 */

struct IntegrationStatus {
  char name[30];
  bool enabled;
  bool configured;  // API keys added
  unsigned long lastFetch;
  int fetchCount;
  int errorCount;
  float successRate;
};

IntegrationStatus integrations[12] = {
  {"GitHub", false, false, 0, 0, 0, 0.0},
  {"Weather", false, false, 0, 0, 0, 0.0},
  {"Stripe", false, false, 0, 0, 0, 0.0},
  {"Linear", false, false, 0, 0, 0, 0.0},
  {"Calendar", false, false, 0, 0, 0, 0.0},
  {"Gmail", false, false, 0, 0, 0, 0.0},
  {"Slack", false, false, 0, 0, 0, 0.0},
  {"Discord", false, false, 0, 0, 0, 0.0},
  {"Notion", false, false, 0, 0, 0, 0.0},
  {"Twilio", false, false, 0, 0, 0, 0.0},
  {"Stocks", false, false, 0, 0, 0, 0.0},
  {"Crypto", false, false, 0, 0, 0, 0.0}
};

// Track API fetch attempt
void trackAPIFetch(int index, bool success) {
  integrations[index].lastFetch = millis();
  integrations[index].fetchCount++;

  if (!success) {
    integrations[index].errorCount++;
  }

  // Calculate success rate
  if (integrations[index].fetchCount > 0) {
    float successCount = integrations[index].fetchCount - integrations[index].errorCount;
    integrations[index].successRate = (successCount / integrations[index].fetchCount) * 100.0;
  }
}

// Get integration summary (self-awareness!)
String getIntegrationSummary() {
  int enabled = 0;
  int configured = 0;
  int active = 0;

  for (int i = 0; i < 12; i++) {
    if (integrations[i].enabled) enabled++;
    if (integrations[i].configured) configured++;
    if (integrations[i].fetchCount > 0) active++;
  }

  String summary = "APIs: " + String(active) + "/" + String(enabled) + " active, " +
                   String(configured) + " configured";
  return summary;
}

// Print detailed integration report
void printIntegrationReport() {
  Serial.println("\n╔══════════════════════════════════════════════════════════╗");
  Serial.println("║   ESP32 API INTEGRATION STATUS - SELF-DIAGNOSTIC        ║");
  Serial.println("╠══════════════════════════════════════════════════════════╣");

  for (int i = 0; i < 12; i++) {
    if (integrations[i].enabled || integrations[i].configured) {
      char status[10];
      if (integrations[i].fetchCount > 0) {
        strcpy(status, "🟢 ACTIVE");
      } else if (integrations[i].configured) {
        strcpy(status, "🟡 READY ");
      } else {
        strcpy(status, "⚪ SETUP ");
      }

      Serial.printf("║ %-15s %s   Fetches: %4d   Errors: %3d   ║\n",
                    integrations[i].name,
                    status,
                    integrations[i].fetchCount,
                    integrations[i].errorCount);

      if (integrations[i].fetchCount > 0) {
        Serial.printf("║                      Success Rate: %.1f%%                 ║\n",
                      integrations[i].successRate);
      }
    }
  }

  Serial.println("╚══════════════════════════════════════════════════════════╝");
  Serial.println(getIntegrationSummary());
}

// Save integration stats to SPIFFS (persistence!)
bool saveIntegrationStats() {
  File file = SPIFFS.open("/integration_stats.json", "w");
  if (!file) return false;

  DynamicJsonDocument doc(4096);
  JsonArray array = doc.to<JsonArray>();

  for (int i = 0; i < 12; i++) {
    JsonObject obj = array.createNestedObject();
    obj["name"] = integrations[i].name;
    obj["enabled"] = integrations[i].enabled;
    obj["configured"] = integrations[i].configured;
    obj["fetchCount"] = integrations[i].fetchCount;
    obj["errorCount"] = integrations[i].errorCount;
    obj["successRate"] = integrations[i].successRate;
  }

  serializeJson(doc, file);
  file.close();
  return true;
}

// Load integration stats from SPIFFS
bool loadIntegrationStats() {
  File file = SPIFFS.open("/integration_stats.json", "r");
  if (!file) return false;

  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) return false;

  JsonArray array = doc.as<JsonArray>();
  int i = 0;

  for (JsonObject obj : array) {
    if (i >= 12) break;

    integrations[i].enabled = obj["enabled"];
    integrations[i].configured = obj["configured"];
    integrations[i].fetchCount = obj["fetchCount"];
    integrations[i].errorCount = obj["errorCount"];
    integrations[i].successRate = obj["successRate"];
    i++;
  }

  Serial.println("✅ Loaded integration statistics from previous session");
  return true;
}

#endif // API_FUNCTIONS_H
