#ifndef __PLUG_H__
#define __PLUG_H__

//see file plug.h PLUG_TYPE_MAX
typedef enum 
{
	PLUG_EXTERN_TYPE_OTHER=0,
	PLUG_EXTERN_TYPE_REQUEST,
	PLUG_EXTERN_TYPE_RESPONSE,
	PLUG_TYPE_MAX
}PLUG_EXTERN_TYPE;

void new_plug(int (*plug_hook)(void *) , int plug_type_num);

int plug_hook(void *data , int plug_type_num);

int plug_init();
void plug_fini();

#endif 
