#ifndef __TCPSRV_H__
#define __TCPSRV_H__

#define TCPSRV_PORT      80 // 44444

#ifdef __cplusplus
extern "C" {
#endif

bool_t tcpsrv_init(void);
void tcpsrv_start(void);


#ifdef __cplusplus
}
#endif

#endif
