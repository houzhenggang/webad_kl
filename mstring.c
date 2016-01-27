#include "main.h"

struct bm_search_data
{
	int id;
	char* keystr;
	int keylen;
	struct ts_config *conf;
};

static struct bm_search_data bm_search_data_arr[] = {
	{BM_SEARCH_HTTP_CHUNKED_END, "0\r\n", 3,NULL},
	{BM_SEARCH_HTTP_CONTENT_LEN_END, "</html>", 7,NULL},
	{BM_SEARCH_INSERT_JS1, "<!DOCTYPE", 9,NULL},
	{BM_SEARCH_INSERT_JS2, "<html", 5,NULL},
	{BM_SEARCH_MAX, NULL, 0,NULL}
};

int bm_search(BM_SEARCH_ID id,const char *src, int len)
{
	int pos;
	struct ts_state state;
	
	pos = textsearch_find_continuous(bm_search_data_arr[id].conf, &state, src, len);
	
	if (pos != UINT_MAX)
	{
		return pos;
	}
		
	
	return -1;
}

int bm_search_init(void)
{
	struct bm_search_data *tmp;
	for (tmp = bm_search_data_arr; tmp->id<BM_SEARCH_MAX; tmp++)
	{
		tmp->conf = textsearch_prepare("bm", tmp->keystr, tmp->keylen,GFP_KERNEL, TS_AUTOLOAD);
		if (IS_ERR(tmp->conf)) 
		{
			tmp->conf=NULL;
			return -1;
		}
			
	}
	return 0;

}
void bm_search_fini(void)
{
	struct bm_search_data *tmp;
	for (tmp = bm_search_data_arr; tmp->id<BM_SEARCH_MAX; tmp++)
	{
		if(!IS_ERR(tmp->conf))
		{
			textsearch_destroy(tmp->conf);
		}
	}
}


