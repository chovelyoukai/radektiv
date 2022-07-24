CFLAGS=-c -g -Wall -Wno-return-type -I./
LDFLAGS=-L.
LDLIBS=-lGL -lglfw -lGLEW -lm

MKDIR=mkdir
RMDIR=rm -r
CC=gcc

SRCDIR=./src
SRCS=$(wildcard $(SRCDIR)/*.c)
OBJDIR=./obj
OBJS=$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
BIN=radektiv

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	$(MKDIR) $@

clean:
	$(RMDIR) $(OBJDIR) $(BIN)
