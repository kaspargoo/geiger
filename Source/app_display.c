#include "app_display.h"
#include "OSAL.h"
#include "oled.h"
#include "version.h"

/*
 * 128x32 layout (pages are 8 px rows):
 *
 *   x 0........................95  96......127
 *   p0-1  "  1234" (8x16)  "~"/"uR/h"  | logo 32x32
 *   p2    "CPM 21"                     | (inverted
 *   p3    "Zigbee OK"   "CLK"          |  on alert)
 */

#define LOGO_X 96
#define LOGO_W 32
#define TEXT_W 96
#define VALUE_CHARS 6 // up to 130000 uR/h
#define UNIT_X (VALUE_CHARS * 8 + 2)

// 32x32 radiation sign, vertical bytes, page by page
static const uint8 logo32[] = {
    0x00, 0x00, 0x80, 0xE0, 0xF0, 0xF8, 0xFC, 0xFC, 0xF8, 0xE0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xE0, 0xF8, 0xFC, 0xFC, 0xF8, 0xF0, 0xE0, 0x80, 0x00, 0x00,
    0x70, 0x7E, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x1E, 0x0C, 0xC0, 0xE0, 0xE0,
    0xE0, 0xE0, 0xC0, 0x0C, 0x1E, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7E, 0x70,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xF1, 0xF3, 0xF3,
    0xF3, 0xF3, 0xF1, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x3E, 0x3F, 0x3F, 0x3F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x3F, 0x3F, 0x3F, 0x3E, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static bool displayPresent = FALSE;
static bool displayOn = FALSE;
static display_data_t shown;  // what is currently on the panel
static bool shownValid = FALSE;

/* Right-aligned unsigned to text, padded with spaces to `width` chars. buf must hold width+1 */
static void formatUint(char *buf, uint8 width, uint32 value) {
    uint8 i = width;
    buf[width] = '\0';
    do {
        buf[--i] = '0' + (char)(value % 10);
        value /= 10;
    } while (value && i);
    while (i) {
        buf[--i] = ' ';
    }
}

static void drawLogo(bool inverted) {
    uint8 tmp[LOGO_W];
    uint8 p, i;
    if (!inverted) {
        OLED_DrawBitmap(LOGO_X, 0, LOGO_W, 4, logo32);
        return;
    }
    for (p = 0; p < 4; p++) {
        for (i = 0; i < LOGO_W; i++) {
            tmp[i] = ~logo32[p * LOGO_W + i];
        }
        OLED_DrawBitmap(LOGO_X, p, LOGO_W, 1, tmp);
    }
}

static void drawValue(const display_data_t *d) {
    char buf[VALUE_CHARS + 1];
    formatUint(buf, VALUE_CHARS, d->urh);
    OLED_DrawString8x16(0, 0, buf);
    OLED_DrawString6x8(UNIT_X, 0, d->estimate ? "~   " : "    ");
    OLED_DrawString6x8(UNIT_X, 1, "uR/h");
}

static void drawCpm(const display_data_t *d) {
    char buf[7];
    formatUint(buf, 6, d->cpm);
    OLED_DrawString6x8(0, 2, "CPM");
    OLED_DrawString6x8(18, 2, buf); // "CPM" + 6 right-aligned digits = 9 chars
}

static void drawStatus(const display_data_t *d) {
    const char *net;
    if (d->alert) {
        net = "! ALERT !  ";
    } else {
        switch (d->net) {
        case NET_ONLINE:
            net = "Zigbee OK  ";
            break;
        case NET_JOINING:
            net = "Joining... ";
            break;
        case NET_LOST:
            net = "Rejoining..";
            break;
        default:
            net = "No network ";
            break;
        }
    }
    OLED_DrawString6x8(0, 3, net); // 11 chars = 66 px
    OLED_DrawString6x8(72, 3, d->clicks ? "CLK" : "   ");
}

bool Display_Init(void) {
    displayPresent = OLED_Probe();
    if (!displayPresent) {
        return FALSE;
    }
    OLED_Init(); // clears RAM while the panel is still off
    OLED_DrawBitmap(0, 0, LOGO_W, 4, logo32);
    OLED_DrawString8x16(40, 0, "Geiger");
    OLED_DrawString6x8(LOGO_W, 2, APP_HW_NAME);
    OLED_DrawString6x8(LOGO_W, 3, zclApp_DateCodeNT); // build date = firmware version
    shownValid = FALSE;
    Display_Power(TRUE);
    return TRUE;
}

bool Display_IsPresent(void) { return displayPresent; }

void Display_Power(bool on) {
    if (!displayPresent) {
        return;
    }
    if (on) {
        OLED_On();
    } else {
        OLED_Off();
    }
    displayOn = on;
}

bool Display_IsOn(void) { return displayOn; }

void Display_ShowMain(const display_data_t *data) {
    if (!displayPresent) {
        return;
    }
    OLED_Clear();
    drawLogo(data->alert);
    drawValue(data);
    drawCpm(data);
    drawStatus(data);
    shown = *data;
    shownValid = TRUE;
}

void Display_Update(const display_data_t *data) {
    if (!displayPresent) {
        return;
    }
    if (!shownValid) {
        Display_ShowMain(data);
        return;
    }
    if (data->urh != shown.urh || data->estimate != shown.estimate) {
        drawValue(data);
    }
    if (data->cpm != shown.cpm) {
        drawCpm(data);
    }
    if (data->net != shown.net || data->alert != shown.alert || data->clicks != shown.clicks) {
        drawStatus(data);
    }
    if (data->alert != shown.alert) {
        drawLogo(data->alert);
    }
    shown = *data;
}
