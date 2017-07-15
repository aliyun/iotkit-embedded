.PHONY: all

ifeq (cc,$(CC))
CC          := gcc
endif

UT_PROG     := lite-utils_prog
SRCS        := $(wildcard *.c)
SRCS        := $(filter-out $(UT_SRC),$(SRCS))
OBJS        := $(subst .c,.o,$(SRCS))

LIB_FMT     ?= static
CC          ?= gcc
AR          ?= ar
LD          ?= ld
CFLAGS      += -Werror
LDFLAGS     += -L. -llite-utils

VARS    := \
    UT_PROG \
    SRCS \
    OBJS \
    LIB_FMT \
    CC \
    AR \
    LD \
    ALL_TARGET \
    CFLAGS \
    LDFLAGS \

ifeq (static,$(LIB_FMT))
ALL_TARGET := liblite-utils.a
else
$(error LIB_FMT must be static!)
endif
ALL_TARGET += $(UT_PROG)

all: $(ALL_TARGET)

$(UT_PROG): $(UT_PROG).c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	@rm -f *.o *.d *.a *.so $(UT_PROG)

%.a: $(OBJS)
	$(AR) -rcs $@ $(OBJS)
	@rm -f *.o *.d

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# %.d: %.c
# 	@$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
# 	    sed 's,\($*\)\.o[ :]*,\1.o $@: ,g' < $@.$$$$ > $@; \
# 	    rm -f $@.$$$$
# 
# sinclude $(OBJS:.o=.d)

info:
	@rm -f *.o *.d *.a *.so

ifeq (info,$(MAKECMDGOALS))
$(foreach V,$(sort $(VARS)),$(info $(V) = $($(V))))
endif
