#include "main.h"

static unsigned int package_capture(unsigned int hooknum,
		struct sk_buff *skb,
		const struct net_device *in,
		const struct net_device *out,
		int (*okfn)(struct sk_buff *))
{
	struct user_skb u_skb;

	if(NULL == skb)
	{
		return NF_ACCEPT;
	}

	rcu_read_lock();
	
	if (0 != skb_linearize(skb))
	{
		printk(" !!! skb_linearize error\n");
		goto OUT;
	}
	
	u_skb.skb = skb;

	u_skb.iph = ip_hdr(skb);
	if(NULL == u_skb.iph)
	{
		goto OUT;
	}

	/* Is protocol TCP */
	if(u_skb.iph->protocol != IPPROTO_TCP)
	{
		//printk("it is not TCP\n");
		goto OUT;
	}

	/*port 80 is used for http*/
	u_skb.tcph = (struct tcphdr *)((u8 *)(u_skb.iph) + (u_skb.iph->ihl<<2));

	u_skb.t4.sip = u_skb.iph->saddr;
	u_skb.t4.dip = u_skb.iph->daddr;
	u_skb.t4.sp = ntohs(u_skb.tcph->source);
	u_skb.t4.dp = ntohs(u_skb.tcph->dest);

	u_skb.seq = ntohl(u_skb.tcph->seq);
	u_skb.ack_seq = ntohl(u_skb.tcph->ack_seq);
	
	u_skb.data = (unsigned char *)(u_skb.tcph) + (u_skb.tcph->doff<<2);
	u_skb.data_len = ntohs(u_skb.iph->tot_len) - (u_skb.iph->ihl<<2) - (u_skb.tcph->doff*4);

	if(u_skb.data_len <=0)
	{
		goto OUT;
	}
	//maybe http
	if(u_skb.t4.dp == 80)
	{	
		//printk("client\n");
		u_skb.result = RESULT_FROM_CLIENT;
		process_http(&u_skb);
	}
	else if(u_skb.t4.sp == 80)
	{
		//printk("server\n");
		u_skb.result = RESULT_FROM_SERVER;
		process_http(&u_skb);
	}
	else
	{
		u_skb.result = RESULT_IGNORE;
	}

	OUT:
	rcu_read_unlock();
	return NF_ACCEPT;
	
}


static struct nf_hook_ops capture_ops[] =
{

    {
        {NULL,NULL},
        .hook               = (nf_hookfn *)package_capture,
        .owner              = THIS_MODULE,
        .pf                 = PF_INET,
        .hooknum            = NF_INET_LOCAL_IN,
        .priority           = NF_IP_PRI_FIRST,
    },
    
    {
        {NULL,NULL},
        .hook               = (nf_hookfn *)package_capture,
        .owner              = THIS_MODULE,
        .pf                 = PF_INET,
        .hooknum            = NF_INET_LOCAL_OUT,
        .priority           = NF_IP_PRI_FIRST,
    },
};

int __init capture_init(void)
{
    int ret;
	
    ret = nf_register_hooks(capture_ops, ARRAY_SIZE(capture_ops));
    if(ret < 0)
    {
        return -1;
    }

    return 0;
}

void capture_fini(void)
{

    nf_unregister_hooks(capture_ops, ARRAY_SIZE(capture_ops));
    
    printk(KERN_EMERG"!!!!!!!!! capture_exit\n");
}

