#ifndef __TCPSEND_H__
#define __TCPSEND_H__

/* Remote information */
#define REMOTE_IP           "192.168.0.112" 
#define REMOTE_PORT         44444

#ifdef __cplusplus
extern "C" {
#endif

bool_t tcpsnd_init(void);
void tcpsnd_start(void);

#ifdef __cplusplus
}
#endif

#endif    
