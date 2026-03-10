/**
 * BlackRoad OS – Native Unit Tests
 *
 * Tests for pure business logic that does NOT depend on Arduino hardware.
 * Run with:  pio test -e native
 *
 * Covered modules / logic:
 *  1. API URL building          (api_config.h)
 *  2. Sovereignty score / status thresholds  (sovereign_stack.h)
 *  3. License forkability check (sovereign_stack.h)
 *  4. Uptime string formatting  (performance.h)
 *  5. WiFi quality string       (performance.h)
 *  6. Alert level strings       (alerts.h)
 *  7. AI recommendation engine  (main.cpp – getAIRecommendation)
 *  8. Rate-limit window logic   (api_functions.h)
 */

#include <unity.h>
#include <string>
#include <cstring>
#include <cstdint>
#include <cstdio>

// ═══════════════════════════════════════════════════════════════════
// Minimal Arduino-compatible String class required by the tested logic
// ═══════════════════════════════════════════════════════════════════

class String {
public:
  std::string _s;
  String() = default;
  explicit String(const char* s) : _s(s ? s : "") {}
  explicit String(int n)          : _s(std::to_string(n)) {}
  explicit String(unsigned int n) : _s(std::to_string(n)) {}
  explicit String(long n)         : _s(std::to_string(n)) {}
  explicit String(unsigned long n): _s(std::to_string(n)) {}
  explicit String(float n)        : _s(std::to_string(n)) {}
  explicit String(double n)       : _s(std::to_string(n)) {}
  // implicit construction from string literal via const-char*
  String(std::string s)           : _s(std::move(s)) {}

  const char* c_str()       const { return _s.c_str(); }
  size_t      length()      const { return _s.size(); }
  bool        endsWith(const char* suf) const {
    size_t sl = strlen(suf);
    if (_s.size() < sl) return false;
    return _s.substr(_s.size() - sl) == suf;
  }
  int indexOf(const String& o) const {
    auto p = _s.find(o._s);
    return p == std::string::npos ? -1 : (int)p;
  }
  int indexOf(const char* o) const {
    auto p = _s.find(o);
    return p == std::string::npos ? -1 : (int)p;
  }
  String substring(size_t from, size_t to = std::string::npos) const {
    return String(_s.substr(from, to == std::string::npos ? std::string::npos : to - from));
  }
  String& operator+=(const char*   s) { _s += s;   return *this; }
  String& operator+=(const String& s) { _s += s._s; return *this; }
  String  operator+ (const char*   s) const { return String(_s + s); }
  String  operator+ (const String& s) const { return String(_s + s._s); }
  bool    operator==(const String& o) const { return _s == o._s; }
  bool    operator!=(const String& o) const { return _s != o._s; }
  bool    operator==(const char*   o) const { return _s == o; }
  bool    operator!=(const char*   o) const { return _s != o; }
};

static inline String operator+(const char* lhs, const String& rhs) {
  return String(std::string(lhs) + rhs._s);
}

// ═══════════════════════════════════════════════════════════════════
// 1. API URL building  (mirrors api_config.h  buildAPIUrl)
// ═══════════════════════════════════════════════════════════════════

static String buildAPIUrl(const char* base, const char* endpoint) {
  String url(base);
  if (!url.endsWith("/") && endpoint[0] != '/') {
    url += "/";
  }
  url += endpoint;
  return url;
}

void test_buildAPIUrl_no_slash() {
  String url = buildAPIUrl("https://api.github.com", "users");
  TEST_ASSERT_EQUAL_STRING("https://api.github.com/users", url.c_str());
}

void test_buildAPIUrl_base_has_slash() {
  String url = buildAPIUrl("https://api.github.com/", "users");
  TEST_ASSERT_EQUAL_STRING("https://api.github.com/users", url.c_str());
}

void test_buildAPIUrl_endpoint_has_slash() {
  String url = buildAPIUrl("https://api.github.com", "/users");
  TEST_ASSERT_EQUAL_STRING("https://api.github.com/users", url.c_str());
}

void test_buildAPIUrl_both_have_slash() {
  String url = buildAPIUrl("https://api.github.com/", "/users");
  // base already has slash, endpoint starts with slash → "https://api.github.com//users"
  // The function only adds a slash when neither side has one; when endpoint has one it
  // will concatenate directly, so result is "https://api.github.com//users".
  // This tests the current behaviour (documenting the edge case).
  TEST_ASSERT_EQUAL_STRING("https://api.github.com//users", url.c_str());
}

void test_buildAPIUrl_empty_endpoint() {
  String url = buildAPIUrl("https://api.example.com", "");
  // empty endpoint doesn't start with '/' so a slash is added
  TEST_ASSERT_EQUAL_STRING("https://api.example.com/", url.c_str());
}

// ═══════════════════════════════════════════════════════════════════
// 2. Sovereignty status thresholds  (mirrors sovereign_stack.h)
// ═══════════════════════════════════════════════════════════════════

static String getSovereigntyStatus(float score) {
  if (score >= 80.0f) return String("SOVEREIGN");
  if (score >= 60.0f) return String("HYBRID");
  if (score >= 40.0f) return String("DEPENDENT");
  return String("CAPTURED");
}

void test_sovereignty_sovereign()  { TEST_ASSERT_EQUAL_STRING("SOVEREIGN",  getSovereigntyStatus(100.0f).c_str()); }
void test_sovereignty_hybrid()     { TEST_ASSERT_EQUAL_STRING("HYBRID",     getSovereigntyStatus(70.0f).c_str());  }
void test_sovereignty_dependent()  { TEST_ASSERT_EQUAL_STRING("DEPENDENT",  getSovereigntyStatus(50.0f).c_str());  }
void test_sovereignty_captured()   { TEST_ASSERT_EQUAL_STRING("CAPTURED",   getSovereigntyStatus(20.0f).c_str());  }
void test_sovereignty_boundary_80(){ TEST_ASSERT_EQUAL_STRING("SOVEREIGN",  getSovereigntyStatus(80.0f).c_str());  }
void test_sovereignty_boundary_60(){ TEST_ASSERT_EQUAL_STRING("HYBRID",     getSovereigntyStatus(60.0f).c_str());  }
void test_sovereignty_boundary_40(){ TEST_ASSERT_EQUAL_STRING("DEPENDENT",  getSovereigntyStatus(40.0f).c_str());  }
void test_sovereignty_zero()       { TEST_ASSERT_EQUAL_STRING("CAPTURED",   getSovereigntyStatus(0.0f).c_str());   }

// Sovereignty score calculation
static float sovereigntyScore(int sovereign, int total) {
  if (total == 0) return 0.0f;
  return (float)sovereign / total * 100.0f;
}

void test_sovereignty_score_full()    { TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, sovereigntyScore(10, 10)); }
void test_sovereignty_score_half()    { TEST_ASSERT_FLOAT_WITHIN(0.01f,  50.0f, sovereigntyScore(5, 10));  }
void test_sovereignty_score_none()    { TEST_ASSERT_FLOAT_WITHIN(0.01f,   0.0f, sovereigntyScore(0, 10));  }
void test_sovereignty_score_zero_div(){ TEST_ASSERT_FLOAT_WITHIN(0.01f,   0.0f, sovereigntyScore(0, 0));   }

// ═══════════════════════════════════════════════════════════════════
// 3. License forkability  (mirrors sovereign_stack.h isForkableLicense)
// ═══════════════════════════════════════════════════════════════════

static bool isForkableLicense(const String& license) {
  const char* forkable[] = {
    "MIT", "Apache-2.0", "BSD", "GPLv3", "AGPLv3", "MPL-2.0",
    "LGPLv3", "PostgreSQL", "ISC"
  };
  for (int i = 0; i < 9; i++) {
    if (license.indexOf(forkable[i]) >= 0) return true;
  }
  return false;
}

void test_license_mit()        { TEST_ASSERT_TRUE(isForkableLicense(String("MIT"))); }
void test_license_apache()     { TEST_ASSERT_TRUE(isForkableLicense(String("Apache-2.0"))); }
void test_license_bsd()        { TEST_ASSERT_TRUE(isForkableLicense(String("BSD-3"))); }
void test_license_gpl()        { TEST_ASSERT_TRUE(isForkableLicense(String("GPLv3"))); }
void test_license_postgresql() { TEST_ASSERT_TRUE(isForkableLicense(String("PostgreSQL"))); }
void test_license_proprietary(){ TEST_ASSERT_FALSE(isForkableLicense(String("Proprietary"))); }
void test_license_na()         { TEST_ASSERT_FALSE(isForkableLicense(String("N/A"))); }
void test_license_empty()      { TEST_ASSERT_FALSE(isForkableLicense(String(""))); }

// ═══════════════════════════════════════════════════════════════════
// 4. Uptime string formatting  (mirrors performance.h getUptimeString)
// ═══════════════════════════════════════════════════════════════════

static String getUptimeString(uint32_t seconds) {
  uint32_t minutes = seconds / 60;
  uint32_t hours   = minutes / 60;
  uint32_t days    = hours   / 24;

  char buf[32];
  if (days > 0) {
    snprintf(buf, sizeof(buf), "%ud %uh", (unsigned)days, (unsigned)(hours % 24));
  } else if (hours > 0) {
    snprintf(buf, sizeof(buf), "%uh %um", (unsigned)hours, (unsigned)(minutes % 60));
  } else if (minutes > 0) {
    snprintf(buf, sizeof(buf), "%um %us", (unsigned)minutes, (unsigned)(seconds % 60));
  } else {
    snprintf(buf, sizeof(buf), "%us", (unsigned)seconds);
  }
  return String(buf);
}

void test_uptime_seconds() { TEST_ASSERT_EQUAL_STRING("45s",      getUptimeString(45).c_str()); }
void test_uptime_minutes() { TEST_ASSERT_EQUAL_STRING("3m 15s",   getUptimeString(195).c_str()); }
void test_uptime_hours()   { TEST_ASSERT_EQUAL_STRING("2h 5m",    getUptimeString(7500).c_str()); }
void test_uptime_days()    { TEST_ASSERT_EQUAL_STRING("1d 1h",    getUptimeString(90000).c_str()); }
void test_uptime_zero()    { TEST_ASSERT_EQUAL_STRING("0s",       getUptimeString(0).c_str()); }
void test_uptime_exact_min(){ TEST_ASSERT_EQUAL_STRING("1m 0s",   getUptimeString(60).c_str()); }
void test_uptime_exact_hour(){ TEST_ASSERT_EQUAL_STRING("1h 0m",  getUptimeString(3600).c_str()); }

// ═══════════════════════════════════════════════════════════════════
// 5. WiFi quality string  (mirrors performance.h getWiFiQualityString)
// ═══════════════════════════════════════════════════════════════════

static String getWiFiQualityString(uint8_t quality) {
  if (quality >= 80) return String("Excellent");
  if (quality >= 60) return String("Good");
  if (quality >= 40) return String("Fair");
  if (quality >= 20) return String("Poor");
  return String("Offline");
}

void test_wifi_excellent() { TEST_ASSERT_EQUAL_STRING("Excellent", getWiFiQualityString(100).c_str()); }
void test_wifi_good()      { TEST_ASSERT_EQUAL_STRING("Good",      getWiFiQualityString(70).c_str());  }
void test_wifi_fair()      { TEST_ASSERT_EQUAL_STRING("Fair",      getWiFiQualityString(50).c_str());  }
void test_wifi_poor()      { TEST_ASSERT_EQUAL_STRING("Poor",      getWiFiQualityString(30).c_str());  }
void test_wifi_offline()   { TEST_ASSERT_EQUAL_STRING("Offline",   getWiFiQualityString(0).c_str());   }
void test_wifi_boundary_80(){ TEST_ASSERT_EQUAL_STRING("Excellent", getWiFiQualityString(80).c_str()); }
void test_wifi_boundary_60(){ TEST_ASSERT_EQUAL_STRING("Good",      getWiFiQualityString(60).c_str()); }

// ═══════════════════════════════════════════════════════════════════
// 6. Alert level strings  (mirrors alerts.h getAlertLevelString)
// ═══════════════════════════════════════════════════════════════════

enum AlertLevel { ALERT_INFO = 0, ALERT_WARNING, ALERT_CRITICAL };

static String getAlertLevelString(AlertLevel level) {
  switch (level) {
    case ALERT_INFO:     return String("INFO");
    case ALERT_WARNING:  return String("WARNING");
    case ALERT_CRITICAL: return String("CRITICAL");
    default:             return String("UNKNOWN");
  }
}

void test_alert_info()     { TEST_ASSERT_EQUAL_STRING("INFO",     getAlertLevelString(ALERT_INFO).c_str());     }
void test_alert_warning()  { TEST_ASSERT_EQUAL_STRING("WARNING",  getAlertLevelString(ALERT_WARNING).c_str());  }
void test_alert_critical() { TEST_ASSERT_EQUAL_STRING("CRITICAL", getAlertLevelString(ALERT_CRITICAL).c_str()); }

// ═══════════════════════════════════════════════════════════════════
// 7. AI recommendation engine  (mirrors main.cpp getAIRecommendation)
// ═══════════════════════════════════════════════════════════════════

static String getAIRecommendation(int approveCount, int rejectCount, int holdCount,
                                   int shipCount,    int criticalCount, int blockedCount) {
  int total = approveCount + rejectCount + holdCount + shipCount + blockedCount + criticalCount;
  if (total == 0) return String("Ready for decisions");

  int approve_ratio  = (approveCount  * 100) / (total + 1);
  int critical_ratio = (criticalCount * 100) / (total + 1);

  if (critical_ratio > 30) return String("HIGH URGENCY: Review critical items");
  if (approve_ratio  > 70) return String("SHIP READY: Approved items ready to deploy");
  if (blockedCount > approveCount) return String("BLOCKED: Clear blockers before proceeding");
  if (shipCount > 0 && holdCount > 0) return String("MIXED: Review held items, ship approved");
  if (holdCount > approveCount) return String("REVIEW: Many held items need decisions");
  return String("MONITOR: Keep reviewing incoming items");
}

void test_ai_no_actions()    { TEST_ASSERT_EQUAL_STRING("Ready for decisions",                    getAIRecommendation(0,0,0,0,0,0).c_str()); }
void test_ai_high_critical() { TEST_ASSERT_EQUAL_STRING("HIGH URGENCY: Review critical items",    getAIRecommendation(1,1,1,0,5,0).c_str()); }
void test_ai_mostly_approved(){ TEST_ASSERT_EQUAL_STRING("SHIP READY: Approved items ready to deploy", getAIRecommendation(10,1,0,0,0,0).c_str()); }
void test_ai_blocked()       { TEST_ASSERT_EQUAL_STRING("BLOCKED: Clear blockers before proceeding", getAIRecommendation(1,0,0,0,0,5).c_str()); }

// ═══════════════════════════════════════════════════════════════════
// 8. Rate-limit window logic  (mirrors api_functions.h checkRateLimit)
// ═══════════════════════════════════════════════════════════════════

struct RateLimitTracker {
  unsigned long lastRequest;
  int requestCount;
  unsigned long windowStart;
};

static RateLimitTracker rateLimits[5];

static bool checkRateLimit(int apiIndex, int maxRequests,
                            unsigned long windowMs, unsigned long nowMs) {
  RateLimitTracker& tracker = rateLimits[apiIndex];

  // Reset window if expired
  if (nowMs - tracker.windowStart >= windowMs) {
    tracker.windowStart  = nowMs;
    tracker.requestCount = 0;
  }

  if (tracker.requestCount >= maxRequests) return false;

  tracker.requestCount++;
  tracker.lastRequest = nowMs;
  return true;
}

void test_ratelimit_allows_first_request() {
  memset(&rateLimits[0], 0, sizeof(RateLimitTracker));
  TEST_ASSERT_TRUE(checkRateLimit(0, 5, 60000, 1000));
}

void test_ratelimit_allows_up_to_max() {
  memset(&rateLimits[0], 0, sizeof(RateLimitTracker));
  for (int i = 0; i < 5; i++) {
    TEST_ASSERT_TRUE(checkRateLimit(0, 5, 60000, (unsigned long)(1000 + i * 100)));
  }
}

void test_ratelimit_blocks_after_max() {
  memset(&rateLimits[0], 0, sizeof(RateLimitTracker));
  for (int i = 0; i < 5; i++) checkRateLimit(0, 5, 60000, 1000);
  TEST_ASSERT_FALSE(checkRateLimit(0, 5, 60000, 2000));
}

void test_ratelimit_resets_after_window() {
  memset(&rateLimits[0], 0, sizeof(RateLimitTracker));
  // Exhaust limit
  for (int i = 0; i < 5; i++) checkRateLimit(0, 5, 60000, 1000);
  TEST_ASSERT_FALSE(checkRateLimit(0, 5, 60000, 2000));
  // After window expires (60,001 ms later) the count resets
  TEST_ASSERT_TRUE(checkRateLimit(0, 5, 60000, 62000));
}

// ═══════════════════════════════════════════════════════════════════
// Test runner
// ═══════════════════════════════════════════════════════════════════

void setUp(void)    {}
void tearDown(void) {}

int main(int /*argc*/, char** /*argv*/) {
  UNITY_BEGIN();

  // 1. URL building
  RUN_TEST(test_buildAPIUrl_no_slash);
  RUN_TEST(test_buildAPIUrl_base_has_slash);
  RUN_TEST(test_buildAPIUrl_endpoint_has_slash);
  RUN_TEST(test_buildAPIUrl_both_have_slash);
  RUN_TEST(test_buildAPIUrl_empty_endpoint);

  // 2. Sovereignty status
  RUN_TEST(test_sovereignty_sovereign);
  RUN_TEST(test_sovereignty_hybrid);
  RUN_TEST(test_sovereignty_dependent);
  RUN_TEST(test_sovereignty_captured);
  RUN_TEST(test_sovereignty_boundary_80);
  RUN_TEST(test_sovereignty_boundary_60);
  RUN_TEST(test_sovereignty_boundary_40);
  RUN_TEST(test_sovereignty_zero);
  RUN_TEST(test_sovereignty_score_full);
  RUN_TEST(test_sovereignty_score_half);
  RUN_TEST(test_sovereignty_score_none);
  RUN_TEST(test_sovereignty_score_zero_div);

  // 3. License forkability
  RUN_TEST(test_license_mit);
  RUN_TEST(test_license_apache);
  RUN_TEST(test_license_bsd);
  RUN_TEST(test_license_gpl);
  RUN_TEST(test_license_postgresql);
  RUN_TEST(test_license_proprietary);
  RUN_TEST(test_license_na);
  RUN_TEST(test_license_empty);

  // 4. Uptime strings
  RUN_TEST(test_uptime_seconds);
  RUN_TEST(test_uptime_minutes);
  RUN_TEST(test_uptime_hours);
  RUN_TEST(test_uptime_days);
  RUN_TEST(test_uptime_zero);
  RUN_TEST(test_uptime_exact_min);
  RUN_TEST(test_uptime_exact_hour);

  // 5. WiFi quality
  RUN_TEST(test_wifi_excellent);
  RUN_TEST(test_wifi_good);
  RUN_TEST(test_wifi_fair);
  RUN_TEST(test_wifi_poor);
  RUN_TEST(test_wifi_offline);
  RUN_TEST(test_wifi_boundary_80);
  RUN_TEST(test_wifi_boundary_60);

  // 6. Alert level strings
  RUN_TEST(test_alert_info);
  RUN_TEST(test_alert_warning);
  RUN_TEST(test_alert_critical);

  // 7. AI recommendation engine
  RUN_TEST(test_ai_no_actions);
  RUN_TEST(test_ai_high_critical);
  RUN_TEST(test_ai_mostly_approved);
  RUN_TEST(test_ai_blocked);

  // 8. Rate-limit logic
  RUN_TEST(test_ratelimit_allows_first_request);
  RUN_TEST(test_ratelimit_allows_up_to_max);
  RUN_TEST(test_ratelimit_blocks_after_max);
  RUN_TEST(test_ratelimit_resets_after_window);

  return UNITY_END();
}
