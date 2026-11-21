#define TC_LINKKEY_JOIN
#define NV_INIT
#define NV_RESTORE

#define TP2_LEGACY_ZC
// patch sdk
// #define ZDSECMGR_TC_ATTEMPT_DEFAULT_KEY TRUE

#define NWK_AUTO_POLL
#define MULTICAST_ENABLED FALSE

#define ZCL_READ
#define ZCL_WRITE
#define ZCL_BASIC
#define ZCL_IDENTIFY
#define ZCL_REPORTING_DEVICE
#define ZCL_ON_OFF

#define DISABLE_GREENPOWER_BASIC_PROXY
#define BDB_FINDING_BINDING_CAPABILITY_ENABLED 1
#define BDB_REPORTING TRUE

#ifdef DEFAULT_CHANLIST
#undef DEFAULT_CHANLIST
#endif
#define DEFAULT_CHANLIST 0x07FFF800

#define HAL_BUZZER FALSE
#define HAL_KEY FALSE

#define HAL_LED TRUE
#define HAL_ADC FALSE
#define HAL_LCD FALSE

#define BLINK_LEDS TRUE

// one of this boards
// #define HAL_BOARD_TARGET
// #define HAL_BOARD_CHDTECH_DEV

#if !defined(HAL_BOARD_TARGET) && !defined(HAL_BOARD_CHDTECH_DEV)
#error "Board type must be defined"
#endif

// Buzzer pin on port 1: P1.5 on the DIYRuZ board (passive piezo through a transistor)
#define BUZZER_P1_PIN 5

// Optional OLED SSD1306 128x32 (I2C address 0x3C) on bit-banged I2C, detected at boot
#define OCM_CLK_PORT 0 // SCL = P0.5
#define OCM_CLK_PIN 5
#define OCM_DATA_PORT 0 // SDA = P0.6
#define OCM_DATA_PIN 6

// Optional button to GND, polled, internal pull-up
#define APP_BUTTON_SBIT P1_2 // comment out to build without the button
#define APP_BUTTON_BV BV(2)
#define APP_BUTTON_SEL P1SEL
#define APP_BUTTON_DIR P1DIR
#define APP_BUTTON_INP P1INP

#if defined(HAL_BOARD_TARGET)
#define APP_TX_POWER TX_PWR_PLUS_19
#define APP_HW_NAME "DIYRuZ" // shown on the boot screen
#elif defined(HAL_BOARD_CHDTECH_DEV)
#define APP_TX_POWER TX_PWR_PLUS_4
#define APP_HW_NAME "CHDTECH dev"
#define DO_DEBUG_UART
// #define DO_DEBUG_MT
#endif



#ifdef DO_DEBUG_UART
#define HAL_UART TRUE
#define HAL_UART_DMA 1
#define INT_HEAP_LEN (2685 - 0x4B - 0xBB - 0x50) // - 0x50: room for the application state
#endif

#ifdef DO_DEBUG_MT
#define HAL_UART TRUE
#define HAL_UART_DMA 1
#define HAL_UART_ISR 2
#define INT_HEAP_LEN (2685 - 0x4B - 0xBB)

#define MT_TASK

#define MT_UTIL_FUNC
#define MT_UART_DEFAULT_BAUDRATE HAL_UART_BR_115200
#define MT_UART_DEFAULT_OVERFLOW FALSE

#define ZTOOL_P1

#define MT_APP_FUNC
#define MT_APP_CNF_FUNC
#define MT_SYS_FUNC
#define MT_ZDO_FUNC
#define MT_ZDO_MGMT
#define MT_DEBUG_FUNC

#endif


#include "hal_board_cfg.h"
