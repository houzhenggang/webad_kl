#include "main.h"

struct list_head http_session_list_head; 
static struct kmem_cache *http_session_cachep=NULL;
#define MAX_HTTP_SESSION_NUN 8
#define MAX_HTTP_SESSION_TIMEOUT_SEC 20
static int http_session_num=0;


int change_accept_encoding(struct user_skb* u_skb , struct http_hdr* hhdr)
{
	int match_offset;
	char* http_content;
	
    http_content=u_skb->data;
	
	if(hhdr->accept_encoding.l <= 0)
		return -1;
	
	if(!strncasecmp(hhdr->accept_encoding.c, "Accept-Encoding: gzip" ,21))
	{
		match_offset=hhdr->accept_encoding.c-http_content;
		if(!http_merge_packet(u_skb->skb,match_offset,1,"B",1))
		{
			return -1;
		}
		return 0;
	}

	return -1;
}

int http_request_filter(struct http_hdr* hhdr)
{	
	//maybe not http protocal
	if(hhdr->host.l <= 0)
		return -1;

	//not html page
	if(hhdr->accept.l <= 0)
		return -1;
	
	if(strncasecmp(hhdr->accept.c, "Accept: text/html" ,17)!=0)
	{
		return -1;
	}
	return 0;
}

int http_response_filter(struct http_hdr* hhdr)
{	
	//normal page
	if(hhdr->error_code.l <= 0)
		return -1;
	if(strncasecmp(hhdr->error_code.c , "HTTP/1.1 200 OK" , 15)!=0)
	{
		return -1;
	}
	//not html page
	if(hhdr->content_type.l <= 0)
		return -1;
	if(strncasecmp(hhdr->content_type.c, "Content-Type: text/html" ,23)!=0)
	{
		return -1;
	}
	return 0;
}

struct http_request* new_http_request(struct tuple4* t4)
{
	struct http_request* new_httpr;
	rcu_read_lock();
	new_httpr=kmem_cache_alloc(http_session_cachep, GFP_ATOMIC);
	if(!new_httpr)
	{
		rcu_read_unlock();
		return NULL;
	}
	memset(new_httpr, '\0' ,sizeof(struct http_request));
	memcpy(&new_httpr->t4, t4 ,sizeof(struct tuple4));
	new_httpr->last_time = get_seconds();
	list_add_rcu(&(new_httpr->list), &http_session_list_head);
	http_session_num++;
	rcu_read_unlock();
	return new_httpr;
}


void free_http_request(struct http_request* httpr)
{
	if(!httpr)
		return;
	//debug_log("free_http_request");
	rcu_read_lock();
	list_del_rcu(&httpr->list);	
	kmem_cache_free(http_session_cachep, httpr);
	http_session_num--;
	rcu_read_unlock();
}


struct http_request* find_http_request(struct user_skb* u_skb)
{
	struct http_request *tmp;
	rcu_read_lock();
	list_for_each_entry_rcu(tmp, &http_session_list_head, list) 		
	{
		if(u_skb->t4.sip == tmp->t4.sip &&
			u_skb->t4.dip== tmp->t4.dip &&
			u_skb->t4.sp == tmp->t4.sp &&
			u_skb->t4.dp == tmp->t4.dp
			)
		{
			rcu_read_unlock();
			return tmp;
		}
		else if(u_skb->t4.sip == tmp->t4.dip &&
			u_skb->t4.dip== tmp->t4.sip &&
			u_skb->t4.sp == tmp->t4.dp &&
			u_skb->t4.dp == tmp->t4.sp &&
			u_skb->ack_seq == tmp->curr_seq + tmp->curr_data_len)
		{
			rcu_read_unlock();
			return tmp;
		}
		
	}	
	rcu_read_unlock();
	return NULL;
}

int is_html_end(struct http_request* httpr)
{
	char* http_content = httpr->curr_skb->data;
	int http_content_len = httpr->curr_skb->data_len;
	
	char* search_data ;

	if(httpr->response_num == 1)
		return 0;
	
	if(httpr->hhdr.res_type==HTTP_RESPONSE_TYPE_CHUNKED)
	{
	 	if(http_content_len<8)
		{
			search_data = http_content;
		}
		else
		{
			search_data = http_content+(http_content_len - 8);
		}
		
		if(bm_search(BM_SEARCH_HTTP_CHUNKED_END ,search_data, 8)>=0)
		{
			return 1;
		}

		if(http_content_len == 1)
		{
			if(search_data[0] == '0')
			{
				return 1;
			}
		}
		
	}
	else if(httpr->hhdr.res_type==HTTP_RESPONSE_TYPE_CONTENTLENGTH)
	{
		if(http_content_len<64)
		{
			search_data = http_content;
		}
		else
		{
			search_data = http_content+(http_content_len - 64);
		}
		
		if(bm_search(BM_SEARCH_HTTP_CONTENT_LEN_END ,search_data, 64)>=0)
		{
			return 1;
		}
    }	
	
	return 0;
}

int decode_http(struct http_hdr* hhdr, struct user_skb* u_skb)
{
	int i = 0;
	char *start,*end;
	char* http_head;
	char* http_data;
	int http_len;

	http_len = u_skb->data_len;
	http_head = u_skb->data;
	if(!http_head)
		return -1;

	//////////////http_head_start///////////
	if(!strncasecmp(http_head,"POST ",5))
	{
		hhdr->http_type=HTTP_TYPE_REQUEST_POST;
		return -1;
	}
	else if(!strncasecmp(http_head,"GET " ,4))
	{
		hhdr->http_type=HTTP_TYPE_REQUEST_GET;
	}
	else if(!strncasecmp(http_head,"HTTP/1.",7))
	{
			
		hhdr->http_type=HTTP_TYPE_RESPONSE;
	}
	else 
	{
		hhdr->http_type=HTTP_TYPE_OTHER;
		return 0;
	}
	
	end=start=http_head;
	while(i<http_len)
	{	
		if(memcmp(end , "\r\n" , 2)!=0 )
		{
			i++;
			end++;
			continue;
		}

		if(!strncasecmp(start,"GET " ,4))
		{
			new_string(&hhdr->uri, start , end-start);
		}
		else if(!strncasecmp(start,"HTTP/1.",7))
		{
			new_string(&hhdr->error_code , start , end-start);
		}
		else if(!strncasecmp(start,"Host: ",6))
		{
			new_string(&hhdr->host , start , end-start);
		}
		else if(!strncasecmp(start,"Accept-Encoding: ",17))
		{
			new_string(&hhdr->accept_encoding, start , end-start);
		}
		else if(!strncasecmp(start,"Accept: ",8))
		{
			new_string(&hhdr->accept, start , end-start);
		}
		else if(!strncasecmp(start,"User_Agent: ",12))
		{
			new_string(&hhdr->user_agent, start , end-start);
		}
		else if(!strncasecmp(start,"Content-Type: ",14))
		{
			new_string(&hhdr->content_type, start , end-start);
		}
		else if(!strncasecmp(start,"Content_Encoding: ",18))
		{
			new_string(&hhdr->content_encoding, start , end-start);
		}
		else if(!strncasecmp(start,"Content-Length: ",16))
		{
			new_string(&hhdr->content_length, start , end-start);
			hhdr->res_type=HTTP_RESPONSE_TYPE_CONTENTLENGTH;
		}
		else if(!strncasecmp(start,"Transfer-Encoding: chunked",26))
		{
			new_string(&hhdr->transfer_encoding, start , end-start);
			hhdr->res_type=HTTP_RESPONSE_TYPE_CHUNKED;
		}
		else if(!strncasecmp(start,"Transfer-Encoding:  chunked",27))
		{
			new_string(&hhdr->transfer_encoding, start , end-start);
			hhdr->res_type=HTTP_RESPONSE_TYPE_CHUNKED;
		}
		i+=2;
		end+=2;
		start=end;
		if(!memcmp(start , "\r\n" , 2))
		{
			start+=2;
			break;
		}
		
	}

	//////////////http_head_end///////////
	http_data=start;
	if(!http_data)
	{
		return -1;
	}
	
	//////////////include /r/n/r/n ///////
	hhdr->httph_len = http_data - http_head;
	
	return 0;
}


void http_timeout(void)
{
	struct http_request *cursor , *tmp;
	long current_sec;

	//debug_log("%d" , http_session_num);
	if(http_session_num	< MAX_HTTP_SESSION_NUN)
		return;
	
	current_sec=get_seconds();

    list_for_each_entry_safe(cursor, tmp, &http_session_list_head, list)
    {
  		 if(current_sec - cursor->last_time > MAX_HTTP_SESSION_TIMEOUT_SEC)
		 {
		 	//debug_log("http session timeout");
			free_http_request(cursor);
		 }
	}
}

void process_http(void *data)
{
	struct user_skb* u_skb = (struct user_skb*)data;
	struct http_request* new_httpr=NULL;

	http_timeout();
	
	switch(u_skb->result)
	{
		case RESULT_FROM_CLIENT:
			{
				new_httpr = find_http_request(u_skb);
				//first request packet must get
				if(!new_httpr)
				{
					new_httpr = new_http_request(&u_skb->t4);
					if(-1 == decode_http(&new_httpr->hhdr, u_skb))
					{
						goto result_ignore;
					}
					
					if(new_httpr->hhdr.http_type != HTTP_TYPE_REQUEST_GET)
					{
						goto result_ignore;
					}
					
					if(-1 == http_request_filter(&new_httpr->hhdr))
					{
						goto result_ignore;
					}
					
					if(-1 == change_accept_encoding(u_skb , &new_httpr->hhdr))
					{
						goto result_ignore;
					}
					
					goto result_client;
					
				}
				
				//repeat session request
				//debug_log("repeat");
				goto result_ignore;
			}
		case RESULT_FROM_SERVER:
			{
				new_httpr =  find_http_request(u_skb);
				//no find
				if(!new_httpr)
				{
					goto result_ignore;
				}
				
				//first response packet must have http head
				if(new_httpr->hhdr.http_type == HTTP_TYPE_REQUEST_GET)
				{
				
					if(-1 == decode_http(&new_httpr->hhdr , u_skb))
					{
						goto result_ignore;
					}
					
					if(new_httpr->hhdr.http_type != HTTP_TYPE_RESPONSE)
					{
						goto result_ignore;
					}
					
					if(-1 == http_response_filter(&new_httpr->hhdr))
					{
						goto result_ignore;
					}
					

					goto result_server;
				}
				//other response packet no need decode http head
				else if(new_httpr->hhdr.http_type == HTTP_TYPE_RESPONSE)
				{
					goto result_server;
				}
				else
				{
					goto result_ignore;
				}
				break;
			}
			
		default:break;	
	}

	result_client:
		new_httpr->curr_seq = u_skb->seq ;
		new_httpr->curr_data_len = u_skb->data_len;
		new_httpr->curr_skb = u_skb;
		plug_hook(new_httpr , PLUG_EXTERN_TYPE_REQUEST);
		return;
	result_server:
		new_httpr->response_num++;
		new_httpr->curr_skb = u_skb;
		plug_hook(new_httpr , PLUG_EXTERN_TYPE_RESPONSE);
		if(is_html_end(new_httpr))
		{
			free_http_request(new_httpr);
		}
		return;
	result_ignore:
		free_http_request(new_httpr);
		return;
}

int http_session_init(void)
{
	INIT_LIST_HEAD(&http_session_list_head);
	
	http_session_cachep = kmem_cache_create("http_session", 	  
			sizeof(struct http_request), 0,					   
			0, NULL);	if(!http_session_cachep)	
		{		
			return -1;
		}
	return 0;
}

void http_session_fini(void)
{
	struct http_request* tmp;
	list_for_each_entry_rcu(tmp, &http_session_list_head, list) 		
	{
		list_del_rcu(&tmp->list);
		kmem_cache_free(http_session_cachep, tmp);	
	}	

	if(http_session_cachep)
	{	
		kmem_cache_destroy(http_session_cachep);	
	}

}

