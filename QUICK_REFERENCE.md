# ⚡ ESP32 UX/UI Quick Reference Card

## 📐 Screen: 240×320 (Portrait)
```
Status:  0-20   (20px)
Header:  20-50  (30px)
Content: 50-270 (220px) ← YOUR WORKING AREA
BottomNav: 270-320 (50px)
```

## 🎨 Brand Colors (RGB565)
```cpp
COLOR_HOT_PINK     0xF8EA  // PRIMARY!
COLOR_AMBER        0xFD40
COLOR_VIOLET       0x9A74
COLOR_ELECTRIC_BLUE 0x14FF
COLOR_BLACK        0x0000
COLOR_WHITE        0xFFFF
```

## 📏 Golden Ratio Spacing
```cpp
SPACE_XS   8px
SPACE_SM   13px  ← margins
SPACE_MD   21px  ← padding
SPACE_LG   34px  ← cards
SPACE_XL   55px  ← sections
```

## 🏃 Animation Timing
```cpp
ANIM_FAST    100ms  // touch feedback
ANIM_MEDIUM  200ms  // transitions
ANIM_SLOW    400ms  // emphasis
```

## 👆 Touch Zones
```cpp
HOME button:   x:10-90,   y:278-313 (80×35)
ACTION button: x:150-230, y:278-313 (80×35)
App icons:     45×45px minimum
```

## 💡 Quick Functions
```cpp
// Gradient icon with badge
brUI.drawGradientIcon(x, y, 45, "AI", 
  COLOR_VIOLET, COLOR_DEEP_PURPLE, badgeCount);

// Animated button
brUI.drawAnimatedButton(x, y, w, h, "OK",
  COLOR_HOT_PINK, COLOR_WHITE, isPressed);

// Touch ripple
brUI.startTouchFeedback(touchX, touchY);
brUI.drawTouchRipple(touchX, touchY, COLOR_ELECTRIC_BLUE);

// Progress bar (0.0 - 1.0)
brUI.drawProgressBar(x, y, w, h, progress,
  COLOR_DARK_GRAY, COLOR_AMBER);

// Toast notification
brUI.showToast("Connected!", COLOR_ELECTRIC_BLUE, 2000);

// Color interpolation
uint16_t mixed = brUI.lerpColor(color1, color2, 0.5);
```

## 🎯 Layout Template
```cpp
void drawMyApp() {
  tft.fillScreen(COLOR_BLACK);
  
  // Status bar (auto-drawn by drawStatusBar())
  drawStatusBar();
  
  // Header (y: 20-50)
  tft.fillRect(0, 20, SCREEN_WIDTH, 30, COLOR_DARK_GRAY);
  tft.setTextColor(COLOR_HOT_PINK);
  tft.drawString("MY APP", 120, 27, 4);
  
  // Content (y: 50-270)
  // ... your UI here (220px available) ...
  
  // Bottom navigation (auto-drawn)
  drawBottomNav();
}
```

## ⚠️ REMEMBER
- ✅ Use brand colors ONLY
- ✅ Touch targets ≥ 40×40px
- ✅ Content fits in 50-270 zone
- ✅ Golden Ratio spacing
- ✅ Keep animations < 16ms/frame
- ❌ Don't draw over bottom nav
- ❌ Don't use old colors (#FF9D00, #FF6B00, etc.)

---
**Created by SILAS for the ESP32 Team** 🚀
