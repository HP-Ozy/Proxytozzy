#include "config.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void config_defaults(Config *cfg) {
    memset(cfg, 0, sizeof(*cfg));
    strncpy(cfg->listen_host, "0.0.0.0", sizeof(cfg->listen_host) - 1);
    cfg->listen_port        = 8080;
    cfg->backlog            = 128;
    cfg->connect_timeout_ms = 5000;
    cfg->read_timeout_ms    = 30000;
    cfg->write_timeout_ms   = 30000;
    cfg->max_header_bytes   = 8192;
    cfg->max_body_bytes     = 10 * 1024 * 1024; /* 10 MB */
    cfg->preserve_host      = true;
    cfg->log_level          = LOG_INFO;
}

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)*(end - 1))) end--;
    *end = '\0';
    return s;
}

static int parse_backend_str(const char *value, Backend *be) {
    char buf[MAX_HOST_LEN + 16];
    strncpy(buf, value, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    /* Cerca l'ultimo ':' per supportare IPv6 in futuro */
    char *colon = strrchr(buf, ':');
    if (!colon) return -1;
    *colon = '\0';

    strncpy(be->host, buf, MAX_HOST_LEN - 1);
    be->host[MAX_HOST_LEN - 1] = '\0';
    be->port = atoi(colon + 1);

    if (be->port <= 0 || be->port > 65535) return -1;
    if (be->host[0] == '\0') return -1;
    return 0;
}

/* Sort routes: prefix più lungo prima (greedy longest match) */
static void routes_sort(Config *cfg) {
    for (int i = 0; i < cfg->route_count - 1; i++) {
        for (int j = i + 1; j < cfg->route_count; j++) {
            size_t li = strlen(cfg->routes[i].prefix);
            size_t lj = strlen(cfg->routes[j].prefix);
            if (li < lj) {
                Route tmp = cfg->routes[i];
                cfg->routes[i] = cfg->routes[j];
                cfg->routes[j] = tmp;
            }
        }
    }
}

int config_load(Config *cfg, const char *path) {
    config_defaults(cfg);

    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "config: cannot open '%s'\n", path);
        return -1;
    }

    char line[1024];
    int  lineno = 0;
    int  errors = 0;

    while (fgets(line, sizeof(line), f)) {
        lineno++;
        char *l = trim(line);

        /* Ignora vuote e commenti */
        if (!l[0] || l[0] == '#' || l[0] == ';') continue;

        char *eq = strchr(l, '=');
        if (!eq) {
            fprintf(stderr, "config:%d: no '=' found, skipping\n", lineno);
            errors++;
            continue;
        }
        *eq = '\0';
        char *key = trim(l);
        char *val = trim(eq + 1);

        /* Rimuove commenti inline */
        char *comment = strchr(val, '#');
        if (comment) { *comment = '\0'; trim(val); }

        if      (!strcmp(key, "listen_host"))         strncpy(cfg->listen_host, val, MAX_HOST_LEN - 1);
        else if (!strcmp(key, "listen_port"))         cfg->listen_port = atoi(val);
        else if (!strcmp(key, "backlog"))             cfg->backlog = atoi(val);
        else if (!strcmp(key, "connect_timeout_ms"))  cfg->connect_timeout_ms = atoi(val);
        else if (!strcmp(key, "read_timeout_ms"))     cfg->read_timeout_ms = atoi(val);
        else if (!strcmp(key, "write_timeout_ms"))    cfg->write_timeout_ms = atoi(val);
        else if (!strcmp(key, "max_header_bytes"))    cfg->max_header_bytes = (size_t)atol(val);
        else if (!strcmp(key, "max_body_bytes"))      cfg->max_body_bytes = (size_t)atol(val);
        else if (!strcmp(key, "preserve_host"))       cfg->preserve_host = (!strcmp(val,"true") || !strcmp(val,"1"));
        else if (!strcmp(key, "log_file"))            strncpy(cfg->log_file, val, sizeof(cfg->log_file) - 1);
        else if (!strcmp(key, "log_level")) {
            if      (!strcmp(val, "debug")) cfg->log_level = LOG_DEBUG;
            else if (!strcmp(val, "info"))  cfg->log_level = LOG_INFO;
            else if (!strcmp(val, "warn"))  cfg->log_level = LOG_WARN;
            else if (!strcmp(val, "error")) cfg->log_level = LOG_ERROR;
            else { fprintf(stderr, "config:%d: unknown log_level '%s'\n", lineno, val); errors++; }
        }
        else if (!strncmp(key, "route.", 6)) {
            if (cfg->route_count >= MAX_ROUTES) {
                fprintf(stderr, "config:%d: too many routes (max %d)\n", lineno, MAX_ROUTES);
                errors++;
                continue;
            }
            Route *r = &cfg->routes[cfg->route_count];
            strncpy(r->prefix, key + 6, MAX_PATH_LEN - 1);
            r->prefix[MAX_PATH_LEN - 1] = '\0';
            if (parse_backend_str(val, &r->backend) < 0) {
                fprintf(stderr, "config:%d: invalid backend '%s' (expected host:port)\n", lineno, val);
                errors++;
                continue;
            }
            cfg->route_count++;
        }
        else {
            fprintf(stderr, "config:%d: unknown key '%s'\n", lineno, key);
        }
    }

    fclose(f);

    if (cfg->route_count == 0) {
        fprintf(stderr, "config: no routes defined\n");
        return -1;
    }

    /* Validazioni base */
    if (cfg->listen_port <= 0 || cfg->listen_port > 65535) {
        fprintf(stderr, "config: invalid listen_port %d\n", cfg->listen_port);
        return -1;
    }

    routes_sort(cfg);
    return errors > 0 ? 1 : 0; /* 1 = warning (continua), -1 = fatal */
}

void config_dump(const Config *cfg) {
    log_info("listen     = %s:%d (backlog=%d)",
             cfg->listen_host, cfg->listen_port, cfg->backlog);
    log_info("timeouts   = connect:%dms read:%dms write:%dms",
             cfg->connect_timeout_ms, cfg->read_timeout_ms, cfg->write_timeout_ms);
    log_info("limits     = headers:%zu body:%zu",
             cfg->max_header_bytes, cfg->max_body_bytes);
    log_info("preserve_host = %s", cfg->preserve_host ? "true" : "false");
    for (int i = 0; i < cfg->route_count; i++) {
        log_info("route[%d]   = '%s' → %s:%d",
                 i, cfg->routes[i].prefix,
                 cfg->routes[i].backend.host, cfg->routes[i].backend.port);
    }
}
