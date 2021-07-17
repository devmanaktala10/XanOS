# keyboard_itr_wrapper
# DESCRIPTION: helper function wrapper that calls the keyboard interrupt handler
#              code. saves and restores all registers.
# INPUTS: None
# OUTPTUS: None
# SIDE EFFECTS: None
.globl keyboard_helper
keyboard_helper:
    pushal
    cld
    call keyboard_interrupt_handler
    cmpl $1, %eax
    jne  ret_to_user
    popal
    jmp sigint_handler
    ret_to_user:
    popal
    iret


# rtc_helper
# DESCRIPTION: helper function wrapper that calls the rtc interrupt handler
#              code. saves and restores all registers.
# INPUTS: None
# OUTPTUS: None
# SIDE EFFECTS: None
.globl rtc_helper
rtc_helper:
    pushal
    cld
    call rtc_interrupt_handler
    popal
    iret


.globl pit_helper
pit_helper:
    pushal
    cld
    call pit_handler
    popal
    iret

.globl mouse_helper
mouse_helper:
    pushal
    cld
    call mouse_interrupt_handler
    popal
    iret

.globl networking_helper
networking_helper:
    pushal
    cld
    call networking_interrupt
    popal
    iret

.globl sound_helper
sound_helper:
    pushal;
    cld
    call sb16_interrupt_handler
    popal
    iret
