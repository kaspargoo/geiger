#include "oled.h"
#include "OnBoard.h"
#include "hal_i2c.h"
#include "oledfont.h"

#define OLED_I2C_ADDR 0x3C
#define OLED_CTRL_CMD 0x00
#define OLED_CTRL_DATA 0x40
#define OLED_CHUNK 16

/*
 * SSD1306 128x32 init sequence (matches the Adafruit trace in the hardware notes).
 * Page addressing mode is set explicitly: the display is not reset together with the
 * MCU, so it must not depend on whatever state the previous firmware left behind.
 */
static const uint8 oledInitCmds[] = {
    0xAE,       // display off
    0xD5, 0x80, // oscillator frequency
    0xA8, 0x1F, // multiplex ratio: 32 rows
    0xD3, 0x00, // display offset 0
    0x40,       // start line 0
    0x8D, 0x14, // charge pump on
    0x20, 0x02, // page addressing mode
    0xA1,       // segment remap (column 127 -> SEG0)
    0xC8,       // COM scan direction remapped
    0xDA, 0x02, // COM pins config for 128x32
    0x81, 0x8F, // contrast
    0xD9, 0xF1, // pre-charge period
    0xDB, 0x40, // VCOMH deselect level
    0xA4,       // display follows RAM
    0xA6,       // normal (not inverted)
    0x2E,       // no scrolling
};

static void oledSend(uint8 ctrl, const uint8 *buf, uint8 len) {
    // the I2C API takes a non-const pointer but only reads from it
    I2C_WriteMultByte(OLED_I2C_ADDR, ctrl, (uint8 *)buf, len);
}

static void oledSetPos(uint8 x, uint8 page) {
    uint8 cmd[3];
    cmd[0] = 0xB0 | (page & 0x07); // mask: an out-of-range page must never turn into another command
    cmd[1] = 0x10 | (x >> 4);
    cmd[2] = x & 0x0F;
    oledSend(OLED_CTRL_CMD, cmd, sizeof(cmd));
}

bool OLED_Probe(void) {
    static const uint8 displayOff = 0xAE;
    HalI2CInit();
    // without a display nobody pulls SDA low for the address ACK (the pins have pull-ups)
    return I2C_WriteMultByte(OLED_I2C_ADDR, OLED_CTRL_CMD, (uint8 *)&displayOff, 1) == I2C_SUCCESS;
}

void OLED_Init(void) {
    HalI2CInit();
    oledSend(OLED_CTRL_CMD, oledInitCmds, sizeof(oledInitCmds));
    OLED_Clear();
}

void OLED_On(void) {
    static const uint8 cmd[] = {0x8D, 0x14, 0xAF};
    oledSend(OLED_CTRL_CMD, cmd, sizeof(cmd));
}

void OLED_Off(void) {
    static const uint8 cmd[] = {0xAE, 0x8D, 0x10};
    oledSend(OLED_CTRL_CMD, cmd, sizeof(cmd));
}

void OLED_FillArea(uint8 x, uint8 page, uint8 width, uint8 pages, uint8 pattern) {
    uint8 buf[OLED_CHUNK];
    uint8 i, n;
    for (i = 0; i < OLED_CHUNK; i++) {
        buf[i] = pattern;
    }
    for (; pages > 0 && page < OLED_PAGES; pages--, page++) {
        oledSetPos(x, page);
        for (i = 0; i < width; i += n) {
            n = width - i;
            if (n > OLED_CHUNK) {
                n = OLED_CHUNK;
            }
            oledSend(OLED_CTRL_DATA, buf, n);
        }
    }
}

void OLED_ClearArea(uint8 x, uint8 page, uint8 width, uint8 pages) { OLED_FillArea(x, page, width, pages, 0x00); }

void OLED_Clear(void) { OLED_FillArea(0, 0, OLED_WIDTH, OLED_PAGES, 0x00); }

/* Bitmap format: vertical bytes, page by page (LCD Assistant "vertical"), width*pages bytes */
void OLED_DrawBitmap(uint8 x, uint8 page, uint8 width, uint8 pages, const uint8 *bmp) {
    uint8 p, i, n;
    for (p = 0; p < pages && (page + p) < OLED_PAGES; p++) {
        oledSetPos(x, page + p);
        for (i = 0; i < width; i += n) {
            n = width - i;
            if (n > OLED_CHUNK) {
                n = OLED_CHUNK;
            }
            oledSend(OLED_CTRL_DATA, bmp + (uint16)p * width + i, n);
        }
    }
}

static uint8 oledGlyphIndex(char c) {
    if (c < ' ' || c > '~') {
        c = '?';
    }
    return (uint8)(c - ' ');
}

uint8 OLED_DrawString6x8(uint8 x, uint8 page, const char *str) {
    oledSetPos(x, page);
    while (*str && x <= OLED_WIDTH - 6) {
        oledSend(OLED_CTRL_DATA, F6x8[oledGlyphIndex(*str)], 6);
        x += 6;
        str++;
    }
    return x;
}

uint8 OLED_DrawString8x16(uint8 x, uint8 page, const char *str) {
    uint8 half, cx;
    const char *s;
    for (half = 0; half < 2; half++) {
        oledSetPos(x, page + half);
        cx = x;
        for (s = str; *s && cx <= OLED_WIDTH - 8; s++, cx += 8) {
            oledSend(OLED_CTRL_DATA, &F8x16[((uint16)oledGlyphIndex(*s) << 4) + (half << 3)], 8);
        }
    }
    return cx;
}
