#include <common.h>
#include <net.h>

#include <asm/processor.h>
#include <asm/ppc4xx.h>

#include "../include/bit.h"
#include "../include/fpga.h"

static u8 local_mac[6] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab};
static u8 remote_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

#define IP(x3,x2,x1,x0) ((((x3) & 0xff) << 24) | \
                         (((x2) & 0xff) << 16) | \
                         (((x1) & 0xff) << 8) | \
                         (((x0) & 0xff) << 0))

static IPaddr_t default_local_ip = IP(192,168,41,5);
static IPaddr_t default_server_ip = IP(192,168,41,1);

static u16 pingseq = 0;

IPaddr_t v6gbe_local_ip(void)
{
  /* v6gbe ip env var is defined then */

  /* else */
  return default_local_ip;
}

IPaddr_t v6gbe_server_ip(void)
{
  /* v6gbe ip env var is defined then */

  /* else */
  return default_server_ip;
}

void v6gbe_reg_set_short(u32 reg, u16 data)
{
  u32 offset = CONFIG_SYS_FPGA_BASE + BSP_GBE_OFFSET;
  *((u16*)(offset + BSP_GBE_REG_OFFSET + reg)) = data;
}

u16 v6gbe_reg_get_short(u32 reg)
{
  u32 offset = CONFIG_SYS_FPGA_BASE + BSP_GBE_OFFSET;
  return *((u16*)(offset + BSP_GBE_REG_OFFSET + reg));
}

int v6gbe_sendpacket(u8* data, int size)
{
  u32 offset = CONFIG_SYS_FPGA_BASE + BSP_GBE_OFFSET;
  memcpy((u8*)(offset + BSP_GBE_TX_OFFSET), data, size);
  v6gbe_reg_set_short(BSP_GBE_REG_TXSIZE, size);
  return 0;
}

int v6gbe_getframe(Ethernet_t *eth_h, u8 *data)
{
  u16 rxsize;
  u32 offset = CONFIG_SYS_FPGA_BASE + BSP_GBE_OFFSET;

  rxsize = v6gbe_reg_get_short(BSP_GBE_REG_RXSIZE);
  if (rxsize == 0)
    return 0;


  if (rxsize <= ETHER_HDR_SIZE) {
    v6gbe_reg_set_short(BSP_GBE_REG_RXSIZE, 0);
    return -1;
  }

  if (rxsize > 1500 + ETHER_HDR_SIZE) {
    v6gbe_reg_set_short(BSP_GBE_REG_RXSIZE, 0);
    return -2;
  } else {
    rxsize = rxsize - ETHER_HDR_SIZE;  /* rxsize is now the payload size */
  }

  memcpy((u8*)eth_h, (u8*)(offset + BSP_GBE_RX_OFFSET), ETHER_HDR_SIZE);

  memcpy(data, (u8*)(offset + BSP_GBE_RX_OFFSET + ETHER_HDR_SIZE), rxsize);

  /* Ack the buffer */
  v6gbe_reg_set_short(BSP_GBE_REG_RXSIZE, 0);

  return rxsize;
}

int v6gbe_flushrx(void)
{
  /* Ack the buffer */
  v6gbe_reg_set_short(BSP_GBE_REG_RXSIZE, 0);
  /* Ack the buffer */
  v6gbe_reg_set_short(BSP_GBE_REG_RXSIZE, 0);

  return 0;
}

#define PACKET_BUFFER_MAX 1500

int v6gbe_sendping(void)
{
  u8 packet_buffer[PACKET_BUFFER_MAX];
  u8* packet_index = packet_buffer;
  Ethernet_t eth_h;
  IP_t ip_h;
  u16 icmp_h[4];
  int ipsize;

  memcpy(eth_h.et_dest, remote_mac, 6);
  memcpy(eth_h.et_src, local_mac, 6);
  eth_h.et_protlen = PROT_IP;

  ipsize = IP_HDR_SIZE_NO_UDP + 8;

  ip_h.ip_hl_v = 0x45; /* IPV4 */
  ip_h.ip_tos = 0x0;
  ip_h.ip_len = ipsize; /* fixed size, dont care really */
  ip_h.ip_id = 0xa865;
  ip_h.ip_off = IP_FLAGS_DFRAG;
  ip_h.ip_ttl = 0xff;
  ip_h.ip_p = IPPROTO_ICMP;
  ip_h.ip_sum = 0x0;
  ip_h.ip_src = v6gbe_local_ip();
  ip_h.ip_dst = v6gbe_server_ip();

  ip_h.ip_sum = ~NetCksum((u8*)(&ip_h), IP_HDR_SIZE_NO_UDP/2);

  icmp_h[0] = htons(ICMP_ECHO_REQUEST << 8);
  icmp_h[1] = 0x0;
  icmp_h[2] = 0xbeef;
  icmp_h[3] = htons(pingseq++);

  icmp_h[1] = ~NetCksum((u8*)(icmp_h), 4);

  memcpy(packet_index, (u8 *)(&eth_h), ETHER_HDR_SIZE);
  packet_index += ETHER_HDR_SIZE;
  memcpy(packet_index, (u8 *)(&ip_h), IP_HDR_SIZE_NO_UDP);
  packet_index += IP_HDR_SIZE_NO_UDP;
  memcpy(packet_index, (u8 *)(icmp_h), 4*2);
  return v6gbe_sendpacket(packet_buffer, ipsize + ETHER_HDR_SIZE);
}

int v6gbe_send_arpresponse(u8 *dest_mac, IPaddr_t dest_ip)
{
  u8 packet_buffer[PACKET_BUFFER_MAX];
  u8* packet_index = packet_buffer;
  Ethernet_t eth_h;
  ARP_t arp_h;
  IPaddr_t local_ip = v6gbe_local_ip();
  u8 arp_data[20] = {local_mac[0], local_mac[1], local_mac[2],
              local_mac[3], local_mac[4], local_mac[5],
              (local_ip >> 24) & 0xff,
              (local_ip >> 16) & 0xff,
              (local_ip >> 8)  & 0xff,
              (local_ip >> 0)  & 0xff,
              dest_mac[0], dest_mac[1], dest_mac[2],
              dest_mac[3], dest_mac[4], dest_mac[5],
              (dest_ip >> 24) & 0xff,
              (dest_ip >> 16) & 0xff,
              (dest_ip >> 8)  & 0xff,
              (dest_ip >> 0)  & 0xff};

  memcpy(eth_h.et_dest, dest_mac, 6);
  memcpy(eth_h.et_src, local_mac, 6);
  eth_h.et_protlen = PROT_ARP;

  arp_h.ar_hrd = ARP_ETHER;
  arp_h.ar_pro = 0x0800;
  arp_h.ar_hln = 0x6;
  arp_h.ar_pln = 0x4;
  arp_h.ar_op = ARPOP_REPLY;

  memcpy(packet_index, (u8 *)(&eth_h), ETHER_HDR_SIZE);
  packet_index += ETHER_HDR_SIZE;
  memcpy(packet_index, (u8 *)(&arp_h), 8);
  packet_index += 8;
  memcpy(packet_index, arp_data, 20);

  return v6gbe_sendpacket(packet_buffer, ETHER_HDR_SIZE + 28);
}

int v6gbe_rxloop(void)
{
  Ethernet_t eth_h;
  u8 packet_buffer[PACKET_BUFFER_MAX];
  ARP_t *arp_h;
  IP_t *ip_h;
  int size;

  size = v6gbe_getframe(&eth_h, packet_buffer);
  if (size < 0)
    return -1;
  if (size == 0)
    return 0; /* no action */

  if (eth_h.et_protlen == PROT_ARP) {
    IPaddr_t src_ip;

    arp_h = (ARP_t*) ((u8*)(packet_buffer));
    src_ip = IP(packet_buffer[14], packet_buffer[15], packet_buffer[16], packet_buffer[17]);

    if (arp_h->ar_op == ARPOP_REQUEST) {
      //printf("sent arp\n");
      if (v6gbe_send_arpresponse(packet_buffer + 8, src_ip))
        return -1;
    }
  }

  if (eth_h.et_protlen == PROT_IP) {
    ip_h = (IP_t*)((u8*)(packet_buffer));
    if (ip_h->ip_p == IPPROTO_ICMP && ip_h->udp_src == ICMP_ECHO_REPLY) {
      if (ip_h->udp_xsum == pingseq - 1) {
        return 2;
      } else {
        printf("%x %x %x (%x)\n", ip_h->ip_p, ip_h->udp_src, ip_h->udp_xsum, pingseq);
      }
    }
  }
  /* if ethertype == IP && ICMP && ICMP response
     return 2 (got ping response)
     */
  return 0;
}

#define RXLOOP_DELAY 10000
#define RXLOOP_MAX 100
#define TXLOOP_MAX 4

int gbe_check_phy_status(int delay)
{
  u16 idle_start, idle_end;
  idle_end = v6gbe_reg_get_short(BSP_GBE_REG_STLINK);
  idle_start = v6gbe_reg_get_short(BSP_GBE_REG_STIDLE);
  if (!idle_end) {
    sprintf(bit_strerr,"sgmii link down");
    return -1;
  }
  udelay(delay);
  idle_end = v6gbe_reg_get_short(BSP_GBE_REG_STIDLE);
  if (idle_end != idle_start) {
    sprintf(bit_strerr,"sgmii decoding errors detected");
    return -1;
  }
  return 0;
}

int bit_v6gbe(int which, int subtest, u32 flags) {
  int ret = 0;
  int tx_loops = 0, rx_loops = 0;
  switch (subtest){
  case 0: 
    sprintf(bit_strerr,"not implemented");
    return -1;
    break;
  case 1: 
    return gbe_check_phy_status(1000000);
    break;
  case 2: 
    while (1) {
      v6gbe_flushrx();

      if (tx_loops++ >= TXLOOP_MAX) {
        ret = -2;
        break;
      }
      ret = v6gbe_sendping();
      if (ret) {
        sprintf(bit_strerr,"ping failed during transmit");
        return ret;
      }

      do {
        if (rx_loops++ >= RXLOOP_MAX) {
          break;
        }
        udelay(RXLOOP_DELAY);
        ret = v6gbe_rxloop();
        //printf("rxloop = %x\n",ret);
      } while (ret == 0 || ret == 1);
      if (ret == 2)
        break;
    }
    switch (ret) {
      case 2:
        return 0;
        break;
      case -1:
        sprintf(bit_strerr,"received invalid frame");
        return -1;
        break;
      case -2:
        sprintf(bit_strerr,"timed out waiting for ping");
        return -1;
        break;
      default:
        sprintf(bit_strerr,"unknown rx error waiting for echo response");
        return -1;
        break;
    };
    break;
  default: 
    sprintf(bit_strerr,"subtest not supported");
    return -1;
    break;
  }
  return 0;
}
