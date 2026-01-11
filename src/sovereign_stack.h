#ifndef SOVEREIGN_STACK_H
#define SOVEREIGN_STACK_H

/*
 * ═══════════════════════════════════════════════════════════════════════
 * BLACKROAD SOVEREIGN STACK MONITOR
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Monitors the health and status of all sovereign infrastructure:
 * - Layer 1: Infrastructure (Headscale, Keycloak, PostgreSQL, Forgejo)
 * - Layer 2: Services (Stripe, Canva, HuggingFace)
 * - Layer 3: Applications (CRM, AI, Custom Logic)
 *
 * Based on: BLACKROAD_FORKIES_CANONICAL_STACK.md
 */

// ─────────────────────────────────────────────────────────────────────
// STACK COMPONENT STATUS
// ─────────────────────────────────────────────────────────────────────

struct StackComponent {
  String name;
  String category;      // "identity", "mesh", "ai", "crm", "database"
  String status;        // "active", "degraded", "offline", "unknown"
  bool sovereign;       // true if self-hosted (Layer 1), false if SaaS (Layer 2)
  String license;       // "MIT", "Apache-2.0", "GPLv3", etc.
  unsigned long lastCheck;
};

StackComponent sovereignStack[20];
int stackComponentCount = 0;

// ─────────────────────────────────────────────────────────────────────
// INITIALIZE SOVEREIGN STACK COMPONENTS
// ─────────────────────────────────────────────────────────────────────

void initSovereignStack() {
  Serial.println("\n🏗️ Initializing Sovereign Stack Monitor...");

  stackComponentCount = 0;

  // LAYER 1: INFRASTRUCTURE (Sovereign - Self-Hosted)
  // ─────────────────────────────────────────────────────────────────────

  // Identity & Auth
  sovereignStack[stackComponentCount++] = {
    "Keycloak", "identity", "unknown", true, "Apache-2.0", 0
  };

  sovereignStack[stackComponentCount++] = {
    "Authelia", "identity", "unknown", true, "Apache-2.0", 0
  };

  // Network Mesh
  sovereignStack[stackComponentCount++] = {
    "Headscale", "mesh", "unknown", true, "MIT", 0
  };

  sovereignStack[stackComponentCount++] = {
    "Tailscale", "mesh", "active", false, "BSD-3", 0  // Using their control plane
  };

  // Databases
  sovereignStack[stackComponentCount++] = {
    "PostgreSQL", "database", "unknown", true, "PostgreSQL", 0
  };

  sovereignStack[stackComponentCount++] = {
    "Qdrant", "database", "unknown", true, "Apache-2.0", 0
  };

  // Version Control
  sovereignStack[stackComponentCount++] = {
    "Forgejo", "devops", "unknown", true, "MIT", 0
  };

  // AI Stack
  sovereignStack[stackComponentCount++] = {
    "vLLM", "ai", "unknown", true, "Apache-2.0", 0
  };

  sovereignStack[stackComponentCount++] = {
    "Ollama", "ai", "unknown", true, "MIT", 0
  };

  // CRM
  sovereignStack[stackComponentCount++] = {
    "EspoCRM", "crm", "unknown", true, "GPLv3", 0
  };

  // LAYER 2: SERVICES (External APIs - Used but not controlled)
  // ─────────────────────────────────────────────────────────────────────

  sovereignStack[stackComponentCount++] = {
    "Cloudflare", "infrastructure", "active", false, "N/A", 0
  };

  sovereignStack[stackComponentCount++] = {
    "GitHub", "devops", "active", false, "N/A", 0
  };

  sovereignStack[stackComponentCount++] = {
    "HuggingFace", "ai", "unknown", false, "N/A", 0
  };

  sovereignStack[stackComponentCount++] = {
    "Stripe", "payments", "unknown", false, "N/A", 0
  };

  Serial.printf("  ✓ Initialized %d stack components\n", stackComponentCount);
  Serial.println("  ✓ Sovereign stack monitor ready\n");
}

// ─────────────────────────────────────────────────────────────────────
// CHECK COMPONENT HEALTH
// ─────────────────────────────────────────────────────────────────────

String checkComponentHealth(String category) {
  // Map category to navigation state
  if (category == "mesh") {
    return navState.meshHealthy ? "active" : "degraded";
  }
  else if (category == "crm") {
    return navState.crmHealthy ? "active" : "degraded";
  }
  else if (category == "ai") {
    return navState.aiHealthy ? "active" : "degraded";
  }
  else if (category == "identity" || category == "database") {
    return "unknown";  // Not monitored yet
  }
  return "unknown";
}

void updateStackHealth() {
  Serial.println("\n🔍 Checking sovereign stack health...");

  for (int i = 0; i < stackComponentCount; i++) {
    String prevStatus = sovereignStack[i].status;
    sovereignStack[i].status = checkComponentHealth(sovereignStack[i].category);
    sovereignStack[i].lastCheck = millis();

    if (prevStatus != sovereignStack[i].status) {
      Serial.printf("  %s: %s → %s\n",
        sovereignStack[i].name.c_str(),
        prevStatus.c_str(),
        sovereignStack[i].status.c_str()
      );
    }
  }
}

// ─────────────────────────────────────────────────────────────────────
// SOVEREIGNTY METRICS
// ─────────────────────────────────────────────────────────────────────

struct SovereigntyMetrics {
  int totalComponents;
  int sovereignComponents;
  int activeComponents;
  int degradedComponents;
  int offlineComponents;
  float sovereigntyScore;  // Percentage of sovereign components
};

SovereigntyMetrics getSovereigntyMetrics() {
  SovereigntyMetrics metrics = {0};

  metrics.totalComponents = stackComponentCount;

  for (int i = 0; i < stackComponentCount; i++) {
    if (sovereignStack[i].sovereign) {
      metrics.sovereignComponents++;
    }

    if (sovereignStack[i].status == "active") {
      metrics.activeComponents++;
    } else if (sovereignStack[i].status == "degraded") {
      metrics.degradedComponents++;
    } else if (sovereignStack[i].status == "offline") {
      metrics.offlineComponents++;
    }
  }

  metrics.sovereigntyScore = (float)metrics.sovereignComponents / metrics.totalComponents * 100.0;

  return metrics;
}

// ─────────────────────────────────────────────────────────────────────
// FORKIES CHECK - Can we fork everything?
// ─────────────────────────────────────────────────────────────────────

bool isForkableLicense(String license) {
  // OSI-approved open source licenses
  const char* forkable[] = {
    "MIT", "Apache-2.0", "BSD", "GPLv3", "AGPLv3", "MPL-2.0",
    "LGPLv3", "PostgreSQL", "ISC"
  };

  for (int i = 0; i < 9; i++) {
    if (license.indexOf(forkable[i]) >= 0) {
      return true;
    }
  }

  return false;
}

int getForkableCount() {
  int count = 0;
  for (int i = 0; i < stackComponentCount; i++) {
    if (sovereignStack[i].sovereign && isForkableLicense(sovereignStack[i].license)) {
      count++;
    }
  }
  return count;
}

// ─────────────────────────────────────────────────────────────────────
// SOVEREIGNTY STATUS SUMMARY
// ─────────────────────────────────────────────────────────────────────

String getSovereigntyStatus() {
  SovereigntyMetrics metrics = getSovereigntyMetrics();

  if (metrics.sovereigntyScore >= 80.0) {
    return "SOVEREIGN";
  } else if (metrics.sovereigntyScore >= 60.0) {
    return "HYBRID";
  } else if (metrics.sovereigntyScore >= 40.0) {
    return "DEPENDENT";
  } else {
    return "CAPTURED";
  }
}

uint16_t getSovereigntyColor() {
  float score = getSovereigntyMetrics().sovereigntyScore;

  if (score >= 80.0) return COLOR_VIVID_PUR;    // Sovereign = Purple
  if (score >= 60.0) return COLOR_CYBER_BLUE;   // Hybrid = Blue
  if (score >= 40.0) return COLOR_SUNRISE;      // Dependent = Amber
  return COLOR_HOT_PINK;                        // Captured = Pink (alert!)
}

#endif // SOVEREIGN_STACK_H
