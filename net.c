#ifndef CPROXY_NET_H
#define CPROXY_NET_H

#include <sys/types.h>

/* Crea socket in ascolto. Ritorna fd o -1. */
int net_listen(const char *host, int port, int backlog);

/* Connette a host:port con timeout_ms. Ritorna fd o -1. */
int net_connect(const char *host, int port, int timeout_ms);

/* Imposta SO_RCVTIMEO e SO_SNDTIMEO. Ritorna 0 o -1. */
int net_set_timeouts(int fd, int recv_ms, int send_ms);

/* TCP_NODELAY — riduce latenza per proxy */
int net_set_nodelay(int fd);

/* Copia fino a max_bytes da src_fd a dst_fd usando buf[buflen].
   Se content_length >= 0, copia esattamente quel numero di byte.
   Se content_length < 0, copia fino a EOF su src_fd.
   Ritorna byte copiati, -1 su errore, -2 su timeout. */
ssize_t net_pipe(int src_fd, int dst_fd,
                 char *buf, size_t buflen,
                 long long content_length);

/* Scrive esattamente n byte. Ritorna n o -1 su errore. */
ssize_t net_write_all(int fd, const char *buf, size_t n);

#endif /* CPROXY_NET_H */
