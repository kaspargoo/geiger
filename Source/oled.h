#ifndef OLED_H
#define OLED_H

/*
 * Minimal SSD1306 128x32 driver over the bit-banged I2C from zstack-lib/hal_i2c.c.
 * There is no frame buffer (RAM is tight): everything is drawn straight into the
 * display RAM. Coordinates: x = column 0..127, page = 8-pixel row 0..3.
 */

#include "hal_types.h"

#define OLED_WIDTH 128
#define OLED_PAGES 4

bool OLED_Probe(void); // TRUE if a display ACKs its I2C address
void OLED_Init(void);
void OLED_On(void);
void OLED_Off(void);
void OLED_Clear(void);
void OLED_ClearArea(uint8 x, uint8 page, uint8 width, uint8 pages);
void OLED_DrawBitmap(uint8 x, uint8 page, uint8 width, uint8 pages, const uint8 *bmp);
void OLED_FillArea(uint8 x, uint8 page, uint8 width, uint8 pages, uint8 pattern);

/* Return the x position right after the drawn text */
uint8 OLED_DrawString6x8(uint8 x, uint8 page, const char *str);
uint8 OLED_DrawString8x16(uint8 x, uint8 page, const char *str);

#endif
