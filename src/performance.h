#ifndef PERFORMANCE_H
#define PERFORMANCE_H

/*
 * ═══════════════════════════════════════════════════════════════════════
 * BLACKROAD PERFORMANCE MONITOR
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Real-time system performance tracking:
 * - Heap memory usage and fragmentation
 * - CPU usage and loop timing
 * - WiFi signal strength
 * - API response times
 * - Screen refresh rate
 * - Touch responsiveness
 */

#include <esp_system.h>
#include <esp_heap_caps.h>

// ─────────────────────────────────────────────────────────────────────
// PERFORMANCE METRICS
// ─────────────────────────────────────────────────────────────────────

struct PerformanceMetrics {
  // Memory
  uint32_t freeHeap;
  uint32_t minFreeHeap;
  uint32_t heapSize;
  uint8_t heapFragmentation;

  // CPU & Timing
  uint32_t loopTime;        // microseconds
  uint32_t avgLoopTime;
  uint32_t maxLoopTime;
  float loopsPerSecond;

  // WiFi
  int8_t rssi;              // Signal strength (dBm)
  uint8_t wifiQuality;      // 0-100%

  // API Performance
  uint32_t lastNavUpdateMs;
  uint32_t avgApiResponseMs;

  // Uptime
  uint32_t uptimeSeconds;
  uint32_t lastRebootReason;
};

PerformanceMetrics perfMetrics;
unsigned long lastLoopTime = 0;
unsigned long loopCount = 0;
unsigned long loopTimeSum = 0;

// ─────────────────────────────────────────────────────────────────────
// INITIALIZE PERFORMANCE MONITORING
// ─────────────────────────────────────────────────────────────────────

void initPerformanceMonitor() {
  perfMetrics.freeHeap = ESP.getFreeHeap();
  perfMetrics.minFreeHeap = perfMetrics.freeHeap;
  perfMetrics.heapSize = ESP.getHeapSize();
  perfMetrics.heapFragmentation = 0;
  perfMetrics.loopTime = 0;
  perfMetrics.avgLoopTime = 0;
  perfMetrics.maxLoopTime = 0;
  perfMetrics.loopsPerSecond = 0;
  perfMetrics.rssi = 0;
  perfMetrics.wifiQuality = 0;
  perfMetrics.lastNavUpdateMs = 0;
  perfMetrics.avgApiResponseMs = 0;
  perfMetrics.uptimeSeconds = 0;
  perfMetrics.lastRebootReason = esp_reset_reason();

  Serial.println("\n📊 Performance Monitor Initialized");
  Serial.printf("   Heap Size: %d bytes\n", perfMetrics.heapSize);
  Serial.printf("   Free Heap: %d bytes\n", perfMetrics.freeHeap);
}

// ─────────────────────────────────────────────────────────────────────
// UPDATE PERFORMANCE METRICS
// ─────────────────────────────────────────────────────────────────────

void updatePerformanceMetrics() {
  // Memory metrics
  perfMetrics.freeHeap = ESP.getFreeHeap();
  if (perfMetrics.freeHeap < perfMetrics.minFreeHeap) {
    perfMetrics.minFreeHeap = perfMetrics.freeHeap;
  }

  // Calculate fragmentation percentage
  uint32_t largestBlock = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  if (perfMetrics.freeHeap > 0) {
    perfMetrics.heapFragmentation = 100 - ((largestBlock * 100) / perfMetrics.freeHeap);
  }

  // Loop timing
  unsigned long currentTime = micros();
  if (lastLoopTime > 0) {
    perfMetrics.loopTime = currentTime - lastLoopTime;

    if (perfMetrics.loopTime > perfMetrics.maxLoopTime) {
      perfMetrics.maxLoopTime = perfMetrics.loopTime;
    }

    loopTimeSum += perfMetrics.loopTime;
    loopCount++;

    if (loopCount >= 1000) {
      perfMetrics.avgLoopTime = loopTimeSum / loopCount;
      loopTimeSum = 0;
      loopCount = 0;
    }
  }
  lastLoopTime = currentTime;

  // Calculate loops per second
  if (perfMetrics.avgLoopTime > 0) {
    perfMetrics.loopsPerSecond = 1000000.0 / perfMetrics.avgLoopTime;
  }

  // WiFi metrics
  if (WiFi.status() == WL_CONNECTED) {
    perfMetrics.rssi = WiFi.RSSI();

    // Convert RSSI to quality percentage
    // -30 dBm = 100%, -90 dBm = 0%
    if (perfMetrics.rssi >= -30) {
      perfMetrics.wifiQuality = 100;
    } else if (perfMetrics.rssi <= -90) {
      perfMetrics.wifiQuality = 0;
    } else {
      perfMetrics.wifiQuality = map(perfMetrics.rssi, -90, -30, 0, 100);
    }
  } else {
    perfMetrics.rssi = -100;
    perfMetrics.wifiQuality = 0;
  }

  // Uptime
  perfMetrics.uptimeSeconds = millis() / 1000;
}

// ─────────────────────────────────────────────────────────────────────
// MEMORY HELPERS
// ─────────────────────────────────────────────────────────────────────

uint32_t getFreeHeap() {
  return ESP.getFreeHeap();
}

uint32_t getHeapSize() {
  return ESP.getHeapSize();
}

uint8_t getHeapUsagePercent() {
  return ((perfMetrics.heapSize - perfMetrics.freeHeap) * 100) / perfMetrics.heapSize;
}

String getHeapUsageString() {
  uint32_t used = perfMetrics.heapSize - perfMetrics.freeHeap;
  return String(used / 1024) + "/" + String(perfMetrics.heapSize / 1024) + "KB";
}

// ─────────────────────────────────────────────────────────────────────
// CPU HELPERS
// ─────────────────────────────────────────────────────────────────────

float getCPUFrequencyMHz() {
  return getCpuFrequencyMhz();
}

String getUptimeString() {
  uint32_t seconds = perfMetrics.uptimeSeconds;
  uint32_t minutes = seconds / 60;
  uint32_t hours = minutes / 60;
  uint32_t days = hours / 24;

  if (days > 0) {
    return String(days) + "d " + String(hours % 24) + "h";
  } else if (hours > 0) {
    return String(hours) + "h " + String(minutes % 60) + "m";
  } else if (minutes > 0) {
    return String(minutes) + "m " + String(seconds % 60) + "s";
  } else {
    return String(seconds) + "s";
  }
}

// ─────────────────────────────────────────────────────────────────────
// WIFI HELPERS
// ─────────────────────────────────────────────────────────────────────

String getWiFiQualityString() {
  if (perfMetrics.wifiQuality >= 80) return "Excellent";
  if (perfMetrics.wifiQuality >= 60) return "Good";
  if (perfMetrics.wifiQuality >= 40) return "Fair";
  if (perfMetrics.wifiQuality >= 20) return "Poor";
  return "Offline";
}

uint16_t getWiFiQualityColor() {
  if (perfMetrics.wifiQuality >= 80) return COLOR_VIVID_PUR;
  if (perfMetrics.wifiQuality >= 60) return COLOR_CYBER_BLUE;
  if (perfMetrics.wifiQuality >= 40) return COLOR_SUNRISE;
  return COLOR_HOT_PINK;
}

// ─────────────────────────────────────────────────────────────────────
// REBOOT REASON DECODER
// ─────────────────────────────────────────────────────────────────────

String getRebootReasonString() {
  switch (perfMetrics.lastRebootReason) {
    case ESP_RST_POWERON: return "Power On";
    case ESP_RST_SW: return "Software Reset";
    case ESP_RST_PANIC: return "Panic/Exception";
    case ESP_RST_INT_WDT: return "Watchdog";
    case ESP_RST_TASK_WDT: return "Task Watchdog";
    case ESP_RST_WDT: return "Other Watchdog";
    case ESP_RST_DEEPSLEEP: return "Deep Sleep";
    case ESP_RST_BROWNOUT: return "Brownout";
    case ESP_RST_SDIO: return "SDIO Reset";
    default: return "Unknown";
  }
}

// ─────────────────────────────────────────────────────────────────────
// PERFORMANCE WARNING CHECKS
// ─────────────────────────────────────────────────────────────────────

bool isLowMemory() {
  return perfMetrics.freeHeap < 10000;  // Less than 10KB free
}

bool isHighFragmentation() {
  return perfMetrics.heapFragmentation > 30;  // More than 30% fragmented
}

bool isSlowLoop() {
  return perfMetrics.avgLoopTime > 100000;  // Slower than 100ms
}

bool isWiFiWeak() {
  return perfMetrics.wifiQuality < 40;
}

// ─────────────────────────────────────────────────────────────────────
// DIAGNOSTIC REPORT
// ─────────────────────────────────────────────────────────────────────

void printDiagnosticReport() {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║   BLACKROAD PERFORMANCE DIAGNOSTICS    ║");
  Serial.println("╠════════════════════════════════════════╣");

  // Memory
  Serial.println("║ MEMORY                                 ║");
  Serial.printf("║   Free:       %6d KB                ║\n", perfMetrics.freeHeap / 1024);
  Serial.printf("║   Min Free:   %6d KB                ║\n", perfMetrics.minFreeHeap / 1024);
  Serial.printf("║   Total:      %6d KB                ║\n", perfMetrics.heapSize / 1024);
  Serial.printf("║   Usage:      %6d%%                  ║\n", getHeapUsagePercent());
  Serial.printf("║   Fragment:   %6d%%                  ║\n", perfMetrics.heapFragmentation);

  // CPU & Timing
  Serial.println("║ CPU & TIMING                           ║");
  Serial.printf("║   Frequency:  %6.0f MHz               ║\n", getCPUFrequencyMHz());
  Serial.printf("║   Loop Time:  %6lu µs                ║\n", perfMetrics.loopTime);
  Serial.printf("║   Avg Loop:   %6lu µs                ║\n", perfMetrics.avgLoopTime);
  Serial.printf("║   Max Loop:   %6lu µs                ║\n", perfMetrics.maxLoopTime);
  Serial.printf("║   Loop/Sec:   %6.0f                   ║\n", perfMetrics.loopsPerSecond);

  // WiFi
  Serial.println("║ WIFI                                   ║");
  Serial.printf("║   RSSI:       %6d dBm                ║\n", perfMetrics.rssi);
  Serial.printf("║   Quality:    %6d%% (%s)%*s║\n",
    perfMetrics.wifiQuality,
    getWiFiQualityString().c_str(),
    9 - getWiFiQualityString().length(), "");

  // System
  Serial.println("║ SYSTEM                                 ║");
  Serial.printf("║   Uptime:     %s%*s║\n",
    getUptimeString().c_str(),
    25 - getUptimeString().length(), "");
  Serial.printf("║   Last Boot:  %s%*s║\n",
    getRebootReasonString().c_str(),
    24 - getRebootReasonString().length(), "");

  // Warnings
  Serial.println("║ WARNINGS                               ║");
  if (isLowMemory()) {
    Serial.println("║   ⚠️  LOW MEMORY!                      ║");
  }
  if (isHighFragmentation()) {
    Serial.println("║   ⚠️  HIGH FRAGMENTATION!              ║");
  }
  if (isSlowLoop()) {
    Serial.println("║   ⚠️  SLOW LOOP TIME!                  ║");
  }
  if (isWiFiWeak()) {
    Serial.println("║   ⚠️  WEAK WIFI SIGNAL!                ║");
  }
  if (!isLowMemory() && !isHighFragmentation() && !isSlowLoop() && !isWiFiWeak()) {
    Serial.println("║   ✅ All systems nominal               ║");
  }

  Serial.println("╚════════════════════════════════════════╝\n");
}

#endif // PERFORMANCE_H
