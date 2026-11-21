#ifndef APP_BUZZER_H
#define APP_BUZZER_H

/*
 * Passive piezo buzzer (PKLCS1212E4001, resonance ~4 kHz) driven through a transistor.
 * The tone comes from Timer 3, the pin is set by BUZZER_P1_PIN in preinclude.h.
 * Non-blocking: note timing is done with an OSAL timer event of the app task.
 */

#include "hal_types.h"

void Buzzer_Init(uint8 taskId, uint16 event);
void Buzzer_ProcessEvent(void);

void Buzzer_Click(void);         // ~3 ms tick, skipped if something else is playing
void Buzzer_Alarm(void);         // ~1 s two-tone alarm
void Buzzer_Confirm(bool on);    // short rising (on) / falling (off) chirp
void Buzzer_Stop(void);
bool Buzzer_IsBusy(void);

#endif
