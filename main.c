#include "main.h"

static int enlarge_skb(struct sk_buff *skb, unsigned int extra)
{
	if (skb->len + extra > 65535)
		return 0;

	if (pskb_expand_head(skb, 0, extra - skb_tailroom(skb), GFP_ATOMIC))
		return 0;

	return 1;
}

static void nf_nat_csum(struct sk_buff *skb, const struct iphdr *iph, void *data,
			int datalen, __sum16 *check, int oldlen)
{
	struct rtable *rt = skb_rtable(skb);

	if (skb->ip_summed != CHECKSUM_PARTIAL) {
		if (!(rt->rt_flags & RTCF_LOCAL) &&
		    (!skb->dev || skb->dev->features & NETIF_F_V4_CSUM)) {
			skb->ip_summed = CHECKSUM_PARTIAL;
			skb->csum_start = skb_headroom(skb) +
					  skb_network_offset(skb) +
					  iph->ihl * 4;
			skb->csum_offset = (void *)check - data;
			*check = ~csum_tcpudp_magic(iph->saddr, iph->daddr,
						    datalen, iph->protocol, 0);
		} else {
			*check = 0;
			*check = csum_tcpudp_magic(iph->saddr, iph->daddr,
						   datalen, iph->protocol,
						   csum_partial(data, datalen,
								0));
			if (iph->protocol == IPPROTO_UDP && !*check)
				*check = CSUM_MANGLED_0;
		}
	} else
		inet_proto_csum_replace2(check, skb,
					 htons(oldlen), htons(datalen), 1);
}

static void merge_contents(struct sk_buff *skb,
			    unsigned int dataoff,
			    unsigned int match_offset,
			    unsigned int match_len,
			    const char *rep_buffer,
			    unsigned int rep_len)
{
	unsigned char *data;

	BUG_ON(skb_is_nonlinear(skb));
	data = skb_network_header(skb) + dataoff;

	/* move post-replacement */
	memmove(data + match_offset + rep_len,
		data + match_offset + match_len,
		skb->tail - (skb->network_header + dataoff +
			     match_offset + match_len));

	/* insert data from buffer */
	memcpy(data + match_offset, rep_buffer, rep_len);

	/* update skb info */
	if (rep_len > match_len) {
		//printk("nf_nat_mangle_packet: Extending packet by "
			 //"%u from %u bytes\n", rep_len - match_len, skb->len);
		skb_put(skb, rep_len - match_len);
	} else {
		//printk("nf_nat_mangle_packet: Shrinking packet from "
			 //"%u from %u bytes\n", match_len - rep_len, skb->len);
		__skb_trim(skb, skb->len + rep_len - match_len);
	}

	/* fix IP hdr checksum information */
	ip_hdr(skb)->tot_len = htons(skb->len);
	ip_send_check(ip_hdr(skb));
}

int http_merge_packet(struct sk_buff *skb,
			       unsigned int match_offset,
			       unsigned int match_len,
			       const char *rep_buffer,
			       unsigned int rep_len)
{
	struct iphdr *iph;
	struct tcphdr *tcph;
	int oldlen, datalen;

   // printk("!! match_offset=%d,match_len=%d,rep_buffer=%s,rep_len=%d\n",match_offset,match_len,rep_buffer,rep_len);
    
	if (!skb_make_writable(skb, skb->len))
	{	
	    printk("can't skb_make_writable\n");
	    return 0;
    }
	if (rep_len > match_len &&
	    rep_len - match_len > skb_tailroom(skb) &&
	    !enlarge_skb(skb, rep_len - match_len))
	{
	     printk("can't enlarge_skb\n");
	    return 0;
    }
	SKB_LINEAR_ASSERT(skb);

	iph = ip_hdr(skb);
	tcph = (void *)iph + iph->ihl*4;

	oldlen = skb->len - iph->ihl*4;
	merge_contents(skb, iph->ihl*4 + tcph->doff*4,
			match_offset, match_len, rep_buffer, rep_len);

	datalen = skb->len - iph->ihl*4;
	nf_nat_csum(skb, iph, tcph, datalen, &tcph->check, oldlen);

	return 1;
}

void change_seq(struct sk_buff *skb , unsigned long last_seq)
{
	struct iphdr *iph;
	struct tcphdr *tcph;
	int data_len;
	if (!skb_make_writable(skb, skb->len))
	{	
	    printk("can't skb_make_writable\n");
	    return;
    }
	iph = ip_hdr(skb);
	tcph = (void *)iph + iph->ihl*4;
	tcph->seq=htonl(last_seq);
	data_len = skb->len - iph->ihl*4;
	nf_nat_csum(skb, iph, tcph, data_len, &tcph->check, data_len);
}

static int __init app_init(void)
{
	printk("Loading Module\n");

	if(http_session_init())
	{
		printk("http_session_init failed\n");
	}
		
	if(-1==bm_search_init())
	{
		printk("bm_search_init failed\n");
	}
	if(-1==mnlk_init())
	{
		printk("mnlk_init failed\n");
	}
	if(-1==plug_init())
	{
		printk("plug_init failed\n");
	}
	
	if(-1==capture_init())
	{
		printk("capture_init failed\n");
	}
	
	printk( "Finished Loading: Ready for capture\n");

	return 0;
}

static void __exit app_exit(void)
{
	printk("Module Unloading\n");

	http_session_fini();	
	bm_search_fini();
	mnlk_fini();
	plug_fini();
	capture_fini();

	printk("Unloaded\n");
	return;
}

module_init(app_init);
module_exit(app_exit);

MODULE_DESCRIPTION("app check");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("yj");


