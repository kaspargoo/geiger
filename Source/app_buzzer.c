#include "app_buzzer.h"
#include "OSAL.h"
#include "hal_mcu.h"

/*
 * BUZZER_P1_PIN (preinclude.h) selects the port 1 pin of the buzzer.
 * Timer 3 runs in modulo mode and its overflow interrupt toggles the pin, so any
 * port 1 pin works. The interrupt only runs while a tone is playing.
 */
#ifndef BUZZER_P1_PIN
#error "BUZZER_P1_PIN must be defined"
#endif
#define BUZZER_PIN_SBIT_(n) P1_##n
#define BUZZER_PIN_SBIT_X(n) BUZZER_PIN_SBIT_(n)
#define BUZZER_PIN_SBIT BUZZER_PIN_SBIT_X(BUZZER_P1_PIN)
#define BUZZER_PIN_BV BV(BUZZER_P1_PIN)

#define T3CTL_DIV16 0x80
#define T3CTL_DIV32 0xA0
#define T3CTL_START 0x10
#define T3CTL_OVFIM 0x08
#define T3CTL_CLEAR 0x04
#define T3CTL_MODULO 0x02

/*
 * Tones. f = 32 MHz / (div * (CC0 + 1) * 2) (one toggle per timer period halves the frequency)
 */
enum { TONE_OFF = 0, TONE_4K, TONE_3K, TONE_2K };
static const uint8 toneDiv[] = {0, T3CTL_DIV16, T3CTL_DIV32, T3CTL_DIV32};
static const uint8 toneCC0[] = {0, 249, 166, 249}; // 4000 Hz, 2994 Hz, 2000 Hz

typedef struct {
    uint8 tone;
    uint8 ms;
} note_t;

static const note_t clickNotes[] = {{TONE_4K, 3}};
static const note_t alarmNotes[] = {{TONE_4K, 150}, {TONE_OFF, 80}, {TONE_3K, 150}, {TONE_OFF, 80},
                                    {TONE_4K, 150}, {TONE_OFF, 80}, {TONE_3K, 150}};
static const note_t confirmOnNotes[] = {{TONE_2K, 60}, {TONE_OFF, 30}, {TONE_4K, 80}};
static const note_t confirmOffNotes[] = {{TONE_4K, 60}, {TONE_OFF, 30}, {TONE_2K, 80}};

static uint8 buzzerTaskId;
static uint16 buzzerEvent;
static const note_t *melody = NULL;
static uint8 melodyLen = 0;
static uint8 melodyPos = 0;

HAL_ISR_FUNCTION(buzzerTimer3Isr, T3_VECTOR) {
    T3OVFIF = 0;
    T3IF = 0;
    BUZZER_PIN_SBIT ^= 1;
}

static void buzzerOff(void) {
    T3CTL = 0;
    T3IE = 0;
    BUZZER_PIN_SBIT = 0; // keep the transistor closed between notes
}

static void buzzerToneOn(uint8 tone) {
    buzzerOff();
    if (tone == TONE_OFF) {
        return;
    }
    T3CC0 = toneCC0[tone];
    T3CTL = toneDiv[tone] | T3CTL_MODULO | T3CTL_OVFIM | T3CTL_CLEAR;
    T3OVFIF = 0;
    T3IF = 0;
    T3IE = 1;
    T3CTL |= T3CTL_START;
}

static void buzzerPlay(const note_t *notes, uint8 len) {
    osal_stop_timerEx(buzzerTaskId, buzzerEvent);
    melody = notes;
    melodyLen = len;
    melodyPos = 0;
    Buzzer_ProcessEvent();
}

void Buzzer_Init(uint8 taskId, uint16 event) {
    buzzerTaskId = taskId;
    buzzerEvent = event;
    P1SEL &= ~BUZZER_PIN_BV; // GPIO
    P1DIR |= BUZZER_PIN_BV;
    buzzerOff();
}

void Buzzer_ProcessEvent(void) {
    if (melody == NULL || melodyPos >= melodyLen) {
        Buzzer_Stop();
        return;
    }
    buzzerToneOn(melody[melodyPos].tone);
    osal_start_timerEx(buzzerTaskId, buzzerEvent, melody[melodyPos].ms);
    melodyPos++;
}

void Buzzer_Stop(void) {
    osal_stop_timerEx(buzzerTaskId, buzzerEvent);
    melody = NULL;
    buzzerOff();
}

bool Buzzer_IsBusy(void) { return melody != NULL; }

void Buzzer_Click(void) {
    if (!Buzzer_IsBusy()) {
        buzzerPlay(clickNotes, sizeof(clickNotes) / sizeof(note_t));
    }
}

void Buzzer_Alarm(void) { buzzerPlay(alarmNotes, sizeof(alarmNotes) / sizeof(note_t)); }

void Buzzer_Confirm(bool on) {
    if (on) {
        buzzerPlay(confirmOnNotes, sizeof(confirmOnNotes) / sizeof(note_t));
    } else {
        buzzerPlay(confirmOffNotes, sizeof(confirmOffNotes) / sizeof(note_t));
    }
}
