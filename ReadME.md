![task_01kj7tx470fdste5ftm0erc6qf_1771937135_img_0](https://github.com/user-attachments/assets/0d95c3cc-cf50-4788-ac24-bfee373af6e4)




# cproxy

`cproxy` è un **reverse proxy HTTP leggero scritto in C**, pensato per essere semplice da capire, facile da installare e abbastanza modulare da estendere.

L’obiettivo del progetto è offrire una base solida per gestire richieste HTTP in ingresso, instradarle verso servizi interni (backend), e aggiungere funzionalità utili come logging, routing e statistiche.

---

## Perché questo progetto

Questo progetto nasce per:

- capire davvero come funziona un reverse proxy (a basso livello)
- avere un componente leggero e controllabile
- costruire una base open source chiara, leggibile e migliorabile
- imparare networking, parsing HTTP e gestione socket in C

In pratica: è un progetto didattico **ma con una struttura reale**, utile anche come base per sviluppi più avanzati.

---

## Cosa fa (in sintesi)

`cproxy`:

- ascolta su una porta (socket server)
- accetta connessioni client
- legge la richiesta HTTP
- decide dove instradarla (routing)
- inoltra la richiesta al backend corretto
- riceve la risposta
- la rimanda al client
- registra log e aggiorna statistiche

---

## Architettura del progetto

```text
cproxy/
├── Makefile
├── proxy.conf
├── README.md
└── src/
    ├── main.c        ← entry point, signal handling, accept loop
    ├── config.h/c    ← parsing config file, validazione
    ├── log.h/c       ← logger thread-safe con livelli
    ├── stats.h/c     ← contatori atomici, JSON export
    ├── net.h/c       ← socket primitives, connect timeout, pipe
    ├── http.h/c      ← parse request/response, header manipulation
    ├── router.h/c    ← route matching (longest prefix first)
    └── proxy.h/c     ← logica core: forward request, handle interno
