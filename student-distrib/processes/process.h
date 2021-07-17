#ifndef _PROCESS_H
#define _PROCESS_H

#include "../types.h"
#include "../drivers/terminal.h"
#include "../filesystem/filesystem.h"
#include "../paging/page.h"
#include "../signals/signals.h"
#include "../drivers/gui.h"
#include "../text.h"

// #define SIGNAL_NUM 5

#define MAX_FILE 8
#define MAX_SCHEDULE 10
#define MAX_PID 23

#define SIGNAL_NUM 5

// typedef struct ptregs {
//   uint32_t ebx;
//   uint32_t ecx;
//   uint32_t edx;
//   uint32_t esi;
//   uint32_t edi;
//   uint32_t ebp;
//   uint32_t eax;

//   uint32_t ds;
//   uint32_t es;
//   uint32_t fs;

//   uint32_t sig_num;
//   uint32_t err_code;
//   uint32_t ret_add;
//   uint32_t cs;
//   uint32_t eflags;
//   uint32_t esp;
//   uint32_t ss;
// } ptregs_t;

struct signals {
  void* sig_ptr[SIGNAL_NUM];
  uint8_t bitvector;
};

typedef struct signals signals_t;

struct function_table{
  int32_t (*open)();
  int32_t (*read)(void * buf, int32_t nbytes);
  int32_t (*write)(const void* buf, int32_t nbytes);
  int32_t (*close)();
  int32_t (*ioctl)(unsigned long cmd, unsigned long args);
};

typedef struct file {
    struct function_table* file_op;
    uint32_t inode;
    uint32_t fileposition;
    uint32_t flags;
} file_t;

typedef struct pcb {

    int pid;
    struct pcb * pp;
    uint32_t ra;
    uint32_t ebp;
    uint32_t esp;

    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t flags;

    struct signals sigs;
    uint32_t sig_hw_esp;
    uint32_t gui_hw_esp;

    file_t file[MAX_FILE];
    int term;
    uint32_t curdir;
    char isBaseTerm;
    char pname[48];
    struct gui_element * gui_term;
    struct gui_element * queue_head;
    struct gui_element * queue_tail;

} pcb_t;

typedef struct pswap {

    uint32_t kesp; // 40
    uint32_t kebp; // 44

} pswap_t;

char pid[3];

pcb_t* process_pcb;
int process_term;
int32_t rel_fd;
pcb_t * schedule[MAX_SCHEDULE];
pswap_t sched_swaps[MAX_SCHEDULE];
int num_active_terminals;

void setup_process(int pid, uint32_t inode_num);
void teardown_process(pcb_t * process_pcb);
void context_switch(pcb_t * next_pcb);
void reset_pids();
void set_pid(uint32_t index, int val);
int get_pid(uint32_t index);
int find_free_pid();
void free_pid(uint32_t index);


int32_t stdin_open();
int32_t stdin_close();
int32_t stdin_write();
int32_t stdin_ioctl(unsigned long cmd, unsigned long arg);

int32_t stdout_open();
int32_t stdout_close();
int32_t stdout_read();
int32_t stdout_ioctl(unsigned long cmd, unsigned long arg);

void sched_process();
void init_signals();
extern void sigreturn_helper(ptregs_t temp_ptregs);
extern void signal_handler_helper(int32_t signal_number, ptregs_t temp_ptregs);
volatile struct signals default_signals;
void empty_process();
void make_temp_ptregs(ptregs_t * temp_ptregs);

// pcb_t* get_pcb();

#endif
