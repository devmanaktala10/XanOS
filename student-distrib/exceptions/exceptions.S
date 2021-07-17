# divide_by_zero_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl divide_by_zero_handler
divide_by_zero_handler:
    pushl $0            # garbage error value
    pushl $0            # divide_by_zero exception IDT number
    jmp signal_handler

# debug_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl debug_handler
debug_handler:
    pushl $1            # debug exception IDT number
    pushl %ebp
    movl %esp, %ebp
    jmp exception_handler

# NMI_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl NMI_handler
NMI_handler:
    pushl $2            # NMI exception IDT number
    pushl %ebp
    movl %esp, %ebp
    jmp exception_handler

# breakpoint_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl breakpoint_handler
breakpoint_handler:
    pushl $3            # breakpoint exception IDT number
    pushl %ebp
    movl %esp, %ebp
    jmp exception_handler

# overflow_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl overflow_handler
overflow_handler:
    pushl $4            # overflow exception IDT number
    pushl %ebp
    movl %esp, %ebp
    jmp exception_handler

# bound_range_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl bound_range_handler
bound_range_handler:
  pushl $0            # garbage error value
  pushl $1            # divide_by_zero exception IDT number
  jmp signal_handler

# invalid_opcode_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl invalid_opcode_handler
invalid_opcode_handler:
  pushl $0            # garbage error value
  pushl $1            # divide_by_zero exception IDT number
  jmp signal_handler

# device_NA_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl device_NA_handler
device_NA_handler:
  pushl $0            # garbage error value
  pushl $1            # divide_by_zero exception IDT number
  jmp signal_handler

# double_fault_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl double_fault_handler
double_fault_handler:
  pushl $1            # divide_by_zero exception IDT number
  jmp signal_handler

# coproccesor_segment_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl coproccesor_segment_handler
coproccesor_segment_handler:
  pushl $0            # garbage error value
  pushl $1            # divide_by_zero exception IDT number
  jmp signal_handler

# invalid_TSS_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl invalid_TSS_handler
invalid_TSS_handler:
  pushl $1            # divide_by_zero exception IDT number
  jmp signal_handler

# segment_NP_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl segment_NP_handler
segment_NP_handler:
  pushl $1            # divide_by_zero exception IDT number
  jmp signal_handler

# stack_fault_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl stack_fault_handler
stack_fault_handler:
  pushl $1            # divide_by_zero exception IDT number
  jmp signal_handler

# general_protection_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl general_protection_handler
general_protection_handler:
  pushl $1            # divide_by_zero exception IDT number
  jmp signal_handler

# page_fault_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl page_fault_handler
page_fault_handler:
  pushl $1            # divide_by_zero exception IDT number
  jmp signal_handler

# FP87_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl FP87_handler
FP87_handler:
  pushl $0            # garbage error value
  pushl $1            # divide_by_zero exception IDT number
  jmp signal_handler

# alignment_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl alignment_handler
alignment_handler:
  pushl $1            # divide_by_zero exception IDT number
  jmp signal_handler

# machine_check_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl machine_check_handler
machine_check_handler:
    pushl $18           # machine_check exception IDT number
    pushl %ebp
    movl %esp, %ebp
    jmp exception_handler

# FPSIMD_handler
# DESCRIPTION: pushes the appropriate exception number on the stack
#              and jumps to the exception handler function in kernel.c
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Modifies stack
.globl FPSIMD_handler
FPSIMD_handler:
  pushl $0            # garbage error value
  pushl $1            # divide_by_zero exception IDT number
  jmp signal_handler
