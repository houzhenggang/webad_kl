#include "main.h"

#define	MNETLINK_PROTO		21
static struct sock *nl_sk = NULL;
static int nl_pid=0;

static void mnlk_rcv(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	
	nlh = nlmsg_hdr(skb);

	/* Bad header */
	if (nlh->nlmsg_len < NLMSG_HDRLEN)
	{
		printk("netlink bad header\n");
		return ;
	}

	nl_pid = nlh->nlmsg_pid;

	printk("netlink kla_nlk_rcv userpid:%d\n",nl_pid );
	
}

int mnlk_send( void *buf, int len)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	int	skb_len = NLMSG_SPACE(len);

	if (!nl_sk || !buf || len <=0)
	{
		printk("netlink send error\n");
		return -1;
	}

	skb = alloc_skb(skb_len, GFP_ATOMIC);
	if(NULL == skb)
	{
		printk("netlink alloc_skb error\n");
		return -1;
	}

	nlh = nlmsg_put(skb, 0, 0, 0, len, 0);
	/* Mark kernel process */
	NETLINK_CB(skb).dst_group = 1;
	memcpy(NLMSG_DATA(nlh), buf, len);
	netlink_broadcast(nl_sk, skb, 0, 1, GFP_ATOMIC);  

	return 0;
}

int mnlk_init(void)
{

	#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,14)
	  	struct netlink_kernel_cfg cfg = {  
	        .input = mnlk_rcv,  
	    };
	    nl_sk = netlink_kernel_create(&init_net, MNETLINK_PROTO, &cfg);  
	#else
	   	nl_sk = netlink_kernel_create(&init_net, MNETLINK_PROTO, 1,
				mnlk_rcv, NULL, THIS_MODULE);
	#endif
	
	if (NULL == nl_sk) 
	{
		printk("netlink_kernel_create error\n");
		return - 1;
	}
	return 0;
}

void mnlk_fini(void)
{
	if(nl_sk) 
	{
		//2.6...
		//sock_release(nl_sk->sk_socket);
		//3.1...
        netlink_kernel_release(nl_sk);  
	}
}
