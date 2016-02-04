#include "main.h"

/* Frobs data inside this packet, which is linear. */
static void mangle_contents(struct sk_buff *skb,
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
		//	 "%u from %u bytes\n", rep_len - match_len, skb->len);
		skb_put(skb, rep_len - match_len);
	} else {
		//printk("nf_nat_mangle_packet: Shrinking packet from "
		//	 "%u from %u bytes\n", match_len - rep_len, skb->len);
		__skb_trim(skb, skb->len + rep_len - match_len);
	}

	/* fix IP hdr checksum information */
	ip_hdr(skb)->tot_len = htons(skb->len);
	ip_send_check(ip_hdr(skb));
}

/* Unusual, but possible case. */
static int enlarge_skb(struct sk_buff *skb, unsigned int extra)
{
	if (skb->len + extra > 65535)
		return 0;

	if (pskb_expand_head(skb, 0, extra - skb_tailroom(skb), GFP_ATOMIC))
		return 0;

	return 1;
}

/* Generic function for mangling variable-length address changes inside
 * NATed TCP connections (like the PORT XXX,XXX,XXX,XXX,XXX,XXX
 * command in FTP).
 *
 * Takes care about all the nasty sequence number changes, checksumming,
 * skb enlargement, ...
 *
 * */
int http_merge_packet(struct sk_buff *skb,
			 unsigned int match_offset,
			 unsigned int match_len,
			 const char *rep_buffer,
			 unsigned int rep_len)
{
	struct rtable *rt = skb_rtable(skb);
	struct iphdr *iph;
	struct tcphdr *tcph;
	int oldlen, datalen;

	if (!skb_make_writable(skb, skb->len))
		return 0;

	if (rep_len > match_len &&
	    rep_len - match_len > skb_tailroom(skb) &&
	    !enlarge_skb(skb, rep_len - match_len))
		return 0;

	SKB_LINEAR_ASSERT(skb);

	iph = ip_hdr(skb);
	tcph = (void *)iph + iph->ihl*4;

	oldlen = skb->len - iph->ihl*4;
	mangle_contents(skb, iph->ihl*4 + tcph->doff*4,
			match_offset, match_len, rep_buffer, rep_len);

	datalen = skb->len - iph->ihl*4;
	if (skb->ip_summed != CHECKSUM_PARTIAL) {
		if (!(rt->rt_flags & RTCF_LOCAL) &&
		    skb->dev->features & NETIF_F_V4_CSUM) {
			skb->ip_summed = CHECKSUM_PARTIAL;
			skb->csum_start = skb_headroom(skb) +
					  skb_network_offset(skb) +
					  iph->ihl * 4;
			skb->csum_offset = offsetof(struct tcphdr, check);
			tcph->check = ~tcp_v4_check(datalen,
						    iph->saddr, iph->daddr, 0);
		} else {
			tcph->check = 0;
			tcph->check = tcp_v4_check(datalen,
						   iph->saddr, iph->daddr,
						   csum_partial(tcph,
								datalen, 0));
		}
	} else
		inet_proto_csum_replace2(&tcph->check, skb,
					 htons(oldlen), htons(datalen), 1);
	return 1;
}


int http_repair_packet(struct sk_buff *skb , int seqoff)
{
	
	struct iphdr *iph;
	struct tcphdr *tcph;
	unsigned long new_seq;
	
	if (!skb_make_writable(skb, skb->len))
	{	
		printk("can't skb_make_writable\n");
		return 0;
	}
	iph = ip_hdr(skb);
	tcph = (void *)iph + iph->ihl*4;
	new_seq = htonl(ntohl(tcph->seq) + seqoff);
	
	inet_proto_csum_replace2(&tcph->check, skb, tcph->seq, new_seq, 1);
	tcph->seq=new_seq;
	return 1;
}



