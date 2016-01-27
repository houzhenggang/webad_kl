#ifndef __MNETLINK_H__
#define __MNETLINK_H__

int mnlk_send( void *buf, int len);
int mnlk_init(void);
void mnlk_fini(void);

#endif

