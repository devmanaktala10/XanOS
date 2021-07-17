.data
  temp_eax: .long 0x0
  temp_ecx: .long 0x0
  signal_number: .long 0x0
  error_code: .long 0x0

.text

  .globl signal_handler
  .globl sigint_handler
  .globl signal_handler_helper
  .globl set_tss_args
  .globl tss_args
  .globl temp_ptregs
  .globl default_signals
  .globl process_pcb
  .globl sigreturn_helper

  sigint_handler:
    # add garbage error value and signal no. 2 to stack and call signal_handler
    pushl $0
    pushl $2
    jmp signal_handler

signal_handler:
    # save the values in eax, ecx
  movl    %eax, temp_eax
  movl    %ecx, temp_ecx

    # move signal no into variable
  movl    (%esp), %eax
  movl    %eax, signal_number

  pushal

  pushl   signal_number
  call    sig_default_action
    # remove argument from stack
  popl    %edx

    # ifreturn code is -1, we iret else we go on to push the hardware_context
  cmpl    $-1, %eax
  jne     push_h_context

    # remove the signum and error code
  popal
  popl    %eax
  popl    %eax
  movl    temp_eax, %eax
    # stack set up iret back to user code
  iret

    push_h_context:
    # Not default action, therefore start pushing hardware context
    call set_tss_args
    popal

    movl    tss_args, %ecx       # push fs
    pushl   %ecx
    movl    tss_args+4, %ecx     # push es
    pushl   %ecx
    movl    tss_args+8, %ecx     # push ds
    pushl   %ecx

    movl  temp_eax, %eax
    movl  temp_ecx, %ecx

    # pushall registers to stack in correct order
    pushl %eax
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx

    pushl signal_number
    call signal_handler_helper

  sigreturn_helper:
    # remove return address for sigreturn
    popl   %eax

    # Reinitialize the register values
    popl   %ebx
    popl   %ecx
    popl   %edx
    popl   %esi
    popl   %edi
    popl   %ebp
    popl   %eax

    movl   %eax, temp_eax

    # remove arguments not required from stack
    popl   %eax
    popl   %eax
    popl   %eax
    popl   %eax
    popl   %eax

    movl   temp_eax,  %eax

    # go back to user code
    iret
    