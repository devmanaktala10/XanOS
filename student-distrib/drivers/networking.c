#include "networking.h"

void initialize_header() {
  uint8_t* eth_h_p = (uint8_t*) (&eth_pci_header);
  uint32_t data;
  unsigned i;
  for (i = 0; i <= 0x3C; i = i + 4) {
    data = get_pci_data(ETH_BUS, ETH_DEV_NO, 0, i);
    memcpy(eth_h_p + i, &data, 4);
  }
}

uint32_t get_pci_data(uint32_t bus, uint32_t device, uint32_t func, uint32_t offset) {
  uint32_t addr = (uint32_t) ( (bus << 16) | (device << 11) |
              (func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000) );
  outl(addr, PCI_CONFIG_ADD);
  return inl(PCI_CONFIG_DATA);

}

void set_pci_data(uint32_t data, uint32_t bus,
                uint32_t device, uint32_t func, uint32_t offset) {

  uint32_t addr = (uint32_t) ( (bus << 16) | (device << 11) |
              (func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000) );
  outl(addr, PCI_CONFIG_ADD);
  outl(data, PCI_CONFIG_DATA);
}

void poll_bus() {
  uint32_t address = get_pci_data(ETH_BUS, ETH_DEV_NO, 0, 4);
  outl(address, PCI_CONFIG_ADD);

  uint32_t data = inl(PCI_CONFIG_DATA);

  uint32_t first_4 = data & 0x0000FFFF;
  uint32_t second_4 = (data & 0xFFFF0000) >> 16;

  // graphic_printf("ETHERNET DATA - First 2 Bytes: %u  Sec 2 Bytes: %u", first_4, second_4);
}

void get_mac_addr() {
  // check eeprom
  // MAC address stored in EEPROM

  uint32_t data;
  unsigned i;

  int check_eeprom = 0;
  *((uint32_t*)(mmio_addr + EEPROM_MEM)) = 0x0;
  *((uint32_t*)(mmio_addr + EEPROM_MEM)) = 0x1;
  for (i = 0; i < 1000; i++) {
    data = *((uint32_t*)(mmio_addr + EEPROM_MEM));
    if (data & 0x10) {
      check_eeprom = 1;
      break;
    }
  }

  if (check_eeprom) {
    *((uint32_t*)(mmio_addr + EEPROM_MEM)) = 0x0;
    *((uint32_t*)(mmio_addr + EEPROM_MEM)) = 0x1;
    while (1) {
      data = *((uint32_t*)(mmio_addr + EEPROM_MEM));
      if (data & 0x10) {
        break;
      }
    }
    mac[0] = (uint8_t) ((data >> 16) & 0x00FF);
    mac[1] = (uint8_t) ((data >> 24) & 0x00FF);

    *((uint32_t*)(mmio_addr + EEPROM_MEM)) = 0x0;
    *((uint32_t*)(mmio_addr + EEPROM_MEM)) = (0x1 | 0x1 << 8);
    while (1) {
      data = *((uint32_t*)(mmio_addr + EEPROM_MEM));
      if (data & 0x10) {
        break;
      }
    }
    mac[2] = (uint8_t) ((data >> 16) & 0x00FF);
    mac[3] = (uint8_t) ((data >> 24) & 0x00FF);

    *((uint32_t*)(mmio_addr + EEPROM_MEM)) = 0x0;
    *((uint32_t*)(mmio_addr + EEPROM_MEM)) = (0x1 | 0x2 << 8);
    while (1) {
      data = *((uint32_t*)(mmio_addr + EEPROM_MEM));
      if (data & 0x10) {
        break;
      }
    }
    mac[4] = (uint8_t) ((data >> 16) & 0x00FF);
    mac[5] = (uint8_t) ((data >> 24) & 0x00FF);

  } else {
    uint8_t* mac_addr = (uint8_t*) (mmio_addr + 0x5400);
    for (i = 0; i < 6; i++) {
      mac[i] = mac_addr[i];
    }
  }
}

void networking_init() {
  setup_filesystem();
  setup_ethernet();
}

void setup_ethernet() {
  set_pci_data(NETWORK_PAGE_ADDR, ETH_BUS, ETH_DEV_NO, 0, 0x10);
  set_pci_data(0x0, ETH_BUS, ETH_DEV_NO, 0, 0x14);
  uint32_t data;
  data = get_pci_data(ETH_BUS, ETH_DEV_NO, 0, 0x4);
  data = data | 0x7;
  set_pci_data(data, ETH_BUS, ETH_DEV_NO, 0, 0x4);

  initialize_header();

  // Our kernel IP Address
  sys_ip[0] = 10;
  sys_ip[1] = 0;
  sys_ip[2] = 2;
  sys_ip[3] = 15;

  // Computer IP address
  comp_ip[0] = 10;
  comp_ip[1] = 0;
  comp_ip[2] = 2;
  comp_ip[3] = 2;

  udp_id = 0;

  mmio_addr = eth_pci_header.bar0;
  receive_buf = (uint8_t*) (NETWORK_MEM_PAGE_ADDR);
  packet_stack = (uint8_t*) (NETWORK_MEM_PAGE_ADDR + (FOURMB / 4));
  rx_buf = (uint8_t*) (NETWORK_MEM_PAGE_ADDR + 2*(FOURMB/4));

  file_num = 0;

  unsigned i;
  for (i = 0; i < MAX_PACKET_STORAGE; i++) {
    packet_buffers[i].id = 0x0;
    packet_buffers[i].length = 0x0;
    packet_buffers[i].avail = 0x1;
    packet_buffers[i].base_addr = packet_stack + i * UDP_MAX_DATA_LEN;
    packet_buffers[i].packet_length = 0xFFFF;
  }

  for (i = 0; i < NUM_UDP_PORTS; i++) {
    net_ports[i].port = 0;
    net_ports[i].source_port = DEF_UDP_SPORT;
    net_ports[i].cur_file = 0;
    unsigned j;
    for (j = 0; j < MAX_UDP_FILES; j++) {
      net_ports[i].files[j][0] = '\0';
    }
  }

  // graphic_printf("Memory Base Addr: %u \n", mmio_addr);

  get_mac_addr();

  for (i = 0; i < 6; i++) {
    // graphic_printf("%u:", mac[i]);
  }
  // graphic_printf("\n");

  // Initialize RX Descriptors
  for (i = 0; i < DESC_LENGTH; i++) {
    rx_desc_arr[i].address_low = (uint32_t) (rx_buf + i * PACKET_SIZE);
    rx_desc_arr[i].address_hi = 0;
    rx_desc_arr[i].status = 0;
    rx_desc_arr[i].errors = 0;
    rx_desc_arr[i].length = 0;
    rx_desc_arr[i].checksum = 0;
  }

  data = *((uint32_t*) (mmio_addr + DEV_CTRL));
  // graphic_printf("DEV CTRL: %u\n", data);
  data = data | (1 << 0) | (1 << 5);
  *((uint32_t*) (mmio_addr + DEV_CTRL)) = data;


  // setup RAL, RAH, & MTA
  memcpy(&data, mac, 4);
  *((uint32_t*) (mmio_addr + RX_RAL)) = data;
  data = 0x0;
  memcpy(&data, mac + 4, 2);
  data = data | (1 << 31);
  *((uint32_t*) (mmio_addr + RX_RAH)) = data;
  // clear up the MTA
  for (i = 0; i < 128; i++) {
    *((uint32_t*) (mmio_addr + MTA + i * 4)) = 0x0;
  }

  // clear the control register
  *((uint32_t*) (mmio_addr + RX_CTRL)) = 0;

  // initialize recieve
  *((uint32_t*) (mmio_addr + RX_DESC_LOW)) = (uint32_t) rx_desc_arr;
  *((uint32_t*) (mmio_addr + RX_DESC_HI)) = 0;
  *((uint32_t*) (mmio_addr + RX_DESC_LEN)) = (DESC_LENGTH * 16);
  *((uint32_t*) (mmio_addr + RX_DESC_HEAD)) = 0;
  *((uint32_t*) (mmio_addr + RX_DESC_TAIL)) = DESC_LENGTH - 1;
  cur_rx_desc = 0;

  // set the control register
  // EN, LPE, BAM, SECRC
  // currently enables receive buffer and sets packet size to 4096 bytes
  // also strips packets of CRC bytes, idk what that is Though
  // POSSIBLY ENABLE BAD PACKET RECEPTION?
  data = 0x0;
  data = ( (1 << 1) | (1 << 5) | (1 << 15) | (0 << 16) | (1 << 26) );
  *((uint32_t*) (mmio_addr + RX_CTRL)) = data;

  data = *((uint32_t*) (mmio_addr + RX_CTRL));
  // graphic_printf("RX CTRL DATA: %u\n", data);

  // Initialize TX Descriptors
  for (i = 0; i < DESC_LENGTH; i++) {
    tx_desc_arr[i].address_low = 0;
    tx_desc_arr[i].address_hi = 0;
    tx_desc_arr[i].cmd = 0;
    tx_desc_arr[i].length = 0;
    tx_desc_arr[i].status = 1;
    tx_desc_arr[i].css = 0;
    tx_desc_arr[i].cso = 0;
    tx_desc_arr[i].special = 0;
  }

  *((uint32_t*) (mmio_addr + TX_DESC_LOW)) = (uint32_t) tx_desc_arr;
  *((uint32_t*) (mmio_addr + TX_DESC_HI)) = 0;
  *((uint32_t*) (mmio_addr + TX_DESC_LEN)) = (DESC_LENGTH * 16);
  *((uint32_t*) (mmio_addr + TX_DESC_HEAD)) = 0;
  *((uint32_t*) (mmio_addr + TX_DESC_TAIL)) = 0;
  cur_tx_desc = 0;

  // clear the control register
  *((uint32_t*) (mmio_addr + TX_CTRL)) = 0x0;
  data = 0x0;
  data = ( (1 << 1) | (1 << 3) | (0x40 << 12) | (1 << 24) );
  *((uint32_t*) (mmio_addr + TX_CTRL)) = data;

  *((uint32_t*) (mmio_addr + TX_IPG)) = ( (10) | (8 << 10) | (12 << 20) );

  // enable Interrupts
  // 5 * 1.024 usec timer
  *((uint32_t*) (mmio_addr + RX_RDTR)) = 0x5;

  // clear int cause register
  data = *((uint32_t*) (mmio_addr + INT_CAUSE_READ));

  // LSC, RXSEQ, RXDMT (not used), RX0, RXT
  *((uint32_t*) (mmio_addr + INT_EN)) =
          ( (1 << 2) | (1 << 3) | (1 << 4) | (1 << 6) | (1 << 7) );

  uint32_t status = *((uint32_t*)(mmio_addr + 0x8));
  // graphic_printf("Status: %u\n", status);

  hello_network();

}

void hello_network() {

  eth_header_t eth_head;
  arp_header_t arp_head;
  unsigned i;

  for (i = 0; i < 6; i++) {
    eth_head.sender_mac[i] = mac[i];
    eth_head.dest_mac[i] = 0xFF;
    arp_head.sender_mac[i] = mac[i];
    arp_head.dest_mac[i] = 0x0;
  }

  eth_head.eth_type = ETH_ARP_TYPE;
  arp_head.hwtype = ETH_HW_TYPE;
  arp_head.protype = ETH_IP_TYPE;
  arp_head.hwlen = ARP_HW_SIZE;
  arp_head.prolen = ARP_PRO_SIZE;
  arp_head.opcode = ARP_REQUEST;

  for (i = 0; i < 4; i++) {
    arp_head.sender_ip[i] = sys_ip[i];
    arp_head.dest_ip[i] = comp_ip[i];
  }

  uint32_t length = sizeof(eth_header_t) + sizeof(arp_header_t);

  uint8_t buf[length];

  memcpy(buf, &eth_head, sizeof(eth_header_t));
  memcpy(buf + sizeof(eth_header_t), &arp_head, sizeof(arp_header_t));

  send_packet(buf, length);

}

int send_packet(uint8_t* data, int32_t length) {

  uint32_t num_desc;
  uint32_t desc_bytes;
  uint32_t start_num;
  uint32_t end_num;

  start_num = cur_tx_desc;
  end_num = cur_tx_desc;

  unsigned i = 0;
  while (1) {

    while ((tx_desc_arr[cur_tx_desc].status & 0x1) == 0);

    if (length > PACKET_SIZE) {
      desc_bytes = PACKET_SIZE;
    } else {
      desc_bytes = length;
    }

    tx_desc_arr[cur_tx_desc].address_low = (uint32_t) (data + i * PACKET_SIZE);
    tx_desc_arr[cur_tx_desc].length = desc_bytes;
    tx_desc_arr[cur_tx_desc].status = 0;
    tx_desc_arr[cur_tx_desc].cmd = (1 << 3);

    end_num = cur_tx_desc;
    cur_tx_desc = (cur_tx_desc + 1);
    if (cur_tx_desc == DESC_LENGTH) {
      cur_tx_desc = 0;
    }

    if (length < PACKET_SIZE) {
      break;
    }
    length = length - PACKET_SIZE;
    i++;
  }

  tx_desc_arr[end_num].cmd = ( (1 << 0) | (1 << 3) );
  *((uint32_t*) (mmio_addr + TX_DESC_TAIL)) = end_num + 1;
  // wait till the package gets processed
  while ( (((volatile uint8_t) tx_desc_arr[end_num].status) & 0x1) == 0 );

  uint32_t temp;
  temp = *((uint32_t*) (mmio_addr + TX_DESC_HEAD));
  // graphic_printf("TX HEAD: %u\n", temp);
  temp = *((uint32_t*) (mmio_addr + TX_DESC_TAIL));
  // graphic_printf("TX TAIL: %u\n", temp);

  return 0;

}

void get_packet() {
  uint8_t* data;
  uint32_t total_length = 0;
  uint32_t length = 0;
  uint32_t temp, head, tail;
  uint32_t int_masks;

  // disable interrupts from NIC
  int_masks = *((uint32_t*) (mmio_addr + INT_DIS));
  *((uint32_t*) (mmio_addr + INT_DIS)) = 0;

  // acknowledge interrupt has been processed, and get the cause
  temp = *((uint32_t*) (mmio_addr + INT_CAUSE_READ));
  // graphic_printf("CAUSE OF INT: %u\n", temp);

  head = *((uint32_t*) (mmio_addr + RX_DESC_HEAD));
  // graphic_printf("HEAD: %u\n", temp);
  tail = *((uint32_t*) (mmio_addr + RX_DESC_TAIL));
  // graphic_printf("TAIL: %u\n", temp);

  while ((rx_desc_arr[cur_rx_desc].status & 0x1)) {
    data = (uint8_t*) rx_desc_arr[cur_rx_desc].address_low;
    length = (uint32_t) rx_desc_arr[cur_rx_desc].length;

    // Copy over data in the receive buffer
    memcpy(receive_buf + total_length, data, length);
    total_length += length;

    if (rx_desc_arr[cur_rx_desc].status & 0x2) {
      // graphic_printf("EOP\n");
      process_packet(total_length);
      total_length = 0;
    }

    // order matters in case an interrupt comes in
    rx_desc_arr[cur_rx_desc].status = 0;
    temp = cur_rx_desc;
    cur_rx_desc = (cur_rx_desc + 1) % DESC_LENGTH;

    *((uint32_t*) (mmio_addr + RX_DESC_TAIL)) = temp;

  }

  head = *((uint32_t*) (mmio_addr + RX_DESC_HEAD));
  // graphic_printf("HEAD: %u\n", temp);
  tail = *((uint32_t*) (mmio_addr + RX_DESC_TAIL));
  // graphic_printf("TAIL: %u\n", temp);

  // enable interrupts from NIC
  *((uint32_t*) (mmio_addr + INT_DIS)) = int_masks;

}

void setup_filesystem() {

  if (-1 == find_dentry_ext("networking", &net_dir, 3)) {
    unsigned i = 0;
    while (i < 100) {
      i++;
      if (-1 == make_directory("networking", 3)) { continue; }
      if (-1 == find_dentry_ext("networking", &net_dir, 3)) { continue; }
      break;
    }
  }

  if (-1 == find_dentry_ext("udp", &udp_dir, 3)) {
    unsigned i = 0;
    while (i < 100) {
      i++;
      if (-1 == make_directory("udp", 3)) { continue; }
      if (-1 == find_dentry_ext("udp", &udp_dir, 3)) { continue; }
      break;
    }
  }
}

void networking_interrupt() {
  unsigned long flags;
  cli_and_save(flags);
  disable_irq(11);
  send_eoi(11);

  // call get_packet
  get_packet();

  enable_irq(11);
  restore_flags(flags);
}

void process_packet(uint32_t total_length) {
  eth_header_t* eth_head = (eth_header_t*) (receive_buf);
  arp_header_t* arp_head = (arp_header_t*) (receive_buf + ETH_LEN);
  ip_header_t* ip_head = (ip_header_t*) (receive_buf + ETH_LEN);
  udp_header_t* udp_head = (udp_header_t*) (receive_buf + ETH_LEN + IPV4_LEN);

  unsigned i;
  uint8_t process = 1;

  // only process package if its a broadcast or sent to our mac address
  for (i = 0; i < 6; i++) {
    if (eth_head->dest_mac[i] != mac[i]) {
      process = 0;
      break;
    }
  }

  if ( (eth_head->dest_mac[0] == 0xFF) && (eth_head->dest_mac[1] == 0xFF)
        && (eth_head->dest_mac[2] == 0xFF) && (eth_head->dest_mac[3] == 0xFF)
        && (eth_head->dest_mac[4] == 0xFF) && (eth_head->dest_mac[5] == 0xFF) )
  {
    process = 1;
  }

  if (process == 0) {
    return;
  }

  if (eth_head->eth_type == ETH_ARP_TYPE) {

    if (arp_head->hwtype != ETH_HW_TYPE || arp_head->hwlen != ARP_HW_SIZE) {
      return;
    }

    if (arp_head->protype != ETH_IP_TYPE || arp_head->prolen != ARP_PRO_SIZE) {
      return;
    }

    if (arp_head->opcode == ARP_REQUEST) {
      uint8_t buf[sizeof(eth_header_t) + sizeof(arp_header_t)];
      make_arp_reply(buf, arp_head);
      send_packet(buf, sizeof(eth_header_t) + sizeof(arp_header_t));
    }

    if (arp_head->opcode == ARP_REPLY) {
      for (i = 0; i < 6; i++) {
        comp_mac[i] = arp_head->sender_mac[i];
      }
      // test_send_data();
    }

  } else if (eth_head->eth_type == ETH_IP_TYPE) {

    // verify ip header checksum
    uint16_t csum_verify;
    uint16_t csum_val = ip_head->checksum;
    ip_head->checksum = 0;
    csum_verify = checksum_calc((uint16_t*) ip_head, sizeof(ip_header_t));
    if (csum_verify != csum_val) {
      // graphic_printf("Packet Faulty... Exiting\n");
      return;
    }
    ip_head->checksum = csum_val;

    // convert ip and udp to little endian
    change_endian_ipv4(ip_head);
    change_endian_udp(udp_head);

    // we only speak IPV4
    if (ip_head->version != IPV4_VERSION) {
      return;
    }

    if (ip_head->protype == UDP_PRO_NUM) {
      process_udp(ip_head, udp_head);
    } else if (ip_head->protype == TCP_PRO_NUM) {
      // graphic_printf("TCP not yet implemented... Exiting\n");
    }

  }

}

void process_udp(ip_header_t* ip_head, udp_header_t* udp_head) {
  uint8_t* data = (uint8_t*) (receive_buf + ETH_LEN + IPV4_LEN);
  uint32_t data_length = ip_head->length - IPV4_LEN;
  uint32_t pack_loc = MAX_PACKET_STORAGE + 1;
  uint32_t offset = 0;
  dentry_ext_t dentry;
  char* name_length;

  unsigned i;
  if (ip_head->f_off == 0) {
    if (ip_head->flags == 0) {

      data_length -= UDP_LEN;
      data += UDP_LEN;
      // no more packages store on hard drive
      uint8_t rel_net_desc = NUM_UDP_PORTS;

      for (i = 0; i < NUM_UDP_PORTS; i++) {
        if (udp_head->dest_port == net_ports[i].port) {
          rel_net_desc = i;
          break;
        }
      }

      if (rel_net_desc == NUM_UDP_PORTS) {
        return;
      }

      char name[10];
      name_length = (char*) itoa((uint32_t) file_num, name, 10);
      file_num++;

      i = 0;
      while (i < 100) {
        i++;
        if (-1 != find_dentry_ext(name_length, &dentry, net_dir.inode)) {
          delete_file_ext(dentry.inode, net_dir.inode);
        }
        if (-1 == make_empty_file_ext(name_length, net_dir.inode)) { continue; }
        if (-1 == find_dentry_ext(name_length, &dentry, net_dir.inode)) { continue; }
        if (-1 == write_data_ext(dentry.inode, data, 0, data_length)) { continue; }
        uint16_t cur_file = net_ports[rel_net_desc].cur_file;
        strcpy(net_ports[rel_net_desc].files[cur_file], name_length);
        net_ports[rel_net_desc].cur_file = (cur_file + 1) % MAX_UDP_FILES;
        return;
      }
      return;
    }
  }

  for (i = 0; i < MAX_PACKET_STORAGE; i++) {
    if ((ip_head->id == packet_buffers[i].id) && !packet_buffers[i].avail) {
      pack_loc = i;
      break;
    }
  }

  if (pack_loc == MAX_PACKET_STORAGE + 1) {
    // first packet reception, setup packet_buf
    for (i = 0; i < MAX_PACKET_STORAGE; i++) {
      if (packet_buffers[i].avail) {
        pack_loc = i;
        break;
      }
    }

    packet_buffers[pack_loc].packet_length = 0xFFFF;
    packet_buffers[pack_loc].id = ip_head->id;
    packet_buffers[pack_loc].avail = 0;
    packet_buffers[pack_loc].length = 0;
  }

  if (ip_head->f_off == 0) {
    data = data + UDP_LEN;
    packet_buffers[pack_loc].packet_length = udp_head->length - UDP_LEN;
    data_length = ip_head->length - IPV4_LEN - UDP_LEN;
    packet_buffers[pack_loc].dest_port = udp_head->dest_port;
    packet_buffers[pack_loc].source_port = udp_head->source_port;
  }

  // compute the offset
  offset = (ip_head->f_off * 2);
  if (ip_head->f_off >= 740) {
    offset -= 8;
  }

  // copy over data to correct part in packet buffer
  memcpy((uint8_t*) packet_buffers[pack_loc].base_addr + offset,
          data, data_length);
  // add length of data to buffer
  packet_buffers[pack_loc].length += data_length;

  if (packet_buffers[pack_loc].packet_length == packet_buffers[pack_loc].length) {
    // packet complete store on hard drive, and clean up packet_buffer

    uint8_t rel_net_desc = NUM_UDP_PORTS;

    for (i = 0; i < NUM_UDP_PORTS; i++) {
      if (packet_buffers[pack_loc].dest_port == net_ports[i].port) {
        rel_net_desc = i;
        break;
      }
    }

    if (rel_net_desc == NUM_UDP_PORTS) {
      packet_buffers[pack_loc].avail = 1;
      return;
    }

    char name[10];
    name_length = (char*) itoa((uint32_t) file_num, name, 10);
    file_num++;

    if (find_dentry_ext(name_length, &dentry, net_dir.inode)) {
      delete_file_ext(dentry.inode, net_dir.inode);
    }

    i = 0;
    while (i < 100) {
      i++;
      if (-1 != find_dentry_ext(name_length, &dentry, net_dir.inode)) {
        delete_file_ext(dentry.inode, net_dir.inode);
      }
      if (-1 == make_empty_file_ext(name_length, net_dir.inode)) { continue; }
      if (-1 == find_dentry_ext(name_length, &dentry, net_dir.inode)) { continue; }
      if (-1 == write_data_ext(dentry.inode, (uint8_t*) packet_buffers[pack_loc].base_addr,
                      0, packet_buffers[pack_loc].length)) { continue; }
      packet_buffers[pack_loc].avail = 1;
      uint16_t cur_file = net_ports[rel_net_desc].cur_file;
      strcpy(net_ports[rel_net_desc].files[cur_file], name_length);
      net_ports[rel_net_desc].cur_file = (cur_file + 1) % MAX_UDP_FILES;
      return;
    }

  }

}

void make_arp_reply(uint8_t* buf, arp_header_t* sender_arp_head) {
  unsigned i;
  eth_header_t* eth_head = (eth_header_t*) buf;
  arp_header_t* arp_head = (arp_header_t*) (buf + sizeof(eth_header_t));

  eth_head->eth_type = ETH_ARP_TYPE;
  arp_head->hwtype = ETH_HW_TYPE;
  arp_head->hwlen = ARP_HW_SIZE;
  arp_head->prolen = ARP_PRO_SIZE;
  arp_head->protype = ETH_IP_TYPE;
  arp_head->opcode = ARP_REPLY;

  for (i = 0; i < 6; i++) {
    eth_head->dest_mac[i] = sender_arp_head->sender_mac[i];
    eth_head->sender_mac[i] = mac[i];
    arp_head->dest_mac[i] = sender_arp_head->sender_mac[i];
    arp_head->sender_mac[i] = mac[i];
  }

  for (i = 0; i < 4; i++) {
    arp_head->dest_ip[i] = sender_arp_head->sender_ip[i];
    arp_head->sender_ip[i] = sys_ip[i];
  }

}

uint16_t checksum_calc(uint16_t* start, uint32_t length) {
  uint32_t sum = 0;
  while (length > 1) {
    sum += *start++;
    length -= 2;
  }

  if (length > 0) {
    sum += *(uint8_t*) start;
  }

  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  return (uint16_t) ~sum;
}

uint16_t udp_checksum_calc(uint16_t data_length, uint16_t* data) {
  uint32_t sum = 0;
  uint16_t* start;

  uint32_t temp = 0x0a000202;
  start = (uint16_t*) (&temp);
  sum += *start++;
  sum += *start;

  temp = 0x0a00020f;
  start = (uint16_t*) (&temp);
  sum += *start++;
  sum += *start;

  uint16_t length = UDP_LEN + data_length;
  // length = swap_16(length);
  sum += length;
  sum += UDP_PRO_NUM;

  length = UDP_LEN + data_length;

  while (length > 1) {
    sum += *data++;
    length -= 2;
  }

  if (length > 0) {
    sum += *(uint8_t*) data;
  }

  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  return (uint16_t) ~sum;
}

void change_endian_ipv4(ip_header_t* ip_head) {
  uint8_t temp;
  temp = ip_head->version;
  ip_head->version = ip_head->ihl;
  ip_head->ihl = temp;

  ip_head->length = swap_16(ip_head->length);
  ip_head->id = swap_16(ip_head->id);

  temp = ip_head->flags;
  ip_head->flags = (ip_head->f_off & 0x7);
  ip_head->f_off = ((temp << 10) | (ip_head->f_off >> 3));

}

void change_endian_udp(udp_header_t* udp_head) {
  udp_head->dest_port = swap_16(udp_head->dest_port);
  udp_head->source_port = swap_16(udp_head->source_port);
  udp_head->length = swap_16(udp_head->length);
}

void send_data(uint8_t* data, uint32_t length, uint16_t sport, uint16_t dport) {
  unsigned i;
  uint32_t buf_off = 0;
  uint32_t buf_len = 0;
  uint16_t f_off = 0;
  uint16_t flags = 0;
  uint16_t id = udp_id;
  udp_id++;
  unsigned count = 0;

  while (length > 0) {

    if (count == 0) {
      if (length > 1472) {
        buf_len = 1472;
      } else {
        buf_len = length;
      }
    } else {
      if (length > 1480) {
        buf_len = 1480;
      } else {
        buf_len = length;
      }
    }

    uint8_t buf[MAX_PACKET_SIZE + ETH_LEN];

    eth_header_t* eth_head = (eth_header_t*) buf;
    ip_header_t* ip_head = (ip_header_t*) (buf + ETH_LEN);

    make_eth_header(eth_head);
    make_ip_header(ip_head, id, 0);

    if (count == 0) {
      udp_header_t* udp_head  = (udp_header_t*) (buf + ETH_LEN + IPV4_LEN);
      make_udp_header(udp_head, sport, dport);
      ip_head->length += UDP_LEN + buf_len;
      udp_head->length = length + UDP_LEN;
      uint32_t temp_length = buf_len;
      memcpy(buf + ETH_LEN + IPV4_LEN + UDP_LEN, data + buf_off, temp_length);
      change_endian_udp(udp_head);
    } else {
      ip_head->length += buf_len;
      uint32_t temp_length = buf_len;
      memcpy(buf + ETH_LEN + IPV4_LEN, data + buf_off, temp_length);
    }

    change_endian_ipv4(ip_head);

    length -= buf_len;
    if (length > 0) {
      ip_head->flags = 4;
    }

    ip_head->f_off = 740 * count;

    uint8_t temp = (ip_head->f_off >> 10);
    ip_head->f_off = ((ip_head->f_off << 3) | ip_head->flags);
    ip_head->flags = temp;

    ip_head->checksum = checksum_calc((uint16_t*) ip_head, IPV4_LEN);
    // udp_head->checksum = udp_checksum_calc(buf_len, (uint16_t*) (udp_head));

    if (count == 0) {
      send_packet(buf, ETH_LEN + IPV4_LEN + UDP_LEN + buf_len);
    } else {
      send_packet(buf, ETH_LEN + IPV4_LEN + buf_len);
    }
    buf_off += buf_len;
    count++;
  }

}

void make_eth_header(eth_header_t* eth_head) {
  eth_head->eth_type = ETH_IP_TYPE;

  unsigned i;
  for (i = 0; i < 6; i++) {
    eth_head->dest_mac[i] = comp_mac[i];
    eth_head->sender_mac[i] = mac[i];
  }

}

void make_ip_header(ip_header_t* ip_head, uint16_t id, uint16_t f_off) {
  ip_head->version = 4;
  ip_head->ihl = 5;
  ip_head->tos = 0;
  ip_head->length = IPV4_LEN;
  ip_head->id = (id);
  ip_head->flags = 0;
  ip_head->f_off = f_off;
  ip_head->ttl = 64;
  ip_head->protype = UDP_PRO_NUM;
  ip_head->checksum = 0;

  unsigned i;
  for (i = 0; i < 4; i++) {
    ip_head->dest_addr[i] = comp_ip[i];
    ip_head->sender_addr[i] = sys_ip[i];
  }

}

void make_udp_header(udp_header_t* udp_head, uint16_t sport, uint16_t dport) {
  udp_head->source_port = sport;
  udp_head->dest_port = dport;
  udp_head->length = UDP_LEN;
  udp_head->checksum = 0;
}

void test_send_data() {
  uint8_t *buf = (uint8_t*) (NETWORK_MEM_PAGE_ADDR + 3*(FOURMB/4));
  dentry_ext_t dentry;
  int32_t retval = find_dentry_ext("terminal.txt", &dentry, 7);
  // char buf[] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  // uint32_t length = strlen(buf);
  uint32_t length = read_length_ext(dentry.inode);
  read_data_ext(dentry.inode, 0, buf, length);
  send_data(buf, 10000, 22, 23);
}

int32_t net_open() {
  char udp_port[6];
  int32_t retval = parse_port(open_filename, udp_port);
  if (retval == -1) {
    return -1;
  }

  retval = char_port_to_num(udp_port);

  if (retval == -1 || retval == 0) {
    return -1;
  }

  uint16_t port = (uint16_t) retval;

  unsigned i;
  for (i = 0; i < NUM_UDP_PORTS; i++) {
    if (net_ports[i].port == port) {
      return -1;
    }
  }

  unsigned avail_entry = NUM_UDP_PORTS;
  for (i = 0; i < NUM_UDP_PORTS; i++) {
    if (net_ports[i].port == 0) {
      avail_entry = i;
      break;
    }
  }

  if (avail_entry == NUM_UDP_PORTS) {
    return -1;
  }

  net_ports[avail_entry].port = port;
  net_ports[avail_entry].source_port = port;
  process_pcb->file[rel_fd].fileposition = avail_entry;
  net_ports[avail_entry].cur_file = 0;

  unsigned j;
  for (j = 0; j < MAX_UDP_FILES; j++) {
    net_ports[avail_entry].files[j][0] = '\0';
  }

  return 0;

}

int32_t net_close() {

  unsigned index = process_pcb->file[rel_fd].fileposition;
  if (index >= NUM_UDP_PORTS) {
    return -1;
  }

  if (net_ports[index].port == 0) {
    return -1;
  }

  unsigned i;
  dentry_ext_t dentry;
  for (i = 0; i < MAX_UDP_FILES; i++) {
    if (-1 != find_dentry_ext(net_ports[index].files[i], &dentry, net_dir.inode)) {
      delete_file_ext(dentry.inode, net_dir.inode);
    }
    net_ports[index].files[i][0] = '\0';
  }

  net_ports[index].cur_file = 0;
  net_ports[index].port = 0;
  net_ports[index].source_port = DEF_UDP_SPORT;

  return 0;
}

int32_t net_read(void* buf, int32_t nbytes) {
  unsigned index = process_pcb->file[rel_fd].fileposition;
  if (index >= NUM_UDP_PORTS) {
    return -1;
  }

  if (net_ports[index].port == 0) {
    return -1;
  }

  unsigned file_to_read = net_ports[index].cur_file;
  if (file_to_read == 0) {
    file_to_read = MAX_UDP_FILES - 1;
  } else {
    file_to_read = net_ports[index].cur_file - 1;
  }

  // wait until a file comes in
  while (net_ports[index].files[file_to_read][0] == '\0') {
    if (net_ports[index].cur_file == 0) {
      file_to_read = MAX_UDP_FILES - 1;
    } else {
      file_to_read = net_ports[index].cur_file - 1;
    }
  }

  dentry_ext_t dentry;
  char* name = net_ports[index].files[file_to_read];
  if (-1 == find_dentry_ext(name, &dentry, net_dir.inode)) {
    return -1;
  }

  net_ports[index].cur_file = file_to_read;

  uint32_t length = read_length_ext(dentry.inode);
  uint32_t retval = 0;
  retval = read_data_ext(dentry.inode, 0, buf, length);
  net_ports[index].files[file_to_read][0] = '\0';

  return retval;
}

int32_t net_write(const void* buf, int32_t nbytes) {
  unsigned index = process_pcb->file[rel_fd].fileposition;
  if (index >= NUM_UDP_PORTS) {
    return -1;
  }

  if (net_ports[index].port == 0) {
    return -1;
  }

  uint16_t dest_port = net_ports[index].port;
  uint16_t source_port = net_ports[index].source_port;

  send_data(buf, nbytes, source_port, dest_port);
}

int32_t net_ioctl(unsigned long cmd, unsigned long arg) {

  unsigned index = process_pcb->file[rel_fd].fileposition;
  if (index >= NUM_UDP_PORTS) {
    return -1;
  }

  if (net_ports[index].port == 0) {
    return -1;
  }

  // 1 corresponds to switch sender port
  if (cmd == 1) {
    int32_t retval = char_port_to_num((char*) arg);
    if (retval == -1 || retval == 0) {
      return -1;
    }
    net_ports[index].source_port = (uint16_t) retval;
    return 0;
  }

  return -1;
}

int32_t parse_port(char* filename, char* udp_port) {
  unsigned i = strlen(filename) - 1;
  unsigned j = 0;
  uint32_t start = 0;
  uint32_t end = 0;
  while (i > 0) {
    if (filename[i] == '/') {
      j = i + 1;
      break;
    }
    i--;
  }
  i = 0;
  while (j < strlen(filename)) {
    udp_port[i++] = filename[j++];
  }
  udp_port[i] = '\0';
  return 0;
}

int32_t char_port_to_num(char* udp_port) {
  uint16_t port = 0;
  uint32_t multiplier = 1;
  int32_t i = strlen(udp_port) - 1;

  while (i >= 0) {
    if (udp_port[i] < 0x30 || udp_port[i] > 0x39) {
      return -1;
    }
    port += multiplier * ((uint32_t) (udp_port[i]) - 0x30);
    multiplier = multiplier * 10;
    i--;
  }

  return (int32_t) port;
}
