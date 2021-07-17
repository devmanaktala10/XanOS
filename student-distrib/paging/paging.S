# constants
.data
    BIT4        = 0x010        # mask to get BIT 4
    BIT31       = 0x80000000   # mask to get BIT 31
    
# set_cr
# DESCRIPTION: Enables paging by changing the cr registers. First, the base
#              address of the page directory is stored in the cr3 register.
#              Bit 4 of cr4 is set to enable 4MB pages. Finally, bit 31 of
#              cr0 is set to enable paging. 
# INPUTS: void * base_address - base address of the page directory
# OUTPUTS: None
# RETURN VALUE: None
# SIDE EFFECTS: Modifies cr0 registers
.globl set_cr
set_cr:
    pushl %ebp
    movl %esp, %ebp
    pushl %eax
    movl 8(%ebp), %eax      # get base address argument
    movl %eax, %cr3         # set base address in cr3
    movl %cr4, %eax
    orl $BIT4, %eax         # set bit 4 of cr4
    movl %eax, %cr4
    movl %cr0, %eax
    orl $BIT31, %eax        # set bit 31 of cr0
    movl %eax, %cr0
    popl %eax
    leave
    ret
