/* 
 * File:   main.c
 * Author: e2310530
 *
 * Created on May 8, 2022, 3:26 PM
 */

#include "device.h"

void tmr_isr();
void __interrupt(high_priority) highPriorityISR(void) {
    if (INTCONbits.TMR0IF) tmr_isr();
}
void __interrupt(low_priority) lowPriorityISR(void) {}

void init_ports()
{
    ADCON1 = 0xf; // All ports digital
    TRISA = 0x00; 
    TRISB = 0x00;
    TRISC = 0xff; // RC0 is input in idle
    TRISD = 0x00;
    TRISE = 0x00;
    TRISF = 0x00;
    TRISG = 0xff; // PORTG is input
}
void init_irq(){
    INTCONbits.TMR0IE = 1;
    INTCONbits.GIE = 1;
}

// ************* Timer task and functions ****************
typedef enum {TMR_IDLE, TMR_RUN, TMR_DONE} tmr_state_t;
tmr_state_t tmr_state = TMR_IDLE;   // Current timer state
uint8_t tmr_startreq = 0;           // Flag to request the timer to start
uint8_t tmr_ticks_left;             // Number of "ticks" until "done"
uint8_t tmr_500ms = 99, tmr_400ms = 79, tmr_300ms = 59;

// 500ms: Preload 39 Tick: 99
// 400ms: Preload 39 Tick: 79
// 300ms: Preload 39 Tick: 59

#define TIMER0_PRELOAD 39

void tmr_preload() {
    TMR0L = TIMER0_PRELOAD & 0xff;    
}
void tmr_isr(){
    INTCONbits.TMR0IF = 0;
    if (--tmr_ticks_left == 0)  
        tmr_state = TMR_DONE;
    else tmr_preload();
}
void tmr_init() {
    // In order to achieve a 500ms delay, we will use Timer0 in 16-bit mode.
    // This setup assumes a 40MHz 18F8722, which corresponds to a 10MHz
    // instruction cycle
    T0CON = 0x47; // internal clock with 1:256 prescaler 
    tmr_preload();
}
void tmr1_init() {
    T1CON = 0xc1;
}

void tmr_start(uint8_t ticks) {
    tmr_ticks_left = ticks;
    tmr_startreq = 1;
    tmr_state = TMR_IDLE;
}
// This function aborts the current timer run and goes back to IDLE
void tmr_abort() {
    T0CON &= 0x7f; // Unset TMR0ON
    tmr_startreq = 0;
    tmr_state = TMR_IDLE;
}
// This is the timer task
void timer_task() {
    static uint16_t tmr_count = 0;  // Current timer count, static local var.
    switch (tmr_state) {
        case TMR_IDLE:
            if (tmr_startreq) {
                // If a start request has been issued, go to the RUN state
                tmr_startreq = 0;
                tmr_preload();
                INTCONbits.T0IF = 0;
                T0CON |= 0x80; // Set TMR0ON
                tmr_state = TMR_RUN;
            }
            break;
        case TMR_RUN:
            // Timer remains in the RUN state until the counter reaches its max
            // "ticks" number of times.

            // This part is "polling" the TMR0 flag. Unused if interrupts 
            // are enabled
            if (INTCONbits.T0IF) {
                INTCONbits.T0IF = 0;
                if (--tmr_ticks_left == 0) 
                    tmr_state = TMR_DONE;
                else tmr_preload();
            }
            break;
        case TMR_DONE:
            // State waits here until tmr_start() or tmr_abort() is called
            break;
    }
}

// ************* Input task and functions ****************
// The "input task" monitors RA4 and RE4 and increments associated counters 
// whenever a high pulse is observed (i.e. HIGH followed by a LOW).
uint8_t inp_rc0_cnt = 0; // Current count for CONFIGURE input(i.e. RA4)
uint8_t inp_rg0_cnt = 0;
uint8_t inp_rg1_cnt = 0;
uint8_t inp_rg2_cnt = 0;
uint8_t inp_rg3_cnt = 0;
uint8_t inp_rg4_cnt = 0;
uint8_t inp_rc0_st = 0, inp_rg0_st = 0, inp_rg1_st = 0, inp_rg2_st = 0, inp_rg3_st = 0, inp_rg4_st = 0;

// This function resets the counter for PORT SELECT input 
void inp_rc0_reset() { inp_rc0_cnt = 0; }
void inp_rg0_reset() { inp_rg0_cnt = 0; }
void inp_rg1_reset() { inp_rg1_cnt = 0; }
void inp_rg2_reset() { inp_rg2_cnt = 0; }
void inp_rg3_reset() { inp_rg3_cnt = 0; }
void inp_rg4_reset() { inp_rg4_cnt = 0; }

// This is the input task function
void input_task() {
    if (PORTCbits.RC0) inp_rc0_st = 1;
    else if (inp_rc0_st == 1) {
        // A high pulse has been observed on the PORT input
        inp_rc0_st = 0;
        inp_rc0_cnt++;
    }
    if (PORTGbits.RG0) inp_rg0_st = 1;
    else if (inp_rg0_st == 1) {
        // A high pulse has been observed on the CONFIGURE input
        inp_rg0_st = 0;
        inp_rg0_cnt++;
    }
    if (PORTGbits.RG1) inp_rg1_st = 1;
    else if (inp_rg1_st == 1) {
        // A high pulse has been observed on the CONFIGURE input
        inp_rg1_st = 0;
        inp_rg1_cnt++;
    }
    if (PORTGbits.RG2) inp_rg2_st = 1;
    else if (inp_rg2_st == 1) {
        // A high pulse has been observed on the CONFIGURE input
        inp_rg2_st = 0;
        inp_rg2_cnt++;
    }
    if (PORTGbits.RG3) inp_rg3_st = 1;
    else if (inp_rg3_st == 1) {
        // A high pulse has been observed on the CONFIGURE input
        inp_rg3_st = 0;
        inp_rg3_cnt++;
    }
    if (PORTGbits.RG4) inp_rg4_st = 1;
    else if (inp_rg4_st == 1) {
        // A high pulse has been observed on the CONFIGURE input
        inp_rg4_st = 0;
        inp_rg4_cnt++;
    }
}

// ************* Display task and functions ****************
uint8_t dsp_porta = 0x00, dsp_portb = 0x00, dsp_portc = 0x00, dsp_portd = 0x00, dsp_porte = 0x00, dsp_portf = 0x00;
uint8_t dsp_updatereq = 1;

void dsp_update_ports()
{
    PORTA = dsp_porta;
    PORTB = dsp_portb;
    PORTC = dsp_portc;
    PORTD = dsp_portd;
    PORTE = dsp_porte;
    PORTF = dsp_portf;
}

void dsp_clear()
{
    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    PORTD = 0x00;
    PORTE = 0x00;
    PORTF = 0x00;
}

void display_task() {

    if (dsp_updatereq)
    {
        dsp_updatereq = 0;
        dsp_update_ports();
        return;
    }
}

// ************* Game task and functions ****************
// This task handles the overall game logic and control remaining tasks 
// through their utility functions and flags

// Game state definitions and the global state
typedef enum {G_INIT,G_LEVEL1,G_LEVEL2,G_LEVEL3,G_END,G_FAIL} game_state_t;
game_state_t game_state = G_INIT;
// Current game choices and the countdown
uint8_t game_level = 1, game_health = 9, game_notes_gnr = 0, game_notes_rmn = 0, rng_shift = 1;
uint16_t game_rng;

// This will be called after push so it inserts val to PORTA
void generate_note() {
    uint8_t rmtb = game_rng & 0x07; //right most three bits
    uint8_t val = rmtb % 5;
    game_rng = (game_rng >> rng_shift) | (game_rng << (16 - rng_shift));
    // Write val to PORTA
    dsp_porta = 1 << val;
}

void push_notes() {
    if (dsp_portf != 0x00)
    {
        game_health--;
        game_notes_rmn--;
    }
    dsp_portf = dsp_porte;
    dsp_porte = dsp_portd;
    dsp_portd = dsp_portc;
    dsp_portc = dsp_portb;
    dsp_portb = dsp_porta;
    dsp_porta = 0x00;
    
    dsp_updatereq = 1;
}

void check_input() {
    if (inp_rg0_cnt != 0)
    {
        inp_rg0_reset();
        if (!PORTFbits.RF0)
        {
            game_health--;
        }
        else
        {
            dsp_portf = 0x00;
            dsp_updatereq = 1;
            game_notes_rmn--;
        }
    }
    if (inp_rg1_cnt != 0)
    {
        inp_rg1_reset();
        if (!PORTFbits.RF1)
        {
            game_health--;
        }
        else
        {
            dsp_portf = 0x00;
            dsp_updatereq = 1;
            game_notes_rmn--;
        }
    }
    if (inp_rg2_cnt != 0)
    {
        inp_rg2_reset();
        if (!PORTFbits.RF2)
        {
            game_health--;
        }
        else
        {
            dsp_portf = 0x00;
            dsp_updatereq = 1;
            game_notes_rmn--;
        }
    }
    if (inp_rg3_cnt != 0)
    {
        inp_rg3_reset();
        if (!PORTFbits.RF3)
        {
            game_health--;
        }
        else
        {
            dsp_portf = 0x00;
            dsp_updatereq = 1;
            game_notes_rmn--;
        }
    }
    if (inp_rg4_cnt != 0)
    {
        inp_rg4_reset();
        if (!PORTFbits.RF4)
        {
            game_health--;
        }
        else
        {
            dsp_portf = 0x00;
            dsp_updatereq = 1;
            game_notes_rmn--;
        }
    }
}

void game_task() {
    switch (game_state) {
        case G_INIT:
            if (inp_rc0_cnt != 0)
            {
                inp_rc0_reset();
                TRISC = 0x00; // PORTC is output in game
                game_rng = (TMR1H << 8) + TMR1L;
                // Level 1 init
                game_state = G_LEVEL1;
                game_level = 1;
                game_notes_gnr = 5;
                game_notes_rmn = 5;
                tmr_start(tmr_500ms);
            }
            break;
        case G_LEVEL1:
            if (game_health <= 0)
            {
                game_state = G_FAIL;
                return;
            }
            if (game_notes_rmn == 0)
            {
                game_state = G_LEVEL2;
                game_level = 2;
                game_notes_gnr = 10;
                game_notes_rmn = 10;
                tmr_start(tmr_400ms);
                return;
            }
            check_input();
            if (tmr_state == TMR_DONE)
            {
                push_notes();
                if (game_notes_gnr != 0)
                {
                    game_notes_gnr--;
                    generate_note();
                }
                tmr_start(tmr_500ms);
            }
            break;
        case G_LEVEL2:
            if (game_health <= 0)
            {
                game_state = G_FAIL;
                return;
            }
            if (game_notes_rmn == 0)
            {
                game_state = G_LEVEL3;
                game_level = 3;
                game_notes_gnr = 15;
                game_notes_rmn = 15;
                tmr_start(tmr_300ms);
                return;
            }
            check_input();
            if (tmr_state == TMR_DONE)
            {
                push_notes();
                if (game_notes_gnr != 0)
                {
                    game_notes_gnr--;
                    generate_note();
                }
                tmr_start(tmr_400ms);
            }
            break;
        case G_LEVEL3:
            if (game_health <= 0)
            {
                game_state = G_FAIL;
                return;
            }
            if (game_notes_rmn == 0)
            {
                game_state = G_END;
                TRISC = 0xff; // Set PORTC as input
            }
            check_input();
            if (tmr_state == TMR_DONE)
            {
                push_notes();
                if (game_notes_gnr != 0)
                {
                    game_notes_gnr--;
                    generate_note();
                }
                tmr_start(tmr_300ms);
            }
            break;
        case G_END:
            // Write end to 7-segment
            if (inp_rc0_cnt != 0)
            {
                inp_rc0_reset();
                // Level 1 init
                game_state = G_LEVEL1;
                game_level = 1;
                game_notes_gnr = 5;
                game_notes_rmn = 5;
                tmr_start(tmr_500ms);
            }
            break;
        case G_FAIL:
            // write lose to 7-segment
            if (inp_rc0_cnt != 0)
            {
                inp_rc0_reset();
                // Level 1 init
                game_state = G_LEVEL1;
                game_level = 1;
                game_notes_gnr = 5;
                game_notes_rmn = 5;
                tmr_start(tmr_500ms);
            }
            break;
    }    
}

/*
 * 
 */
int main(int argc, char** argv) {

    init_ports();
    init_irq();
    tmr_init();
    tmr1_init();
    while (1)
    {
        timer_task();
        input_task();
        display_task();
        game_task();
    }
    
    return (EXIT_SUCCESS);
}

