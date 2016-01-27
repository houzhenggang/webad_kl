#ifndef __MAIN_H__
#define __MAIN_H__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/inetdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/in.h>
#include <linux/ctype.h>
#include <linux/decompress/mm.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/list.h>
#include <linux/netlink.h> 
#include <linux/socket.h> 
#include <linux/types.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netdevice.h>

#include <net/netfilter/nf_log.h>
#include <net/netfilter/nf_nat_helper.h>
#include <net/net_namespace.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <net/protocol.h>
#include <net/route.h>
#include <net/tcp.h>
#include <net/ip_fib.h>
#include <net/rtnetlink.h>
#include <net/ip_fib.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>


#include "capture.h"
#include "mstring.h"
#include "mnetlink.h"
#include "http_session.h"
#include "plug.h"

typedef enum{	
	RESULT_IGNORE,
	RESULT_FROM_CLIENT,	
	RESULT_FROM_SERVER,	
	RESULT_OTHER
}RESULT;

struct tuple4
{	
	unsigned short sp,dp;	
	unsigned long sip,dip;
};

struct user_skb
{
	struct sk_buff *skb;
	struct iphdr *iph;
	struct tcphdr *tcph;
	struct tuple4 t4;
	unsigned long seq,ack_seq;
	unsigned char* data;
	int data_len;
	RESULT result;
};

typedef enum 
{
	HTTP_TYPE_OTHER,
	HTTP_TYPE_REQUEST_POST,
	HTTP_TYPE_REQUEST_GET,
	HTTP_TYPE_RESPONSE
	
}HTTP_TYPE;

typedef enum 
{
	HTTP_RESPONSE_TYPE_OTHER,
	HTTP_RESPONSE_TYPE_CHUNKED,
	HTTP_RESPONSE_TYPE_CONTENTLENGTH
	
}HTTP_RESPONSE_TYPE;

struct http_hdr
{
	HTTP_TYPE http_type;
	HTTP_RESPONSE_TYPE res_type;
	int httph_len;
	//////GET/////////////
	string uri;
	string host;
	string user_agent;
	string accept_encoding;//gzip,deflate
	string accept;//text/html
	////RESPONSE/////////
	string error_code; //200 404 
	string content_encoding;//gzip,deflate,compress
	string content_type;//text/html
	string content_length;
	string transfer_encoding;	//chunked
};

struct http_request
{
	struct list_head list;
	struct tuple4 t4;
	struct http_hdr hhdr;
	int response_num;
	int js_len;
	int qdh_modify_len;
	int redirect_len;
	unsigned long curr_seq;
	int curr_data_len;
	int last_time;
	struct user_skb *curr_skb;
};

int http_merge_packet(struct sk_buff *skb,
			       unsigned int match_offset,
			       unsigned int match_len,
			       const char *rep_buffer,
			       unsigned int rep_len);

void change_seq(struct sk_buff *skb , unsigned long last_seq);

#endif
