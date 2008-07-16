/*
 * Author: pancake < at > youterm <dot> com
 * 
 * pcap based io plugin for radare
 * 
 * Based on: wistumbler2/src/sniffer.c
 *
 * 2004/02/01 - Copylefted by pancake@phreaker.net
 */

// TODO: make it work
// TODO: use pcap_next and so

#include <main.h>
#include <plugin.h>
#include <pcap.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <net/route.h>

#ifndef __linux__
#include <net/if_ether.h>
#endif

#if __FreeBSD__
#include <net/ethernet.h>
#endif

#if __DragonFly__
#include <netinet/in.h>
#include <netinet/if_ether.h>
#endif

#define __FAVOR_BSD
#include <net/if_arp.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#define BUFSIZE 65540

#define PCAPIOFD 7362
static int pcapio_fd = -1;
static unsigned char *pcapio_buf = NULL;
static unsigned int pcapio_bufsz = 0;
static unsigned int pcapio_bufread = 0;
static pcap_t *pd; /* like fd but for pcap :) */

void dissect(char *user,struct pcap_pkthdr *hdr,u_char *pkt)
{
	struct ip *iph;
	struct ether_header *eth;
	struct in_addr_t *ips;
	struct tcphdr *tcp;
	struct udphdr *udp;
	struct arphdr *arp;
	struct arpreq *arr;
	int sport,dport;
	unsigned char *saddr,*daddr;
	char saddr_str[16], daddr_str[16];
	//unsigned char *ptr;

	eth = (struct ether_header*)(pkt);
	iph = (struct ip*)(pkt+sizeof(struct ether_header));
	arp = (struct arphdr*)(pkt+sizeof(struct ether_header));
	udp = (struct udphdr*)(pkt+sizeof(struct ip));
	tcp = (struct tcphdr*)(iph+sizeof(struct ip));

	saddr = (unsigned char *)&iph->ip_src.s_addr;
	daddr = (unsigned char *)&iph->ip_dst.s_addr;
	sprintf(saddr_str,"%d.%d.%d.%d", saddr[0], saddr[1],saddr[2],saddr[3]);
	sprintf(daddr_str,"%d.%d.%d.%d", daddr[0], daddr[1],daddr[2],daddr[3]);

	if (ntohs(eth->ether_type) == ETHERTYPE_IP)
	switch(iph->ip_p)
	{
	case IPPROTO_TCP:
		sport = ntohs( tcp->th_sport );
		dport = ntohs( tcp->th_dport );
		break;
	case IPPROTO_UDP:
		sport = ntohs( udp->uh_sport );
		dport = ntohs( udp->uh_dport );
		break;
	case IPPROTO_ICMP:
		printf("ICMP!\n");
		break;
	default:
		printf("invalid protocol (no udp/tcp) %d\n",iph->ip_p);
		return;
		break;
	} 
}

ssize_t pcapio_write(int fd, const void *buf, size_t count)
{
#if 0
        return socket_write(fd, buf, count);
#endif
}

ssize_t pcapio_read(int fd, void *buf, size_t count)
{
	u8 data[32000];
	int sz = 0;
	u64 s;

	if (config.seek > pcapio_bufsz)
		config.seek = pcapio_bufsz;

	if (fd == pcapio_fd) {
#if 0
		if (socket_ready(fd, 0, 10)) {
			sz = socket_read(fd, data, 32000);
			if (sz == -1) {
				eprintf("Connection closed\n");
				// XXX close foo
			}
#endif
			if (sz>0) {
				if (pcapio_buf)
					pcapio_buf = (u8 *)realloc(pcapio_buf, pcapio_bufsz+sz);
				else 	pcapio_buf = (u8 *)malloc(pcapio_bufsz+sz);
				memcpy(pcapio_buf+(int)pcapio_bufsz, data, sz);
				sprintf((char *)data, "_sockread_%d", pcapio_bufread++);
				flag_set((char *)data, pcapio_bufsz, 0);
				flag_set("_sockread_last", pcapio_bufsz, 0);
				pcapio_bufsz += sz;
			}
#if 0
		}
#endif
		if (config.seek < pcapio_bufsz) {
			s = count;
			if (count+config.seek > pcapio_bufsz)
				s = pcapio_bufsz-config.seek;
			memcpy(buf, pcapio_buf+config.seek, s);
			return s;
		}
	}
        return 0;
}

int pcapio_close(int fd)
{
#if 0
	return close(fd);
#endif
}

u64 pcapio_lseek(int fildes, u64 offset, int whence)
{
	switch(whence) {
	case SEEK_SET:
		return offset;
	case SEEK_CUR:
		if (config.seek+offset>pcapio_bufsz)
			return pcapio_bufsz;
		return config.seek+offset;
	case SEEK_END:
		return 0xffffffff;
	}
	return 0;
}

int pcapio_handle_fd(int fd)
{
	return (fd == pcapio_fd);
}

int pcapio_handle_open(const char *pathname)
{
	return (!memcmp(pathname, "pcap://", 7));
}

int pcapio_open(const char *pathname, int flags, mode_t mode)
{
	char iface[128];
	char buf[1024];
	char *ptr = buf;
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_handler callback;

strcpy(iface, "eth0");
/* pcap://eth0:port=33;host=192.168.2.3 */
/* replace ';' and '=' into spaces :) */

	strncpy(buf, pathname, 1000);

	if (!memcmp(ptr , "pcap://", 7)) {
		ptr= ptr+7;
		// port
		char *port = strchr(ptr, ':');
		if (port == NULL) {
			printf("No port defined.\n");
			return -1;
		}
		port[0] = '\0';
		if (strlen(ptr)==0) {
			// LISTEN HERE
			return -1;
		}
		pd=pcap_open_live(iface,BUFSIZE,1,69,errbuf);
		if (!pd) {
			return -1;
		}
		pcapio_fd = PCAPIOPD;
#ifdef HAVE_PCAP_SET_DATALINK
	pcap_set_datalink(pd,0);
#endif
	callback = (pcap_handler)&dissect;
	pcap_loop(pd,-1,callback,NULL);
	//pcap_lookupnet(Cfg.iface,&net,&mask,errbuf);
#if 0
		file   = strchr(pathname+12,'/');
		if (file == NULL) {
			printf("No remote file specified.\n");
			return -1;
		}
#endif
		// connect
#if 0
		pcapio_fd = socket_connect((char*)ptr, atoi(port+1));
		if (pcapio_fd>=0)
			printf("Connected to: %s at port %d\n", ptr, atoi(port+1));
		else	printf("Cannot connect to '%s' (%d)\n", ptr, atoi(port+1));
#endif
		pcapio_buf = (unsigned char *)malloc(1);
		pcapio_bufsz = 0;
//		config_set("file.write", "true");
		buf[0]='\0';
	}
	return pcapio_fd;
}

plugin_t socket_plugin = {
	.name        = "pcap",
	.desc        = "pcap stream access ( pcap://pcapexpr or pcapfile://file )",
	.init        = NULL,
	.debug       = NULL,
	.system      = NULL,
	.handle_fd   = pcapio_handle_fd,
	.handle_open = pcapio_handle_open,
	.open        = pcapio_open,
	.read        = pcapio_read,
	.write       = pcapio_write,
	.lseek       = pcapio_lseek,
	.close       = pcapio_close
};


#if 0
#include "main.h"

/* [ethernet][ip][tcp][payload] */
#if USE_PCAP
void dissect(char *user,struct pcap_pkthdr *hdr,u_char *pkt)
{
	struct ip *iph;
	struct ether_header *eth;
	struct in_addr_t *ips;
	struct tcphdr *tcp;
	struct udphdr *udp;
	struct arphdr *arp;
	struct arpreq *arr;
	int sport,dport;
	unsigned char *saddr,*daddr;
	char saddr_str[16], daddr_str[16];
	//unsigned char *ptr;

	if (Cfg.sniffer == false)
		return;

	eth = (struct ether_header*)(pkt);
	iph = (struct ip*)(pkt+sizeof(struct ether_header));
	arp = (struct arphdr*)(pkt+sizeof(struct ether_header));
	udp = (struct udphdr*)(pkt+sizeof(struct ip));
	tcp = (struct tcphdr*)(iph+sizeof(struct ip));

	saddr = (unsigned char *)&iph->ip_src.s_addr;
	daddr = (unsigned char *)&iph->ip_dst.s_addr;
	sprintf(saddr_str,"%d.%d.%d.%d", saddr[0], saddr[1],saddr[2],saddr[3]);
	sprintf(daddr_str,"%d.%d.%d.%d", daddr[0], daddr[1],daddr[2],daddr[3]);

	if (ntohs(eth->ether_type) == ETHERTYPE_IP)
	switch(iph->ip_p)
	{
	case IPPROTO_TCP:
		sport = ntohs( tcp->th_sport );
		dport = ntohs( tcp->th_dport );
		break;
	case IPPROTO_UDP:
		sport = ntohs( udp->uh_sport );
		dport = ntohs( udp->uh_dport );
		break;
	case IPPROTO_ICMP:
		printf("ICMP!\n");
		break;
	default:
		printf("invalid protocol (no udp/tcp) %d\n",iph->ip_p);
		return;
		break;
	} 

	if (ntohs(eth->ether_type) == ETHERTYPE_ARP)
	{
printf("ARP %d\n", ntohs(arp->ar_op));
		if (ntohs(arp->ar_op) == ARPOP_REQUEST)
		{
printf("REQ\n");
			if (arp->ar_pln == 4) // 4 = ipv4 == 32 bits
			{
printf("ipv4\n");
#if 0
TODO: ora8!! com coi funciona aixo?
			arr = (struct arpreq*)(pkt+sizeof(struct ether_header)+sizeof(struct arphdr));
			printf("%d.%d.%d.%d.%d\n",arr[20],arr[21],arr[22],arr[23],arr[24]);
			printf("%d.%d.%d.%d.%d\n",arr[24],arr[25],arr[26],arr[27],arr[28]);
#endif
			}

		}
	}

	if (App.stumbler == false)
		return;

	printf("#");
	fflush(stdout);

	if (ntohs(eth->ether_type)==ETHERTYPE_IP)
	{
		//ptr=(unsigned char*)&iph->ip_src.s_addr;

		if ( sport == 67 && dport == 68 )
		{
			// yeah! a BOOTP/DHCP server found at source IP addr!
			printf("DHCP server at '%s'\n", saddr_str);
			// may catch served IPs
		}

		//ips=ntohl(iph->ip_src.s_addr);
		//printf("\nFROM(%s)\n",inet_ntoa(iph->ip_src.s_addr));
		//printf("IP4: FROM: %d.%d.%d.%d ",ptr[0],ptr[1],ptr[2],ptr[3]);
		printf("+");
		fflush(stdout);

		if (saddr[0]!=192 && saddr[0]!=127 && saddr[0]!=10 && saddr[0]!=172 && saddr[0]!=224 && saddr[0]!=0 && saddr[0]!=255 && saddr[0]!=239)
		{
			printf("IP4: FROM %s, TO %s\n", saddr_str, daddr_str );

			printf("SHOULDBEBSSID[%2x:%2x:%2x:%2x:%2x:%2x] \n",
					eth->ether_dhost[0],
					eth->ether_dhost[1],
					eth->ether_dhost[2],
					eth->ether_dhost[3],
					eth->ether_dhost[4],
					eth->ether_dhost[5]);
		} else {
			sniff_add_address(saddr_str);
	//		printf("LAN: %d.%d.%d.%d\n",ptr[0],ptr[1],ptr[2],ptr[3]);
		}

		 if   (daddr[0]!=192 && daddr[0]!=127 && daddr[0]!=10 && daddr[0]!=172 && daddr[0]!=224 && daddr[0]!=0 && daddr[0]!=255 && daddr[0]!=239)
		{
		} else {
			sniff_add_address(daddr_str);
		}
	}
	//printf("TTL: %d FROM[%x:%x:%x:%x:%x:%x]\n",(unsigned char)IP->ip_ttl);
	//printf("ETHER_TYPE: %d\n",(short)eh->ether_type);
	//	printf("[%x] ",hdr->len);
#if 0
	switch(pkt[14])
	{
		case 0x45: printf("eth "); break;
		case 0x00: printf("arp "); break;
	}
	switch(pkt[17])
	{
		case 0x54: printf("icmp "); break;
	}
	printf("\t");
	for(i=14;i<30;i++)
		printf("%0.2x",(unsigned char )pkt[i]);
	printf("\n");
#endif
}
#else
void dissect(char *user,void *hdr, void *pkt)
{
	// NULL code
}

void wistumbler_sniff()
{
	// NULL
}
#endif

/*
TODO:
- support for pcap filters
*/
void 
wistumbler_sniff()
{
	pcap_handler callback;
	pcap_t *pd; /* like fd but for pcap :) */
	bpf_u_int32 net,mask;
	struct pcap_pkthdr header;
	const u_char *packet;
	char errbuf[PCAP_ERRBUF_SIZE];

	while (true)
	{
		if (!Cfg.sniffer || !App.stumbler)
		{
			sleep(1);
			continue;
		}
	callback = (pcap_handler)&dissect;
	//int datalink=0;
	//struct bpf_program filter;
	//char *filter_app="";
	errbuf[0]=0;

	//dev=pcap_lookupdev(errbuf);
	//printf("errbuf: %s\n",errbuf);
	if (Cfg.iface==NULL)
		break;

	pd=pcap_open_live(Cfg.iface,BUFSIZE,1,69,errbuf);
	if (!pd)
	{
		wistumbler_gui_error("No permission to capture data");
		break;
	}
	printf("Sniffer is ON\n");
#ifdef HAVE_PCAP_SET_DATALINK
	pcap_set_datalink(pd,0);
#endif
	//datalink=pcap_datalink(pd);
	pcap_lookupnet(Cfg.iface,&net,&mask,errbuf);
	/* set filter rules */
	//pcap_compile(pd,&filter,filter_app,0,net);
	//pcap_setfilter(pd,&filter);
	pcap_loop(pd,-1,callback,NULL);

	/* EXECUTION STOPS HERE : XXX : this code isn't used */
#if 0
	while(1)
	{
		packet=pcap_next(pd,&header);
		if (!packet) continue;
		printf("[*] %d\n",header.len);
		write(2,packet,header.len);
		printf("--\n");
	}
	pcap_close(pd);
#endif
	}
}

pthread_t th_sniffer=0;
#endif

void sniffer_start()
{
#if USE_PCAP
	pthread_create
	(
		&th_sniffer,
		NULL,
		(void *)wistumbler_sniff,
		(void *)0
	);
#endif
}

void sniffer_stop()
{
#if USE_PCAP
	pthread_join(th_sniffer,NULL);	
#endif
}
