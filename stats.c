#include "stats.h"
#include <string.h>
#include <stdio.h>

void stats_init(Stats *s) {
    memset(s, 0, sizeof(*s));
    s->start_time = time(NULL);
}

void stats_record(Stats *s, int status, uint64_t recv, uint64_t sent) {
    s->requests_total++;
    if      (status >= 200 && status < 300) s->requests_2xx++;
    else if (status >= 400 && status < 500) s->requests_4xx++;
    else if (status >= 500)                 s->requests_5xx++;
    if (status == 502 || status == 504)     s->errors_backend++;
    s->bytes_received += recv;
    s->bytes_sent     += sent;
}

void stats_to_json(const Stats *s, char *buf, size_t buflen) {
    long uptime = (long)(time(NULL) - s->start_time);
    snprintf(buf, buflen,
        "{\n"
        "  \"uptime_seconds\":    %ld,\n"
        "  \"requests_total\":    %llu,\n"
        "  \"requests_2xx\":      %llu,\n"
        "  \"requests_4xx\":      %llu,\n"
        "  \"requests_5xx\":      %llu,\n"
        "  \"errors_backend\":    %llu,\n"
        "  \"bytes_received\":    %llu,\n"
        "  \"bytes_sent\":        %llu\n"
        "}\n",
        uptime,
        (unsigned long long)s->requests_total,
        (unsigned long long)s->requests_2xx,
        (unsigned long long)s->requests_4xx,
        (unsigned long long)s->requests_5xx,
        (unsigned long long)s->errors_backend,
        (unsigned long long)s->bytes_received,
        (unsigned long long)s->bytes_sent);
}
