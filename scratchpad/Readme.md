# references

https://www.tcpdump.org/pcap.html

https://web.archive.org/web/20251009111338/https://www.devdungeon.com/content/using-libpcap-c

https://www.tcpdump.org/linktypes.html

https://winpcap.org/pipermail/winpcap-users/2008-January/002306.html
https://www.winpcap.org/pipermail/winpcap-users/2007-September/002104.html

https://www.winpcap.org/docs/docs_412/html/group__wpcap__tut6.html


# explanation of the header structure
Typical layout for the Ethernet + IPv4 + UDP packet is
* Ethernet header == 14 bytes
* IP header : usually 20 bytes ( can be longer if IP options)
* UDP header: 8 bytes
* UDP payload: the rest of the packet

## Ethernet header -- 14 byes
* uchar ether_dhost[6]; // destination MAC address
* uchar ether_shost[6]; // source MAC address
* ushort ether_type;    // Protocol type ( 2 byes, e.g. 0x0800=IPV4)

## IP header -- 20 bytes (IPV4)
* u_char ver_ihl; // Version ( 4 bits) + IHL ( 4bits, *4 = header bytes) 
* u_char tos;     // Type of Service
* u_short tot_len;    // total length ( header + data) 
* u_short id;         // Identification
* u_short frag_off;    // Fragment offset
* u_char ttl;         // time to live
* u_char protocol;     // Protocl ( e.g. 17= UDP)
* u_short checksum;   // Header checksum
* struct in_addr saddr; // source IP address
* struct in_addr daddr; // destination IP address
* /// Options if ILH > 5 ... will have more variables
---- header length: int ip_len = (ip->var_ihl & 0x0F) * 4


 
##  UDP header format -- in network byte order
uint16_t src_port;
uint16_t dst_port;
uint16_t length;
uint16_t checksum;



