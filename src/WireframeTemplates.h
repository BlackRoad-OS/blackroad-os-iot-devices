#ifndef WIREFRAME_TEMPLATES_H
#define WIREFRAME_TEMPLATES_H

#include <TFT_eSPI.h>

/*
 * ⚡ Wireframe Templates for ESP32-2432S028R (240×320)
 *
 * Instant UI scaffolding - create new apps in SECONDS!
 * All templates follow BlackRoad brand guidelines & Golden Ratio spacing
 *
 * Usage:
 *   WireframeTemplates wf(&tft);
 *   wf.drawTemplate_List(title, items, itemCount);
 */

class WireframeTemplates {
private:
    TFT_eSPI* tft;

    // Helper: Draw template frame
    void drawFrame(const char* title, const char* subtitle = nullptr) {
        tft->fillScreen(COLOR_BLACK);

        // Status bar
        tft->fillRect(0, 0, SCREEN_WIDTH, 20, COLOR_DARK_GRAY);
        tft->setTextColor(COLOR_ELECTRIC_BLUE);
        tft->setTextDatum(TL_DATUM);
        tft->drawString("12:00", 8, 5, 1);
        tft->setTextDatum(TR_DATUM);
        tft->drawString("WiFi 100%", 232, 5, 1);

        // Header
        tft->fillRect(0, 20, SCREEN_WIDTH, 30, COLOR_DARK_GRAY);
        tft->setTextColor(COLOR_HOT_PINK);
        tft->setTextDatum(MC_DATUM);
        tft->drawString(title, 120, 35, 2);

        // Subtitle (if provided)
        if (subtitle) {
            tft->setTextColor(COLOR_AMBER);
            tft->setTextDatum(TC_DATUM);
            tft->drawString(subtitle, 120, 55, 1);
        }

        // Bottom nav
        tft->fillRect(0, 270, SCREEN_WIDTH, 50, COLOR_DARK_GRAY);
        tft->drawFastHLine(0, 270, SCREEN_WIDTH, COLOR_ELECTRIC_BLUE);

        tft->fillRoundRect(SPACE_SM, 278, 80, 35, 8, COLOR_VIOLET);
        tft->setTextColor(COLOR_WHITE);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("HOME", 50, 295, 2);

        tft->fillRoundRect(150, 278, 80, 35, 8, COLOR_HOT_PINK);
        tft->drawString("NEXT", 190, 295, 2);

        tft->fillCircle(120, 295, 5, COLOR_ELECTRIC_BLUE);
    }

public:
    WireframeTemplates(TFT_eSPI* display) : tft(display) {}

    // ==================== TEMPLATE 1: LIST VIEW ====================
    // Perfect for: Messages, Files, Contacts, History
    void drawTemplate_List(const char* title, const char** items, int itemCount, int selectedIndex = -1) {
        drawFrame(title, "List View Template");

        int startY = 70;
        int itemHeight = 40;
        int maxVisible = 5; // Fits in 50-270 zone

        for (int i = 0; i < itemCount && i < maxVisible; i++) {
            int y = startY + (i * itemHeight);

            // Item background
            uint16_t bgColor = (i == selectedIndex) ? COLOR_DEEP_PURPLE : COLOR_BLACK;
            tft->fillRoundRect(SPACE_SM, y, 220, 35, 4, bgColor);

            // Item border
            tft->drawRoundRect(SPACE_SM, y, 220, 35, 4, COLOR_VIOLET);

            // Item text
            tft->setTextColor(COLOR_WHITE);
            tft->setTextDatum(ML_DATUM);
            tft->drawString(items[i], SPACE_MD, y + 17, 2);

            // Right arrow
            tft->setTextColor(COLOR_AMBER);
            tft->setTextDatum(MR_DATUM);
            tft->drawString(">", 220, y + 17, 2);
        }

        // Scroll indicator if more items
        if (itemCount > maxVisible) {
            tft->fillRect(235, 70, 3, 200, COLOR_DARK_GRAY);
            int barHeight = (maxVisible * 200) / itemCount;
            tft->fillRect(235, 70, 3, barHeight, COLOR_HOT_PINK);
        }
    }

    // ==================== TEMPLATE 2: CARD VIEW ====================
    // Perfect for: Dashboards, Stats, Widgets
    void drawTemplate_Cards(const char* title, const char** cardTitles, const char** cardValues, int cardCount) {
        drawFrame(title, "Card View Template");

        int cardWidth = 100;
        int cardHeight = 70;
        int cols = 2;
        int startY = 70;

        for (int i = 0; i < cardCount && i < 4; i++) { // Max 4 cards
            int col = i % cols;
            int row = i / cols;
            int x = SPACE_MD + (col * (cardWidth + SPACE_SM));
            int y = startY + (row * (cardHeight + SPACE_SM));

            // Card background with gradient effect (3 steps)
            uint16_t colors[] = {COLOR_VIOLET, COLOR_DEEP_PURPLE, COLOR_BLACK};
            for (int step = 0; step < 3; step++) {
                int stepH = cardHeight / 3;
                tft->fillRect(x, y + (step * stepH), cardWidth, stepH, colors[step]);
            }

            // Card border
            tft->drawRoundRect(x, y, cardWidth, cardHeight, 6, COLOR_HOT_PINK);

            // Card title
            tft->setTextColor(COLOR_AMBER);
            tft->setTextDatum(TC_DATUM);
            tft->drawString(cardTitles[i], x + cardWidth/2, y + 8, 1);

            // Card value (large)
            tft->setTextColor(COLOR_WHITE);
            tft->setTextDatum(MC_DATUM);
            tft->drawString(cardValues[i], x + cardWidth/2, y + 40, 4);
        }
    }

    // ==================== TEMPLATE 3: FORM VIEW ====================
    // Perfect for: Settings, Login, Input screens
    void drawTemplate_Form(const char* title, const char** fieldLabels, const char** fieldValues, int fieldCount) {
        drawFrame(title, "Form View Template");

        int startY = 70;
        int fieldHeight = 45;
        int maxFields = 4;

        for (int i = 0; i < fieldCount && i < maxFields; i++) {
            int y = startY + (i * fieldHeight);

            // Field label
            tft->setTextColor(COLOR_AMBER);
            tft->setTextDatum(TL_DATUM);
            tft->drawString(fieldLabels[i], SPACE_MD, y, 1);

            // Field input box
            tft->drawRoundRect(SPACE_MD, y + 15, 200, 25, 4, COLOR_VIOLET);

            // Field value
            tft->setTextColor(COLOR_WHITE);
            tft->setTextDatum(ML_DATUM);
            tft->drawString(fieldValues[i], SPACE_MD + 8, y + 27, 2);

            // Input indicator
            tft->fillCircle(215, y + 27, 3, COLOR_HOT_PINK);
        }

        // Submit button
        tft->fillRoundRect(SPACE_LG, 250, 170, 40, 8, COLOR_HOT_PINK);
        tft->setTextColor(COLOR_WHITE);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("SUBMIT", 120, 270, 2);
    }

    // ==================== TEMPLATE 4: GRID VIEW ====================
    // Perfect for: App launcher, Photo gallery, Icons
    void drawTemplate_Grid(const char* title, const char** itemLabels, int itemCount, int cols = 3) {
        drawFrame(title, "Grid View Template");

        int iconSize = 50;
        int spacing = 15;
        int startX = (SCREEN_WIDTH - (cols * (iconSize + spacing))) / 2;
        int startY = 70;

        for (int i = 0; i < itemCount && i < 9; i++) { // Max 9 items
            int col = i % cols;
            int row = i / cols;
            int x = startX + (col * (iconSize + spacing));
            int y = startY + (row * (iconSize + spacing + 20));

            // Icon circle with gradient
            uint16_t iconColors[] = {COLOR_HOT_PINK, COLOR_VIOLET, COLOR_ELECTRIC_BLUE};
            uint16_t iconColor = iconColors[i % 3];

            tft->fillCircle(x + iconSize/2, y + iconSize/2, iconSize/2, iconColor);
            tft->drawCircle(x + iconSize/2, y + iconSize/2, iconSize/2, COLOR_WHITE);

            // Icon label
            tft->setTextColor(COLOR_AMBER);
            tft->setTextDatum(TC_DATUM);
            tft->drawString(itemLabels[i], x + iconSize/2, y + iconSize + 5, 1);
        }
    }

    // ==================== TEMPLATE 5: DETAIL VIEW ====================
    // Perfect for: Single item details, Profile, Info screen
    void drawTemplate_Detail(const char* title, const char* subtitle, const char** labels, const char** values, int rowCount) {
        drawFrame(title, subtitle);

        // Large icon/avatar
        tft->fillCircle(120, 90, 30, COLOR_VIOLET);
        tft->drawCircle(120, 90, 30, COLOR_WHITE);
        tft->setTextColor(COLOR_WHITE);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("OP", 120, 90, 4);

        // Detail rows
        int startY = 135;
        int rowHeight = 25;

        for (int i = 0; i < rowCount && i < 5; i++) {
            int y = startY + (i * rowHeight);

            // Label (left)
            tft->setTextColor(COLOR_AMBER);
            tft->setTextDatum(TL_DATUM);
            tft->drawString(labels[i], SPACE_MD, y, 1);

            // Value (right)
            tft->setTextColor(COLOR_WHITE);
            tft->setTextDatum(TR_DATUM);
            tft->drawString(values[i], 220, y, 1);

            // Divider
            tft->drawFastHLine(SPACE_MD, y + 18, 200, COLOR_DARK_GRAY);
        }
    }

    // ==================== TEMPLATE 6: SPLIT VIEW ====================
    // Perfect for: Side-by-side comparison, Before/After
    void drawTemplate_Split(const char* title, const char* leftTitle, const char* rightTitle) {
        drawFrame(title, "Split View Template");

        int centerX = SCREEN_WIDTH / 2;

        // Vertical divider
        tft->drawFastVLine(centerX, 50, 220, COLOR_VIOLET);

        // Left panel
        tft->setTextColor(COLOR_HOT_PINK);
        tft->setTextDatum(TC_DATUM);
        tft->drawString(leftTitle, centerX/2, 60, 2);

        tft->fillRoundRect(SPACE_SM, 85, 100, 70, 6, COLOR_DEEP_PURPLE);
        tft->setTextColor(COLOR_WHITE);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("Content", centerX/2, 120, 2);

        // Right panel
        tft->setTextColor(COLOR_HOT_PINK);
        tft->setTextDatum(TC_DATUM);
        tft->drawString(rightTitle, centerX + centerX/2, 60, 2);

        tft->fillRoundRect(130, 85, 100, 70, 6, COLOR_DEEP_PURPLE);
        tft->setTextColor(COLOR_WHITE);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("Content", centerX + centerX/2, 120, 2);
    }

    // ==================== TEMPLATE 7: PROGRESS VIEW ====================
    // Perfect for: Loading, Upload, Download, Processing
    void drawTemplate_Progress(const char* title, const char* operation, int percent) {
        drawFrame(title, "Progress View Template");

        // Operation text
        tft->setTextColor(COLOR_WHITE);
        tft->setTextDatum(TC_DATUM);
        tft->drawString(operation, 120, 100, 2);

        // Large percentage
        tft->setTextColor(COLOR_HOT_PINK);
        char percentText[8];
        sprintf(percentText, "%d%%", percent);
        tft->drawString(percentText, 120, 140, 6);

        // Progress bar (large)
        int barWidth = 200;
        int barHeight = 20;
        int barX = (SCREEN_WIDTH - barWidth) / 2;
        int barY = 190;

        // Background
        tft->fillRoundRect(barX, barY, barWidth, barHeight, 10, COLOR_DARK_GRAY);

        // Fill
        int fillWidth = (barWidth * percent) / 100;
        if (fillWidth > 0) {
            tft->fillRoundRect(barX, barY, fillWidth, barHeight, 10, COLOR_VIOLET);
        }

        // Border
        tft->drawRoundRect(barX, barY, barWidth, barHeight, 10, COLOR_ELECTRIC_BLUE);

        // Status text
        tft->setTextColor(COLOR_AMBER);
        tft->setTextDatum(TC_DATUM);
        tft->drawString("Please wait...", 120, 230, 1);
    }

    // ==================== TEMPLATE 8: CHART VIEW ====================
    // Perfect for: Analytics, Graphs, Stats over time
    void drawTemplate_Chart(const char* title, int* dataPoints, int dataCount, int maxValue) {
        drawFrame(title, "Chart View Template");

        // Chart area
        int chartX = SPACE_MD;
        int chartY = 70;
        int chartW = 200;
        int chartH = 140;

        // Draw axes
        tft->drawFastVLine(chartX, chartY, chartH, COLOR_VIOLET);
        tft->drawFastHLine(chartX, chartY + chartH, chartW, COLOR_VIOLET);

        // Draw bars
        int barWidth = chartW / (dataCount + 1);

        for (int i = 0; i < dataCount && i < 10; i++) {
            int x = chartX + 10 + (i * barWidth);
            int barHeight = (dataPoints[i] * chartH) / maxValue;
            int y = chartY + chartH - barHeight;

            // Gradient bar (2 colors)
            uint16_t topColor = COLOR_HOT_PINK;
            uint16_t bottomColor = COLOR_VIOLET;

            for (int h = 0; h < barHeight; h += 2) {
                float ratio = (float)h / barHeight;
                uint16_t r1 = (topColor >> 11) & 0x1F;
                uint16_t g1 = (topColor >> 5) & 0x3F;
                uint16_t b1 = topColor & 0x1F;
                uint16_t r2 = (bottomColor >> 11) & 0x1F;
                uint16_t g2 = (bottomColor >> 5) & 0x3F;
                uint16_t b2 = bottomColor & 0x1F;

                int r = r1 + (r2 - r1) * ratio;
                int g = g1 + (g2 - g1) * ratio;
                int b = b1 + (b2 - b1) * ratio;

                uint16_t color = (r << 11) | (g << 5) | b;
                tft->drawFastHLine(x, y + h, barWidth - 4, color);
            }
        }

        // Y-axis labels
        tft->setTextColor(COLOR_AMBER);
        tft->setTextDatum(MR_DATUM);
        tft->drawString(String(maxValue), chartX - 5, chartY, 1);
        tft->drawString("0", chartX - 5, chartY + chartH, 1);
    }

    // ==================== TEMPLATE 9: EMPTY STATE ====================
    // Perfect for: No data, No results, Placeholder
    void drawTemplate_Empty(const char* title, const char* message, const char* actionText) {
        drawFrame(title, "Empty State Template");

        // Large icon
        tft->drawCircle(120, 130, 40, COLOR_VIOLET);
        tft->drawCircle(120, 130, 38, COLOR_VIOLET);
        tft->setTextColor(COLOR_AMBER);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("?", 120, 130, 6);

        // Message
        tft->setTextColor(COLOR_WHITE);
        tft->setTextDatum(TC_DATUM);
        tft->drawString(message, 120, 190, 2);

        // Action button
        if (actionText) {
            tft->fillRoundRect(SPACE_LG, 230, 170, 35, 8, COLOR_HOT_PINK);
            tft->setTextColor(COLOR_WHITE);
            tft->setTextDatum(MC_DATUM);
            tft->drawString(actionText, 120, 247, 2);
        }
    }

    // ==================== TEMPLATE 10: TABS VIEW ====================
    // Perfect for: Multi-section apps, Settings categories
    void drawTemplate_Tabs(const char* title, const char** tabLabels, int tabCount, int activeTab) {
        drawFrame(title, "Tabs View Template");

        // Tab bar
        int tabWidth = SCREEN_WIDTH / tabCount;

        for (int i = 0; i < tabCount && i < 4; i++) {
            int x = i * tabWidth;

            // Tab background
            uint16_t bgColor = (i == activeTab) ? COLOR_VIOLET : COLOR_DARK_GRAY;
            tft->fillRect(x, 50, tabWidth, 30, bgColor);

            // Tab border
            tft->drawRect(x, 50, tabWidth, 30, COLOR_ELECTRIC_BLUE);

            // Tab label
            uint16_t textColor = (i == activeTab) ? COLOR_WHITE : COLOR_AMBER;
            tft->setTextColor(textColor);
            tft->setTextDatum(MC_DATUM);
            tft->drawString(tabLabels[i], x + tabWidth/2, 65, 1);

            // Active indicator
            if (i == activeTab) {
                tft->fillRect(x + 5, 77, tabWidth - 10, 3, COLOR_HOT_PINK);
            }
        }

        // Content area placeholder
        tft->drawRoundRect(SPACE_SM, 90, 220, 170, 8, COLOR_VIOLET);
        tft->setTextColor(COLOR_AMBER);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("Tab Content Here", 120, 175, 2);
    }
};

#endif // WIREFRAME_TEMPLATES_H
