#ifndef __HTTPD_H__
#define __HTTPD_H__

#define HTTPD_PORT      80
#define MAX_CONNECTIONS 4

#ifdef __cplusplus
extern "C" {
#endif

bool_t httpd_init(void);
void httpd_start(void);


#ifdef __cplusplus
}
#endif

#endif
