# ─── cproxy Makefile ──────────────────────────────────────────────────────
CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic \
          -Wshadow -Wstrict-prototypes \
          -D_POSIX_C_SOURCE=200809L \
          -D_DEFAULT_SOURCE \
          -Isrc
LDFLAGS =

# Build targets
DEBUG_FLAGS   = -g3 -O0 -DDEBUG -fsanitize=address,undefined
RELEASE_FLAGS = -O2 -DNDEBUG

SRCS = src/main.c   \
       src/log.c    \
       src/config.c \
       src/stats.c  \
       src/net.c    \
       src/http.c   \
       src/router.c \
       src/proxy.c

OBJS = $(SRCS:.c=.o)
BIN  = cproxy

.PHONY: all debug release clean install

all: release

release: CFLAGS += $(RELEASE_FLAGS)
release: $(BIN)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: LDFLAGS += -fsanitize=address,undefined
debug: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build OK → ./$(BIN)"

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $

clean:
	rm -f $(OBJS) $(BIN)

install: release
	install -m 755 $(BIN) /usr/local/bin/$(BIN)
	@echo "Installed to /usr/local/bin/$(BIN)"

# Dipendenze header
src/main.o:   src/config.h src/log.h src/stats.h src/net.h src/proxy.h
src/proxy.o:  src/proxy.h src/config.h src/http.h src/router.h src/net.h src/log.h src/stats.h
src/http.o:   src/http.h src/net.h src/log.h
src/router.o: src/router.h src/config.h
src/net.o:    src/net.h src/log.h
src/config.o: src/config.h src/log.h
src/stats.o:  src/stats.h
src/log.o:    src/log.h
