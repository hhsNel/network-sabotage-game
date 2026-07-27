CC ?= cc

-include config.mk

SRCDIR := src
BUILDDIR := build
TESTDIR := test
OBJDIR := $(BUILDDIR)/objs
HARNESSDIR := $(BUILDDIR)/tests
SRCS := $(filter-out $(SRCDIR)/alpha-main.c, $(wildcard $(SRCDIR)/*.c))

ifeq ($(UI),ncurses)
  SRCS += $(SRCDIR)/platform/ui-ncurses.c
else
  $(error expected a UI)
endif

OBJS := $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
ALPHA_SRC := $(SRCDIR)/alpha-main.c
ALPHA_OBJ := $(ALPHA_SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TEST_SRCS := $(wildcard $(TESTDIR)/*.c)
TEST_BINS := $(TEST_SRCS:$(TESTDIR)/%.c=$(HARNESSDIR)/%)
TGT := network-sabotage-alpha

CFLAGS := -std=c99 -Wall -Wextra -Wpedantic -Werror -I$(SRCDIR) -g -D_GNU_SOURCE
LDFLAGS := -lncursesw -g

.PHONY: all clean check

all: $(TGT)

$(TGT): $(OBJS) $(ALPHA_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(HARNESSDIR)/%: $(TESTDIR)/%.c $(OBJS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

check: $(TEST_BINS)
	for f in $(TESTDIR)/*.exp; do expect $$f; done

clean:
	rm -rf $(BUILDDIR) $(TGT)

