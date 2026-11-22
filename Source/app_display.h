#ifndef APP_DISPLAY_H
#define APP_DISPLAY_H

#include "hal_types.h"

typedef enum { NET_NONE = 0, NET_JOINING, NET_ONLINE, NET_LOST } display_net_t;

typedef struct {
    uint32 urh;         // dose rate, uR/h
    uint16 cpm;         // counts per minute
    display_net_t net;  // zigbee connection
    bool estimate;      // window not full yet (first minute after boot)
    bool alert;         // threshold exceeded
    bool clicks;        // click sound enabled
} display_data_t;

/*
 * The display is optional. Display_Init() probes the I2C bus once; when nothing answers,
 * every other call is a no-op and the bus is never touched again.
 */
bool Display_Init(void);                   // init controller and show the splash; FALSE if no display
bool Display_IsPresent(void);
void Display_Power(bool on);               // panel on/off (charge pump off when off)
bool Display_IsOn(void);
void Display_ShowMain(const display_data_t *data); // full redraw of the main screen
void Display_Update(const display_data_t *data);   // redraw only what changed

#endif
