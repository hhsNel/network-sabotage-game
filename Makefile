CC ?= cc

SRCDIR := src
BUILDDIR := build
TESTDIR := test
OBJDIR := $(BUILDDIR)/objs
HARNESSDIR := $(BUILDDIR)/tests
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TEST_SRCS := $(wildcard $(TESTDIR)/*.c)
TEST_BINS := $(TEST_SRCS:$(TESTDIR)/%.c=$(HARNESSDIR)/%)
TGT := network-sabotage

CFLAGS := -std=c99 -Wall -Wextra -Wpedantic -I$(SRCDIR) -g
LDFLAGS := -g

.PHONY: all clean check

all: $(TGT)

$(TGT): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(HARNESSDIR)/%: $(TESTDIR)/%.c $(OBJS) | $(HARNESSDIR)
	$(CC) $(CFLAGS) -o $@ $^

check: $(TEST_BINS)
	for f in $(TESTDIR)/*.exp; do expect $$f; done

$(OBJDIR):
	mkdir -p $@

$(HARNESSDIR):
	mkdir -p $@

clean:
	rm -rf $(BUILDDIR) $(TGT)

