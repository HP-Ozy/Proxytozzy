/* ── proxy.h ──────────────────────────────────────────────────────────── */

typedef struct {
    int         client_fd;
    char        client_ip[48];   /* IPv4 o IPv6 */
    int         client_port;
    struct timespec start_time;  /* per calcolo latenza */

    HttpRequest  req;
    HttpResponse resp;

    const Route *route;          /* route matchata */
    int          backend_fd;     /* -1 se non connesso */

    /* Risultato */
    int    status_sent;          /* HTTP status inviato al client */
    size_t bytes_from_client;
    size_t bytes_to_client;
} RequestContext;
