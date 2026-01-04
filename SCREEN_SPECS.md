# 📐 ESP32-2432S028R Display Specifications

## Hardware
- **Device:** ESP32-2432S028R (Sunton)
- **Display:** ILI9341 TFT LCD
- **Resolution:** 240×320 pixels (portrait orientation)
- **Touch:** XPT2046 resistive touchscreen
- **Processor:** ESP32 @ 240MHz
- **RAM:** 320KB
- **Flash:** 4MB

## Display Specs
- **Width:** 240 pixels
- **Height:** 320 pixels
- **Orientation:** Portrait (240W × 320H)
- **Color Depth:** RGB565 (16-bit, 65,536 colors)
- **Driver:** TFT_eSPI library (Bodmer)
- **SPI Speed:** 40MHz (fast\!)

## Pin Configuration
```cpp
TFT Pins:
  MISO = 12
  MOSI = 13
  SCLK = 14
  CS   = 15
  DC   = 2
  RST  = -1 (not used)
  BL   = 21 (backlight)

Touch Pins (XPT2046):
  IRQ  = 36
  MOSI = 32
  MISO = 39
  CLK  = 25
  CS   = 33
```

## Screen Layout Zones

### Portrait Mode (240×320)
```
     0px ┌──────────────────────────────┐
         │  STATUS BAR                  │
    20px ├──────────────────────────────┤
         │  HEADER / TITLE              │
    50px ├──────────────────────────────┤
         │                              │
         │                              │
         │  MAIN CONTENT AREA           │
         │                              │
         │  (220px height available)    │
         │                              │
         │                              │
   270px ├──────────────────────────────┤
         │  BOTTOM NAVIGATION BAR       │
   320px └──────────────────────────────┘
         0px                        240px
```

## Safe Zones

### Status Bar (y: 0-20)
- **Height:** 20px
- **Purpose:** Time, WiFi, battery, system indicators
- **Background:** `COLOR_DARK_GRAY` (#222222)
- **Text:** Small font (size 1)

### Header (y: 20-50)
- **Height:** 30px
- **Purpose:** App name, subtitle, version
- **Background:** Usually `COLOR_DARK_GRAY`
- **Text:** Medium font (size 2-4)

### Content Area (y: 50-270)
- **Height:** 220px
- **Purpose:** Main UI, app grids, data display
- **Scrollable:** Yes (if content exceeds 220px)
- **Max elements:** Plan for ~200px usable (leave margins)

### Bottom Navigation (y: 270-320)
- **Height:** 50px
- **Purpose:** HOME + ACTION buttons, navigation
- **Always visible:** Yes
- **Background:** `COLOR_DARK_GRAY`
- **Buttons:** 80×35px each

## Touch Zones (Minimum Sizes)

### Optimal Touch Targets
- **Minimum:** 40×40px (resistive touch)
- **Recommended:** 50×50px
- **Buttons:** 80×35px (current bottom nav)
- **App Icons:** 45×45px (good size\!)

### Touch Calibration
```cpp
// Raw touch range: 200-3700
x = map(rawX, 200, 3700, 0, 240);
y = map(rawY, 200, 3700, 0, 320);
x = constrain(x, 0, 240);
y = constrain(y, 0, 320);
```

## App Grid Layout (Home Screen)

### 4×5 Grid (17 apps + 3 empty slots)
```
Row 1 (y=60):  CEO  EXEC  META  WORK
Row 2 (y=120): AI   MSG   CRM   VPN
Row 3 (y=180): ID   FILE  API   SET
Row 4 (y=240): CC   CHAT  TERM  PAGER
Row 5 (y=300): (reserved for future)

Icon size: 45×45px
Spacing: ~10px horizontal, 60px vertical
Total icons: 20 slots (17 used)
```

## Color Format (RGB565)

### Conversion Formula
```cpp
RGB888 (#RRGGBB) → RGB565:
R5 = (R8 >> 3) & 0x1F    // 8-bit → 5-bit
G6 = (G8 >> 2) & 0x3F    // 8-bit → 6-bit  
B5 = (B8 >> 3) & 0x1F    // 8-bit → 5-bit
RGB565 = (R5 << 11) | (G6 << 5) | B5

Example: #FF1D6C (Hot Pink)
  R=0xFF → 0x1F (31)
  G=0x1D → 0x07 (7)
  B=0x6C → 0x0D (13)
  RGB565 = 0xF8EA
```

## Performance Considerations

### Frame Budget (60fps)
- **Frame time:** 16.67ms
- **Render budget:** ~12ms (leave margin)
- **Touch polling:** 4ms

### Optimization Tips
1. **Minimize full-screen redraws** - Update only changed regions
2. **Use fillRect() over pixel-by-pixel** - 10x faster
3. **Batch similar operations** - Reduce SPI overhead
4. **Pre-calculate gradients** - Store in arrays if static
5. **Limit animations** - Max 3-4 concurrent animations

### Memory Budget
- **Firmware:** ~850KB (65% of 1.3MB flash)
- **Runtime RAM:** ~180KB used of 320KB
- **Graphics buffer:** None (direct draw)
- **Font tables:** ~50KB (embedded)

## Constants to Use

```cpp
// Screen dimensions
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// Layout zones
#define STATUS_BAR_HEIGHT    20
#define HEADER_HEIGHT        30
#define CONTENT_Y_START      50
#define CONTENT_Y_END       270
#define BOTTOM_NAV_Y_START  270
#define BOTTOM_NAV_HEIGHT    50

// Safe margins
#define MARGIN_SIDES        10  // Left/right margin
#define MARGIN_TOP          50  // Below header
#define MARGIN_BOTTOM       50  // Above bottom nav

// Usable content dimensions
#define CONTENT_WIDTH       220  // 240 - (10×2)
#define CONTENT_HEIGHT      220  // 270 - 50
```

## Testing Checklist

- [ ] All UI fits within 240×320 bounds
- [ ] Touch targets ≥ 40×40px
- [ ] Text readable at all sizes
- [ ] No elements hidden by bottom nav
- [ ] Status bar always visible
- [ ] Gradients render smoothly
- [ ] Animations run at 60fps
- [ ] Colors match brand system
- [ ] Golden Ratio spacing applied
- [ ] Touch response < 100ms

---

**Last Updated:** January 3, 2026 by SILAS
**Reference:** platformio.ini, main.cpp, BlackRoadUI.h
