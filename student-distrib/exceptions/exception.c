#include "exception.h"
/* Exception Messages for each exception */
const char * exception_messages[NUM_EXCEPTIONS] = {"Divide by Zero", "Debug", "Non-Maskable Interrupt", "Breakpoint",
                                        "Overflow", "Bound Range", "Invalid Opcode", "Device Not Available",
                                        "Double Fault", "Coprocessor Segment", "Invalid TSS", "Segment Not Present",
                                        "Stack Fault", "General Protection", "Page Fault", "Reserved" , "Floating Point x87",
                                        "Alignment Check", "Machine Check", "Floating Point SIMD"};

/* Exception Handler Function Pointer array */
const void * exception_handlers[NUM_EXCEPTIONS] = {divide_by_zero_handler, debug_handler, NMI_handler, breakpoint_handler,
                                       overflow_handler, bound_range_handler, invalid_opcode_handler, device_NA_handler,
                                       double_fault_handler, coproccesor_segment_handler, invalid_TSS_handler,
                                       segment_NP_handler, stack_fault_handler, general_protection_handler,
                                       page_fault_handler, NULL, FP87_handler, alignment_handler, machine_check_handler, FPSIMD_handler};

/*
 * exception handler
 *   DESCRIPTION: The exception specific handler jumps to this function.
 *                Prints the exception message and halts execution.
 *   INPUTS: uint32_t offset - the IDT number of the exception
 *   OUTPUTS: Prints the exception message on the screen
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Disables interrupts and halts execution
 */
void exception_handler(uint32_t offset){
    halt(1);
    printf("              Exception: %s :(", exception_messages[offset]);
    while(1);
}

/*
 * load_exceptions
 *   DESCRIPTION: Load each exception handler into the IDT
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Modifies IDT
 */
void load_exceptions(){

    int i;

    for(i = 0; i < NUM_EXCEPTIONS; i++){
        if(i != RESERVED_EXCEPTION){
            idt_desc_t exception;
            exception.seg_selector = KERNEL_CS; /* Exception are handled in the Kernel */
            exception.reserved4 = 0;
            exception.reserved3 = 1;            /* One for exceptions */
            exception.reserved2 = 1;
            exception.reserved1 = 1;
            exception.size      = 1;
            exception.reserved0 = 0;
            exception.dpl       = 0;            /* Kernel Privelege */
            exception.present   = 1;
            SET_IDT_ENTRY(exception, exception_handlers[i]);    /* Load the Exception handler */
            idt[i] = exception;                                 /* Modify IDT entry */
        }
    }

}
