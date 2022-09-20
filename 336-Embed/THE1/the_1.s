PROCESSOR 18F8722
    
#include <xc.inc>

; configurations
CONFIG OSC = HSPLL, FCMEN = OFF, IESO = OFF, PWRT = OFF, BOREN = OFF, WDT = OFF, MCLRE = ON, LPT1OSC = OFF, LVP = OFF, XINST = OFF, DEBUG = OFF

; global variable declarations
GLOBAL state, level, action, countdown, temp
GLOBAL portb_on, portc_on
GLOBAL ra4_down, re4_down
GLOBAL _t1, _t2, _t3, t1, t2, t3

; allocating memory for variables
PSECT udata_acs
    state:
	DS 1    ; allocates 1 byte
    level:
	DS 1
    action:
	DS 1
    countdown:
	DS 1
    temp:
	DS 1
    ra4_down:
	DS 1
    re4_down:
	DS 1
    portb_on:
	DS 1
    portc_on:
	DS 1
    _t1:
        DS 1    ; allocate 1 byte
    _t2:
        DS 1    ; allocate 1 byte
    _t3:
        DS 1    ; allocate 1 byte
    t1:
	DS 1
    t2:
	DS 1
    t3:
	DS 1

PSECT resetVec,class=CODE,reloc=2
resetVec:
    goto    main

init:
    clrf LATA	
    clrf LATB
    clrf LATC
    clrf LATD
    clrf LATE
    movlw 0x00    ; all pins output
    movwf TRISB
    movwf TRISC
    movwf TRISD
    movlw 00010000B    ; RA4 and RE4 input
    movwf TRISA
    movwf TRISE
    movlw 0
    movwf state	    ; state = 0
    movlw 0
    movwf countdown ; countdown = 0
    movlw 1
    movwf level	    ; level = 1
    movwf action    ; action = 1
    movwf portb_on
    movwf portc_on
    movlw 0
    movwf ra4_down
    movwf re4_down
    movlw 0x2
    movwf t1
    movlw 0xBA
    movwf t2
    movlw 0xE0
    movwf t3
    return
    
update_lights:
    btfsc state, 1
    goto states_2_3
    btfsc state, 0
    goto update_state_1
    goto update_state_0
    states_2_3:
	btfsc state, 0
	goto update_state_3
	goto update_state_2
    update_state_0:
	call light_default
	return
    update_state_1:
	call light_portb
	return
    update_state_2:
	call light_portc
	return
    update_state_3:
	call countdown_timer
	return
    
    
; DO NOT DELETE OR MODIFY
; 500ms pass check for test scripts
ms500_passed:
    nop
    return

; DO NOT DELETE OR MODIFY
; 1sec pass check for test scripts
ms1000_passed:
    nop
    return
    
busy_wait_1000ms:
    movlw 0x84      ; copy desired value to W
    movwf _t3       ; copy W into t3
    _loop3:
        movlw 0xAF      ; copy desired value to W
        movwf _t2       ; copy W into t2
        _loop2:
            movlw 0x8F      ; copy desired value to W
            movwf _t1       ; copy W into t1
            _loop1:
                decfsz _t1, 1   ; decrement t1, if 0 skip next 
                goto _loop1     ; else keep counting down
                decfsz _t2, 1   ; decrement t2, if 0 skip next 
                goto _loop2     ; else keep counting down
                decfsz _t3, 1   ; decrement t3, if 0 skip next 
                goto _loop3     ; else keep counting down
                return  

busy_wait_500ms:
    movlw 0x42      ; copy desired value to W
    movwf _t3       ; copy W into t3
    __loop3:
        movlw 0xAF      ; copy desired value to W
        movwf _t2       ; copy W into t2
        __loop2:
            movlw 0x8F      ; copy desired value to W
            movwf _t1       ; copy W into t1
            __loop1:
                decfsz _t1, 1   ; decrement t1, if 0 skip next 
                goto __loop1     ; else keep counting down
                decfsz _t2, 1   ; decrement t2, if 0 skip next 
                goto __loop2     ; else keep counting down
                decfsz _t3, 1   ; decrement t3, if 0 skip next 
                goto __loop3     ; else keep counting down
                return 
    
light_all:
    movlw 0xFF
    movwf LATD
    movlw 0xF
    movwf LATB
    movlw 0x7
    movwf LATC
    return
    
light_default:
    movlw 0x1
    movwf LATB
    movwf LATC
    movlw 0x0
    movwf LATD
    return
   
light_portb:
    btfsc level, 2
    goto level_4
    btfsc level, 1
    goto level_2_3
    goto level_1
    level_2_3:
	btfsc level, 0
	goto level_3
	goto level_2
    level_4:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	btfss portb_on, 0
	return
	movlw 00001111B
	movwf LATB
	return
    level_3:
	nop
	nop
	btfss portb_on, 0
	return
	movlw 00000111B
	movwf LATB
	return
    level_2:
	btfss portb_on, 0
	return
	movlw 00000011B
	movwf LATB
	return
    level_1:
	nop
	nop
	nop
	btfss portb_on, 0
	return
        movlw 00000001B
	movwf LATB
	return
   
light_portc:
    btfsc action, 1
    goto action_2
    goto action_1
    action_2:
	nop
	nop
	nop
	nop
	nop
	nop
	btfss portc_on, 0
	return
	movlw 00000010B
	movwf LATC
	return
    action_1:
	nop
	nop
	nop
	nop
	btfss portc_on, 0
	return
        movlw 00000001B
	movwf LATC
	return
	
calc_countdown:
    movf level, 0
    btfsc action, 1
    addwf countdown
    addwf countdown
    return
    
countdown_timer:
    call calc_countdown
    btfsc countdown, 3
    goto countdown_8
    btfsc countdown, 2
    goto sum_6_4
    btfsc countdown, 1
    goto sum_3_2
    goto countdown_1
    sum_6_4:
	btfsc countdown, 1
	goto countdown_6
	goto countdown_4
    sum_3_2:
	btfsc countdown, 0
	goto countdown_3
	goto countdown_2
    countdown_8:
	movlw 11111111B
	movwf LATD
	goto countdown_loop
    countdown_6:
	movlw 00111111B
	movwf LATD
	goto countdown_loop
    countdown_4:
	movlw 00001111B
	movwf LATD
	goto countdown_loop
    countdown_3:
	movlw 00000111B
	movwf LATD
	goto countdown_loop
    countdown_2:
	movlw 00000011B
	movwf LATD
	goto countdown_loop
    countdown_1:
	movlw 00000001B
	movwf LATD
	goto countdown_loop
    countdown_loop:
	clrf WREG
	cpfsgt countdown
	goto restart
	call busy_wait_500ms
	decf countdown
	; update portd lights
	movlw 4
	cpfsgt countdown
	goto lower_than_4
	goto greater_than_4
	lower_than_4:
	    movlw 2
	    cpfsgt countdown
	    goto lower_than_2
	    goto greater_than_2
	    lower_than_2:
		btfsc countdown, 0
		goto light_countdown_1
		btfsc countdown, 1
		goto light_countdown_2
		goto light_countdown_0
	    greater_than_2:
		btfsc countdown, 0
		goto light_countdown_3
		goto light_countdown_4
	greater_than_4:
	    movlw 6
	    cpfsgt countdown
	    goto lower_than_6
	    goto greater_than_6
	    lower_than_6:
		btfsc countdown, 0
		goto light_countdown_5
		goto light_countdown_6
	    greater_than_6:
		btfsc countdown, 0
		goto light_countdown_7
		goto light_countdown_8
	light_countdown_0:
	    movlw 00000000B
	    movwf LATD
	    goto countdown_loop
	light_countdown_1:
	    movlw 00000001B
	    movwf LATD
	    goto countdown_loop
	light_countdown_2:
	    movlw 00000011B
	    movwf LATD
	    goto countdown_loop
	light_countdown_3:
	    movlw 00000111B
	    movwf LATD
	    goto countdown_loop
	light_countdown_4:
	    movlw 00001111B
	    movwf LATD
	    goto countdown_loop
	light_countdown_5:
	    movlw 00011111B
	    movwf LATD
	    goto countdown_loop
        light_countdown_6:
	    movlw 00111111B
	    movwf LATD
	    goto countdown_loop
	light_countdown_7:
	    movlw 01111111B
	    movwf LATD
	    goto countdown_loop
	light_countdown_8:
	    movlw 11111111B
	    movwf LATD
	    goto countdown_loop
    restart:
	movlw 1
	movwf action
	movwf level
	movlw 0
	movwf state
	return
		
re4_check_down:
    btfsc PORTE, 4
    bsf re4_down, 0
    return
    
re4_check_up:
    btfsc re4_down, 0
    goto _down
    return
    _down:
	btfsc PORTE, 4
	return
	bcf re4_down, 0
	call update_status
	return
    
ra4_check_down:
    btfsc PORTA, 4
    bsf ra4_down, 0
    return
    
ra4_check_up:
    btfsc ra4_down, 0
    goto __down
    return
    __down:
	btfsc PORTA, 4
	return
	bcf ra4_down, 0
	call update_value
	return
	
update_status:
    incf state
    btfsc state, 2
    goto restart_state
    movlw 1
    movwf portb_on
    movwf portc_on
    call light_portb
    call light_portc
    return
    restart_state:
	movlw 0
	movwf state
	movlw 1
	movwf portb_on
	movwf portc_on
	return
    
update_value:   
    btfsc state, 1
    goto update_value_2_3
    btfsc state, 0
    goto update_level_value ; state 1
    return  ;state 0
    update_value_2_3:
	btfsc state, 0
	return	;state 3
	goto update_action_value ; state 2
    update_action_value:
	btfsc action, 0
	goto incr_action
	decf action
	return
	incr_action:
	    incf action
	    return
    update_level_value:
	btfsc level, 2
	goto restart_level_value
	incf level
	return
	restart_level_value:
	    movlw 1
	    movwf level
	    return
	
count_500ms:
    clrf WREG
    dcfsnz t3
    goto t3_loop
    return
    t3_loop:
	dcfsnz t2
	goto t2_loop
	movlw 0xD0
	movwf t3
	return
	t2_loop:
	    dcfsnz t1
	    goto blink
	    movlw 0xBA
	    movwf t2
	    movlw 0xD0
	    movwf t3
	    return
	    blink:
		movlw 0xD0
		movwf t3
		movlw 0xBA
		movwf t2
		movlw 0x2
		movwf t1
		btfsc state, 0
		goto blink_b
		btfsc state, 1
		goto blink_c
		goto blink_def
		blink_def:
		    call ms500_passed
		    return
		blink_b:
		    btfsc LATB, 0
		    goto blink_off_b
		    goto blink_on_b
		    blink_on_b:
			movlw 1
			movwf portb_on
			call ms500_passed
			return
		    blink_off_b:
			movlw 0
			movwf LATB
			movwf portb_on
			call ms500_passed
			return
		blink_c:
		    btfsc portc_on, 0
		    goto blink_off_c
		    goto blink_on_c
		    blink_on_c:
			movlw 1
			movwf portc_on
			call ms500_passed
			return
		    blink_off_c:
			movlw 0
			movwf LATC
			movwf portc_on
			call ms500_passed
			return
		
 
PSECT CODE
main:
    ; some code to initialize and wait 1000ms here, maybe
    call init
    call light_all
    call busy_wait_1000ms
    call ms1000_passed

    ; a loop here, maybe
    loop:
	call update_lights
	call re4_check_down
	call ra4_check_down
	call re4_check_up
	call ra4_check_up
	call count_500ms
        goto loop

end resetVec