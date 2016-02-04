#include "main.h"
#include "plug_extern.h"

struct plug_info
{
	struct list_head list;
	int plug_type_num;
	int (*plug)(void*);
};

struct list_head plug_list[PLUG_TYPE_MAX];

void new_plug(int (*plug_hook)(void *) , int plug_type_num)
{

	struct plug_info* new;

	if(plug_type_num>=PLUG_TYPE_MAX)
		return;
	
	new = kzalloc(sizeof(struct plug_info), GFP_KERNEL);
	if(!new)
		return;
	
	new->plug_type_num=plug_type_num;
	new->plug=plug_hook;
	list_add_rcu(&(new->list), &plug_list[plug_type_num]);
	
}

int plug_hook(void *data , int plug_type_num)
{
	struct plug_info *tmp;
	if(plug_type_num>=PLUG_TYPE_MAX)
		return -1;
	list_for_each_entry_rcu(tmp, &plug_list[plug_type_num], list) 	
	{
		if(plug_type_num == tmp->plug_type_num)
			tmp->plug(data);
	}
	return 0;
}

int plug_init(void)
{

	int i;
	for(i=0;i<PLUG_TYPE_MAX;i++)
	{
		INIT_LIST_HEAD(&plug_list[i]);
	}

	init_plug_extern();
		
	return 0;
}

void plug_fini(void)
{
	struct plug_info  *tmp;
	int i;	
	for(i = 0; i < PLUG_TYPE_MAX; i++)
	{		
		list_for_each_entry_rcu(tmp, &plug_list[i], list) 		
		{
			list_del_rcu(&tmp->list);
			kfree(tmp);	
		}	
	}
}

