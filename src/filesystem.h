#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <SPIFFS.h>
#include <FS.h>
#include <HTTPClient.h>

/*
 * BlackRoad OS - Operator Device File System
 * SPIFFS-based storage for code, docs, and project files
 * Integrates with Claude Code for real-time editing
 */

// File system stats
struct FileSystemStats {
  size_t totalBytes;
  size_t usedBytes;
  size_t freeBytes;
  int fileCount;
  float usagePercent;
};

FileSystemStats fsStats;

// Initialize SPIFFS file system
bool initFileSystem() {
  Serial.println("\n💾 Initializing SPIFFS File System...");

  if (!SPIFFS.begin(true)) {  // true = format if mount fails
    Serial.println("❌ SPIFFS mount failed");
    return false;
  }

  // Get file system info
  fsStats.totalBytes = SPIFFS.totalBytes();
  fsStats.usedBytes = SPIFFS.usedBytes();
  fsStats.freeBytes = fsStats.totalBytes - fsStats.usedBytes;
  fsStats.usagePercent = (float)fsStats.usedBytes / fsStats.totalBytes * 100.0;

  Serial.println("✅ SPIFFS Initialized");
  Serial.printf("   Total: %u bytes (%.1f KB)\n", fsStats.totalBytes, fsStats.totalBytes/1024.0);
  Serial.printf("   Used:  %u bytes (%.1f KB)\n", fsStats.usedBytes, fsStats.usedBytes/1024.0);
  Serial.printf("   Free:  %u bytes (%.1f KB)\n", fsStats.freeBytes, fsStats.freeBytes/1024.0);
  Serial.printf("   Usage: %.1f%%\n", fsStats.usagePercent);

  return true;
}

// List all files in SPIFFS
void listFiles(const char* dirname = "/", uint8_t levels = 3) {
  Serial.printf("\n📂 Listing files in: %s\n", dirname);
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

  File root = SPIFFS.open(dirname);
  if (!root) {
    Serial.println("❌ Failed to open directory");
    return;
  }

  if (!root.isDirectory()) {
    Serial.println("❌ Not a directory");
    return;
  }

  File file = root.openNextFile();
  int count = 0;

  while (file) {
    if (file.isDirectory()) {
      Serial.printf("📁 %s/\n", file.name());
      if (levels) {
        listFiles(file.name(), levels - 1);
      }
    } else {
      Serial.printf("📄 %s (%u bytes)\n", file.name(), file.size());
      count++;
    }
    file = root.openNextFile();
  }

  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.printf("Total: %d files\n", count);

  fsStats.fileCount = count;
}

// Write file to SPIFFS
bool writeFile(const char* path, const char* content) {
  Serial.printf("💾 Writing file: %s\n", path);

  File file = SPIFFS.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("❌ Failed to open file for writing");
    return false;
  }

  size_t written = file.print(content);
  file.close();

  Serial.printf("✅ Wrote %u bytes to %s\n", written, path);
  return true;
}

// Read file from SPIFFS
String readFile(const char* path) {
  File file = SPIFFS.open(path);
  if (!file || file.isDirectory()) {
    Serial.printf("❌ Failed to open file: %s\n", path);
    return "";
  }

  String content = "";
  while (file.available()) {
    content += (char)file.read();
  }
  file.close();

  Serial.printf("✅ Read %u bytes from %s\n", content.length(), path);
  return content;
}

// Delete file from SPIFFS
bool deleteFile(const char* path) {
  Serial.printf("🗑️  Deleting file: %s\n", path);

  if (!SPIFFS.remove(path)) {
    Serial.println("❌ Delete failed");
    return false;
  }

  Serial.println("✅ File deleted");
  return true;
}

// Create directory
bool createDir(const char* path) {
  Serial.printf("📁 Creating directory: %s\n", path);

  if (SPIFFS.mkdir(path)) {
    Serial.println("✅ Directory created");
    return true;
  } else {
    Serial.println("❌ mkdir failed");
    return false;
  }
}

// Download file from URL and save to SPIFFS
bool downloadFile(const char* url, const char* path) {
  Serial.printf("⬇️  Downloading: %s\n", url);
  Serial.printf("   Saving to: %s\n", path);

  HTTPClient http;
  http.begin(url);

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();

    File file = SPIFFS.open(path, FILE_WRITE);
    if (!file) {
      Serial.println("❌ Failed to create file");
      http.end();
      return false;
    }

    file.print(payload);
    file.close();

    Serial.printf("✅ Downloaded %u bytes\n", payload.length());
    http.end();
    return true;
  } else {
    Serial.printf("❌ HTTP error: %d\n", httpCode);
    http.end();
    return false;
  }
}

// Get file system statistics
FileSystemStats getFileSystemStats() {
  fsStats.totalBytes = SPIFFS.totalBytes();
  fsStats.usedBytes = SPIFFS.usedBytes();
  fsStats.freeBytes = fsStats.totalBytes - fsStats.usedBytes;
  fsStats.usagePercent = (float)fsStats.usedBytes / fsStats.totalBytes * 100.0;

  return fsStats;
}

// Format SPIFFS (WARNING: Deletes all files!)
bool formatFileSystem() {
  Serial.println("\n⚠️  FORMATTING SPIFFS...");
  Serial.println("   This will DELETE ALL FILES!");

  if (SPIFFS.format()) {
    Serial.println("✅ Format complete");
    return true;
  } else {
    Serial.println("❌ Format failed");
    return false;
  }
}

// Sync project files from development machine
bool syncProjectFiles() {
  Serial.println("\n🔄 Syncing Project Files...");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

  // Create project structure
  createDir("/src");
  createDir("/include");
  createDir("/docs");

  // Sync core header files (embedded as strings for now)
  // In production, these would be downloaded from GitHub or served by HTTP

  Serial.println("✅ Project structure created");
  Serial.println("   Note: Full file sync requires HTTP server");
  Serial.println("   Use Claude Code API to push files to device");

  return true;
}

// Export file system contents as JSON
String exportFileSystemJSON() {
  String json = "{\"files\":[";

  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  bool first = true;

  while (file) {
    if (!file.isDirectory()) {
      if (!first) json += ",";
      json += "{";
      json += "\"name\":\"" + String(file.name()) + "\",";
      json += "\"size\":" + String(file.size());
      json += "}";
      first = false;
    }
    file = root.openNextFile();
  }

  json += "],";
  json += "\"stats\":{";
  json += "\"total\":" + String(fsStats.totalBytes) + ",";
  json += "\"used\":" + String(fsStats.usedBytes) + ",";
  json += "\"free\":" + String(fsStats.freeBytes);
  json += "}}";

  return json;
}

#endif // FILESYSTEM_H
