#ifndef __MSTRING_H__
#define __MSTRING_H__

typedef enum {
	BM_SEARCH_HTTP_CHUNKED_END=0,
	BM_SEARCH_HTTP_CONTENT_LEN_END,
	BM_SEARCH_INSERT_JS1,
	BM_SEARCH_INSERT_JS2,
	BM_SEARCH_QDH_FROM,
	BM_SEARCH_MAX=32
}BM_SEARCH_ID;

#define is_ptr(x)	__builtin_types_compatible_p(typeof(x), typeof(&x[0]))
#define app_memeql(b, m)	\
	(sizeof(char [1 - 2 * is_ptr(m)]) * 0 + !memcmp((b), (m), sizeof(m) - 1))

int bm_search(BM_SEARCH_ID id,const char *src, int len);
int bm_search_init(void);
void bm_search_fini(void);


typedef struct  _string
{
	char* c;	
	int l;	
}string;

static inline void new_string(string* s , char* c , int l)
{	
	s->c=c; 
	s->l=l;
}

#endif
