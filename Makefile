CC ?= cc

SRCDIR := src
BUILDDIR := build
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(SRCS:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
TGT := network-sabotage

CFLAGS := -std=c99 -Wall -Wextra -Wpedantic -I$(SRCDIR)
LDFLAGS := 

.PHONY: all clean

all: $(TGT)

$(TGT): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILDDIR):
	mkdir -p $@

clean:
	rm -rf $(BUILDDIR) $(TGT)

