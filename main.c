#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "config.h"
#include "log.h"
#include "stats.h"
#include "net.h"
#include "proxy.h"

/* Flag per graceful shutdown — volatile per signal handler */
static volatile sig_atomic_t g_running = 1;

static void signal_handler(int sig) {
    (void)sig;
    g_running = 0;
}

static void setup_signals(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; /* Riavvia accept() interrotta */

    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* Ignora SIGPIPE: gestiamo gli errori di write esplicitamente */
    signal(SIGPIPE, SIG_IGN);
}

static void print_usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [-c config_file] [-v]\n"
        "  -c FILE   Config file (default: proxy.conf)\n"
        "  -v        Print version and exit\n",
        prog);
}

int main(int argc, char *argv[]) {
    const char *config_path = "proxy.conf";
    int opt;

    while ((opt = getopt(argc, argv, "c:vh")) != -1) {
        switch (opt) {
            case 'c': config_path = optarg; break;
            case 'v':
                printf("cproxy v1.0.0\n");
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    /* Carica configurazione */
    Config cfg;
    int cfg_r = config_load(&cfg, config_path);
    if (cfg_r < 0) {
        fprintf(stderr, "Fatal: failed to load config '%s'\n", config_path);
        return 1;
    }

    /* Inizializza logger */
    log_init(cfg.log_level, cfg.log_file[0] ? cfg.log_file : NULL);
    log_info("cproxy v1.0.0 starting");
    config_dump(&cfg);

    /* Inizializza statistiche */
    Stats stats;
    stats_init(&stats);

    /* Segnali */
    setup_signals();

    /* Apri socket in ascolto */
    int listen_fd = net_listen(cfg.listen_host, cfg.listen_port, cfg.backlog);
    if (listen_fd < 0) {
        log_error("Failed to bind %s:%d", cfg.listen_host, cfg.listen_port);
        log_close();
        return 1;
    }

    log_info("Listening on %s:%d", cfg.listen_host, cfg.listen_port);
    log_info("Press Ctrl+C to stop.");

    /* ── Accept loop ──────────────────────────────────────────────────── */
    while (g_running) {
        struct sockaddr_storage client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(listen_fd,
                               (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR) continue; /* segnale ricevuto */
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            log_error("accept: %s", strerror(errno));
            continue; /* non uscire su errore transiente */
        }

        /* Ricava IP client */
        char client_ip[48] = "unknown";
        int  client_port   = 0;
        if (client_addr.ss_family == AF_INET) {
            struct sockaddr_in *sa = (struct sockaddr_in *)&client_addr;
            inet_ntop(AF_INET, &sa->sin_addr, client_ip, sizeof(client_ip));
            client_port = ntohs(sa->sin_port);
        } else if (client_addr.ss_family == AF_INET6) {
            struct sockaddr_in6 *sa = (struct sockaddr_in6 *)&client_addr;
            inet_ntop(AF_INET6, &sa->sin6_addr, client_ip, sizeof(client_ip));
            client_port = ntohs(sa->sin6_port);
        }

        log_debug("accepted connection from %s:%d (fd=%d)",
                  client_ip, client_port, client_fd);

        /*
         * v1.0: gestione sincrona (una richiesta alla volta).
         * v1.1: sostituire con pthread_create(&tid, NULL, worker, ctx)
         */
        proxy_handle_connection(client_fd, client_ip, client_port, &cfg, &stats);
    }

    /* ── Graceful shutdown ────────────────────────────────────────────── */
    log_info("Shutting down gracefully...");
    close(listen_fd);

    char stats_buf[512];
    stats_to_json(&stats, stats_buf, sizeof(stats_buf));
    log_info("Final stats:\n%s", stats_buf);
    log_info("Goodbye.");

    log_close();
    return 0;
}
