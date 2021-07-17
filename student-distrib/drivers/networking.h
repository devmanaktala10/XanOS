#ifndef _NETWORKING_H
#define _NETWORKING_H

#include "../lib.h"
#include "../interrupts/interrupt.h"

#define PCI_CONFIG_ADD    0xCF8
#define PCI_CONFIG_DATA   0xCFC

// all IP ARP constants in big endian format
#define ETH_IP_TYPE     0x0008
#define ETH_ARP_TYPE    0x0608
#define ETH_HW_TYPE     0x100
#define ARP_REQUEST     0x100
#define ARP_REPLY       0x200
#define ARP_HW_SIZE     0x6
#define ARP_PRO_SIZE    0x4
#define SIZE_ARP_IP     42

#define VGA_BUS       0x0
#define ETH_BUS       0x0
#define VGA_DEV_NO    0x2
#define ETH_DEV_NO    0x3
#define ETH_IRQ       0xB
#define BAR0_OFF      0x10

#define DEV_CTRL        0x0

#define RX_RAL          0x5400
#define RX_RAH          0x5404
#define MTA             0x5200
#define RX_DESC_LOW     0x2800
#define RX_DESC_HI      0x2804
#define RX_DESC_LEN     0x2808
#define RX_DESC_HEAD    0x2810
#define RX_DESC_TAIL    0x2818
#define RX_CTRL         0x0100
#define RX_RDTR         0x2820
#define INT_EN          0x00D0
#define INT_DIS         0x00D8
#define INT_CAUSE_READ  0x00C0
#define INT_CAUSE_SET   0x00C8

#define TX_DESC_LOW     0x3800
#define TX_DESC_HI      0x3804
#define TX_DESC_LEN     0x3808
#define TX_DESC_HEAD    0x3810
#define TX_DESC_TAIL    0x3818
#define TX_CTRL         0x0400
#define TX_IPG          0x0410

#define INT_CAUSE       0x00C0

#define DESC_LENGTH     64
#define PACKET_SIZE     2048

#define EEPROM_MEM      0x0014

#define IPV4_VERSION    4
#define UDP_PRO_NUM     17
#define TCP_PRO_NUM     6

#define UDP_LEN         8
#define IPV4_LEN        20
#define ETH_LEN         14
#define ARP_LEN         28

#define MAX_PACKET_SIZE     1500
#define MAX_PACKET_STORAGE  5
#define UDP_MAX_DATA_LEN    65536
#define NUM_UDP_PORTS       3
#define MAX_UDP_FILES       5
#define DEF_UDP_SPORT       2000

// Pre-processor macros to convert big endian to little endian
#define swap_32(x) ( (x >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | (x << 24) )
#define swap_16(x) ( (x >> 8) | (x << 8) )

typedef struct __attribute__((packed)) pci_header {
  uint16_t vendor_id;
  uint16_t dev_id;
  uint16_t cmd;
  uint16_t status;
  uint8_t rev_id;
  uint8_t prog_if;
  uint8_t subclass;
  uint8_t class_code;
  uint8_t cache_size;
  uint8_t latency_timer;
  uint8_t header_type;
  uint8_t bist;

  uint32_t bar0;
  uint32_t bar1;
  uint32_t bar2;
  uint32_t bar3;
  uint32_t bar4;
  uint32_t bar5;

  uint32_t cbus_pointer;
  uint16_t subsys_vid;
  uint16_t subsys_id;
  uint32_t exp_rom_addr;
  uint32_t res_1;
  uint32_t res_2;

  uint8_t int_line;
  uint8_t int_pin;
  uint8_t min_grant;
  uint8_t max_latency;
} pci_header_t;

typedef struct __attribute__((packed)) rx_desc {
  uint32_t address_low;
  uint32_t address_hi;
  uint16_t length;
  uint16_t checksum;
  uint8_t status;
  uint8_t errors;
  uint16_t special;
} rx_desc_t;

uint8_t* rx_buf;

typedef struct __attribute__((packed)) tx_desc {
  uint32_t address_low;
  uint32_t address_hi;
  uint16_t length;
  uint8_t cso;
  uint8_t cmd;
  uint8_t status;
  uint8_t css;
  uint16_t special;
} tx_desc_t;

typedef struct __attribute__((packed)) eth_header {
  uint8_t dest_mac[6];
  uint8_t sender_mac[6];
  uint16_t eth_type;
} eth_header_t;

typedef struct __attribute__((packed)) arp_header {
  uint16_t hwtype;
  uint16_t protype;
  uint8_t hwlen;
  uint8_t prolen;
  uint16_t opcode;
  uint8_t sender_mac[6];
  uint8_t sender_ip[4];
  uint8_t dest_mac[6];
  uint8_t dest_ip[4];
} arp_header_t;

typedef struct __attribute__((packed)) ip_header {
  uint8_t version : 4;
  uint8_t ihl : 4;
  uint8_t tos;
  uint16_t length;
  uint16_t id;
  uint16_t flags : 3;
  uint16_t f_off : 13;
  uint8_t ttl;
  uint8_t protype;
  uint16_t checksum;
  uint8_t sender_addr[4];
  uint8_t dest_addr[4];
} ip_header_t;

typedef struct __attribute__((packed)) udp_header {
  uint16_t source_port;
  uint16_t dest_port;
  uint16_t length;
  uint16_t checksum;
} udp_header_t;

typedef struct __attribute__((packed)) packet_buf {
  uint32_t base_addr;
  uint16_t id;
  uint16_t length;
  uint16_t avail;
  uint16_t packet_length;
  uint16_t source_port;
  uint16_t dest_port;
} packet_buf_t;

typedef struct __attribute__((packed)) net_port_desc {
  uint16_t port;
  uint16_t source_port;
  char files[MAX_UDP_FILES][8];
  uint8_t cur_file;
} net_file_desc_t;

// Variables required
rx_desc_t rx_desc_arr[DESC_LENGTH] __attribute__ ((aligned (16)));
tx_desc_t tx_desc_arr[DESC_LENGTH] __attribute__ ((aligned (16)));
uint32_t mmio_addr;
uint8_t* receive_buf;
uint8_t* packet_stack;
uint8_t mac[6];
uint8_t comp_mac[6];
uint8_t comp_ip[4];
uint8_t sys_ip[4];
packet_buf_t packet_buffers[MAX_PACKET_STORAGE];

struct dentry_ext net_dir;
struct dentry_ext udp_dir;
uint16_t udp_id;
uint16_t file_num;
char open_filename[32];

struct net_port_desc net_ports[NUM_UDP_PORTS];

uint32_t cur_rx_desc;
uint32_t cur_tx_desc;

pci_header_t eth_pci_header;
void networking_init();
uint32_t get_pci_data(uint32_t bus, uint32_t device, uint32_t func, uint32_t offset);
void set_pci_data(uint32_t data, uint32_t bus, uint32_t device, uint32_t func, uint32_t offset);
void poll_bus();
void get_mac_addr();
void initialize_header();
void networking_interrupt();
void networking_helper();
int send_packet(uint8_t* data, int32_t length);
void get_packet();
void hello_network();
void setup_ethernet();
void setup_filesystem();
void process_packet(uint32_t total_length);
void make_arp_reply(uint8_t* buf, arp_header_t* sender_arp_head);
uint16_t checksum_calc(uint16_t* start, uint32_t length);
void change_endian_ipv4(ip_header_t* ip_head);
void change_endian_udp(udp_header_t* udp_head);
void process_udp(ip_header_t* ip_head, udp_header_t* udp_head);
void make_udp_header(udp_header_t* udp_head, uint16_t sport, uint16_t dport);
void make_ip_header(ip_header_t* ip_head, uint16_t id, uint16_t f_off);
void send_data(uint8_t* data, uint32_t length, uint16_t sport, uint16_t dport);
uint16_t udp_checksum_calc(uint16_t data_length, uint16_t* data);
int32_t parse_port(char* filename, char* udp_port);
int32_t char_port_to_num(char* udp_port);

int32_t net_open();
int32_t net_read(void* buf, int32_t nbytes);
int32_t net_write(const void* buf, int32_t nbytes);
int32_t net_close();
int32_t net_ioctl(unsigned long cmd, unsigned long arg);

#endif
