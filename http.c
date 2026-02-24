/* ── http.h ───────────────────────────────────────────────────────────── */

#define HTTP_MAX_HEADERS 64
#define HTTP_MAX_URI     8192

typedef struct {
    char name[256];
    char value[1024];
} HttpHeader;

typedef struct {
    char method[16];
    char uri[HTTP_MAX_URI];
    char version[16];

    HttpHeader headers[HTTP_MAX_HEADERS];
    int        header_count;

    long long  content_length; /* -1 = non specificato */
    int        is_chunked;     /* 1 = Transfer-Encoding: chunked → 501 in v1 */
    size_t     header_bytes;   /* byte consumati per gli header */
} HttpRequest;

typedef struct {
    int  status_code;
    char reason[64];
    char version[16];

    HttpHeader headers[HTTP_MAX_HEADERS];
    int        header_count;

    long long  content_length; /* -1 = stream fino a chiusura */
    int        is_chunked;     /* passiamo attraverso in risposta */
} HttpResponse;
