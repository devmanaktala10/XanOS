.data
  fuck_you: .long 0x0
  fuck_you_2: .long 0x0

# syscall_handler
# DESCRIPTION: INT x80 handler links to the appropriate system call
#              and jumps via a jump table to the C function in system_call.c
# INPUTS: passed in via registers EAX - syscall number
#                                 EBX - first arg
#                                 ECX - second arg
#                                 EDX - third arg
# OUTPUTS: None
# RETURN VALUE: in EAX - -1 for failure or syscall return value
# SIDE EFFECTS: None
.globl syscall_handler
syscall_handler:
    pushal
    # pushl %esi
    # pushl %edi
    # pushl %edx
    subl $1, %eax             # subtract one from system call number
    cmpl $20, %eax            # check for bad value
    jae bad_arg
    pushl %edx
    pushl %ecx      
    pushl %ebx
    call *jump_table(,%eax,4) # jump to C function using jump table
    popl %ebx
    popl %ecx
    popl %edx
    movl %eax, fuck_you_2
    popal
    movl fuck_you_2, %eax
    # popl %edx
    # popl %edi
    # popl %esi
    iret                      # iret back
bad_arg:
    movl $-1, %eax            # return value = -1
    popl %edx                   
    popl %edi
    popl %esi
    iret

.globl guireturn_helper
guireturn_helper:
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

    movl   %eax, fuck_you

    # remove arguments not required from stack
    popl   %eax
    popl   %eax
    popl   %eax
    popl   %eax
    popl   %eax

    movl   fuck_you,  %eax

    # go back to user code
    iret


jump_table:
.long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn, chdir, mkdir, rmdir, create, unlink, guireturn, ioctl

